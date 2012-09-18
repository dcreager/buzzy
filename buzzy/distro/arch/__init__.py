# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import collections
import os
import os.path

import buzzy.build
import buzzy.build.autotools
import buzzy.build.cmake
import buzzy.build.none
import buzzy.build.python
import buzzy.config
import buzzy.recipe
import buzzy.source
import buzzy.source.download
import buzzy.source.git
import buzzy.utils
from buzzy.errors import BuzzyError
from buzzy.log import log

from buzzy.distro.arch.pkgbuild import *


arch = buzzy.utils.get_arch_from_uname()
pkgdest = None
repo_db = None
repo_files_db = None


#-----------------------------------------------------------------------
# General package functions

def is_native_package(package_name):
    return buzzy.utils.if_run(["pacman", "-Si", package_name])


#-----------------------------------------------------------------------
# Arch packages

class Arch(buzzy.yaml.Fields):
    def fields(self):
        yield "native", {"default": None}
        yield "python", {"default": None}

class Recipe(buzzy.recipe.Recipe):
    def fields(self):
        yield "arch", {"custom": Arch, "default": {}}


#-----------------------------------------------------------------------
# Native packages are easy

class NativePackage(buzzy.recipe.Package):
    def package_spec(self):
        return "%s=%s" % (self.package_name, self.recipe.version)

    def built(self):
        # Native packages are always built.
        return True

    def installed(self):
        return buzzy.utils.if_run(["pacman", "-T", self.package_spec()])

    def perform_build(self):
        # Nothing to build for a native package
        pass

    def perform_install(self):
        buzzy.utils.sudo(["pacman", "-S", "--noconfirm", self.package_spec()])


#-----------------------------------------------------------------------
# Built packages are hard

# The directory that we build a package in.
def build_path(package_name):
    return os.path.join(buzzy.config.env.build_dir, package_name, "build")

class BuiltPackage(buzzy.recipe.Package):
    def __init__(self, package_name, recipe, tags, dep_tag):
        super(BuiltPackage, self).__init__(package_name, recipe, tags, dep_tag)
        self.build_path = build_path(package_name)
        self.pkgbuild = Pkgbuild(self)
        self.pkgbuild.add_producer(ArchPackageMetadata())
        self.pkgbuild.add_producer(ArchLicenseChecker())
        for source in recipe.sources:
            self.pkgbuild.add_producer(source)
        self.pkgbuild.add_producer(recipe.build)

    def package_filename(self):
        return "%s-%s-%s-%s.pkg.tar.xz" % \
            (self.package_name, self.recipe.version, self.recipe.revision, arch)

    def package_path(self):
        return os.path.join(pkgdest, self.package_filename())

    def package_spec(self):
        return "%s=%s-%s" % \
            (self.package_name, self.recipe.version, self.recipe.revision)

    def built(self):
        result = os.path.exists(self.package_path())
        if result:
            log(0, "[%s] Package is already built" % self.full_name)
        return result

    def installed(self):
        result = buzzy.utils.if_run(["pacman", "-T", self.package_spec()])
        if result:
            log(0, "[%s] Package is already installed" % self.full_name)
        return result

    def clean_build_path(self):
        buzzy.utils.rmtree(os.path.join(self.build_path, "src"))
        buzzy.utils.rm(os.path.join(self.build_path, self.package_filename()))

    def make_pkgbuild(self):
        self.pkgbuild.make(self)
        self.pkgbuild.write()

    def perform_build(self):
        log(0, "[%s]   Cleaning build directory" % self.full_name)
        self.clean_build_path()
        log(0, "[%s]   Creating PKGBUILD file" % self.full_name)
        self.make_pkgbuild()
        log(0, "[%s]   Building package" % self.full_name)
        buzzy.utils.run(["makepkg", "-s", "-f"], cwd=self.build_path)
        log(0, "[%s]   Updating repository database" % self.full_name)
        buzzy.utils.run(["repo-add", "-d", repo_db,
                         self.package_path()])
        buzzy.utils.run(["repo-add", "-d", "-f", repo_files_db,
                         self.package_path()])

    def perform_install(self):
        buzzy.utils.sudo(["pacman", "-U", "--noconfirm",
                          self.package_path()])


#-----------------------------------------------------------------------
# Builders

def create_compiled_package(recipe):
    # For "normal" compiled packages, we trim off any subdirectories in the
    # recipe name to get the package name.
    package_name = os.path.basename(recipe.recipe_name)

    # The package is native if the recipe explicitly says so, or if there's a
    # package in the local Arch package database with the package's name.  (The
    # recipe also override this check if there's a conflicting name.)

    if recipe.arch.native is None:
        # Nothing explicit in the recipe; check whether there's an Arch package
        # with this name.
        native = is_native_package(package_name)

    elif not recipe.arch.native:
        # Recipe explicitly says the package is not native.
        native = False

    elif isinstance(recipe.arch.native, str):
        # Recipe explicitly says the package is native.
        native = True
        package_name = recipe.arch.native

    else:
        raise BuzzyError("Unexpected value for arch.native in %s" %
                         recipe.recipe_name)

    # Instantiate the native or built package class.
    if native:
        return NativePackage(package_name, recipe, ["default"])
    else:
        return BuiltPackage(package_name, recipe, ["default"])


class Autotools(buzzy.build.autotools.Autotools):
    def create_packages(self, recipe):
        return [create_compiled_package(recipe)]

    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("options").append("!libtool")

        build = pkgbuild.code("build")
        build.append(BUILD_CONFIGURE, './configure --prefix=/usr')
        build.append(BUILD_MAKE, 'make')

        package = pkgbuild.code("package")
        package.append(INSTALL_STAGE, 'make DESTDIR="$pkgdir" install')


class Cmake(buzzy.build.cmake.Cmake):
    def create_packages(self, recipe):
        return [create_compiled_package(recipe)]

    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("makedepends").append("cmake")

        build = pkgbuild.code("build")
        build.append(BUILD_CONFIGURE, """
        cmake_src=$(pwd)
        mkdir -p "${startdir}/cmake-build"
        pushd "${startdir}/cmake-build"
        cmake -DCMAKE_INSTALL_PREFIX=/usr "${cmake_src}"
        popd
        """)

        build.append(BUILD_MAKE, """
        pushd "${startdir}/cmake-build"
        make
        popd
        """)

        package = pkgbuild.code("package")
        package.append(INSTALL_STAGE, """
        pushd "${startdir}/cmake-build"
        make DESTDIR="$pkgdir" install
        popd
        """)

def python_prefix(python_version):
    if python_version == 2:
        return "python2"
    else:
        return "python"


def create_python_package(recipe, python_version):
    # For Python packages, we trim off any subdirectories in the recipe name to
    # get the package name's suffix.  (The prefix is always "python2-" or
    # "python-".)os.path.basename(recipe.recipe_name)
    prefix = python_prefix(python_version)
    package_suffix = os.path.basename(recipe.recipe_name)
    package_name = "%s-%s" % (prefix, package_suffix)

    # The package is native if the recipe explicitly says so, or if there's a
    # package in the local Arch package database with the package's name.  (The
    # recipe also override this check if there's a conflicting name.)

    if recipe.arch.native is None:
        # Nothing explicit in the recipe; check whether there's an Arch package
        # with this name.
        native = is_native_package(package_name)

    elif not recipe.arch.native:
        # Recipe explicitly says the package is not native.
        native = False

    elif isinstance(recipe.arch.native, str):
        # Recipe explicitly says the package is native.
        native = True
        package_suffix = recipe.arch.native
        package_name = "%s-%s" % (prefix, package_suffix)

    else:
        raise BuzzyError("Unexpected value for arch.native in %s" %
                         recipe.recipe_name)

    # The Python 3 package is the default for non-Python packages that list this
    # as a dependency.
    version_tag = "python%s" % python_version
    if python_version == 3:
        tags = [version_tag, "default"]
    else:
        tags = [version_tag]

    # Instantiate the native or built package class.
    if native:
        return NativePackage(package_name, recipe, tags, version_tag)
    else:
        return BuiltPackage(package_name, recipe, tags, version_tag)


class Python(buzzy.build.python.Python):
    def create_packages(self, recipe):
        return [
            create_python_package(recipe, 3),
            create_python_package(recipe, 2),
        ]

    def make_pkgbuild(self, package, pkgbuild):
        python_package = package.package_name.split("-")[0]
        script_suffix = python_package.replace("python", "")
        pkgbuild.list("depends").append(python_package)

        if self.installer == "distutils":
            # No extra dependencies
            pass
        elif self.installer in ["setuptools", "distribute"]:
            pkgbuild.list("depends").append("%s-distribute" % python_package)

        package = pkgbuild.code("package")
        package.append(INSTALL_STAGE, """
        %s setup.py install --root="$pkgdir/" --optimize=1
        """ % python_package)

        if script_suffix != "":
            package.append(INSTALL_STAGE, """
            for c in "$pkgdir/usr/bin/"*; do
                mv "${c}" "${c}%s"
            done
            """ % script_suffix)


#-----------------------------------------------------------------------
# Sources

class Download(buzzy.source.download.Download):
    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("source").append(self.url)
        pkgbuild.list("md5sums").append(self.md5)

        build = pkgbuild.code("build")
        build.append(BUILD_CD, 'cd "$srcdir/%s"' % self.extracted)

        package = pkgbuild.code("package")
        package.append(INSTALL_CD, 'cd "$srcdir/%s"' % self.extracted)


class Git(buzzy.source.git.Git):
    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("makedepends").append("git")

        if self.deep:
            git_args = ""
        else:
            git_args = '--depth 1 --branch "%s"' % self.commit

        build = pkgbuild.code("build")
        build.append(BUILD_UNPACK, """
        rm -rf "${srcdir}/%(repo_name)s"
        cd "${srcdir}"
        git clone """+git_args+""" "%(url)s"
        cd "${srcdir}/%(repo_name)s"
        git checkout -b buzzy-build origin/"%(commit)s"
        """, self)

        package = pkgbuild.code("package")
        package.append(INSTALL_CD, 'cd "$srcdir/%s"' % self.repo_name)


#-----------------------------------------------------------------------
# Basic package metadata

class ArchPackageMetadata(object):
    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("arch").append(arch)
        pkgbuild.scalar("pkgname").set(package.package_name)
        pkgbuild.scalar("pkgver").set(package.recipe.version)
        pkgbuild.scalar("pkgrel").set(package.recipe.revision)
        pkgbuild.scalar("pkgdesc").set(package.recipe.description.strip())
        pkgbuild.scalar("url").set(package.recipe.url)

        for dep_package in package.depends:
            pkgbuild.list("depends").append(dep_package.package_name)

        for dep_package in package.build_depends:
            pkgbuild.list("makedepends").append(dep_package.package_name)


#-----------------------------------------------------------------------
# Licenses

arch_licenses = collections.defaultdict(lambda: "custom", {
    "AGPL": "AGPL",
    "AGPL3": "AGPL3",
    "Apache": "Apache",
    "BSD": "BSD",
    "BSD3": "BSD",
    "GPL": "GPL",
    "GPL2": "GPL2",
    "GPL3": "GPL3",
    "LGPL": "LGPL",
    "LGPL2.1": "LGPL2.1",
    "LGPL3": "LGPL3",
    "MIT": "MIT",
    "Perl": "PerlArtistic",
    "PHP": "PHP",
    "Python": "Python",
    "Ruby": "RUBY",
    "zlib": "ZLIB",
})

arch_license_file_needed = [
    "custom",
    "BSD",
    "MIT",
    "Python",
    "ZLIB",
]

class ArchLicenseChecker(object):
    def make_pkgbuild(self, package, pkgbuild):
        arch_license = arch_licenses[package.recipe.license]
        pkgbuild.list("license").append(arch_license)
        if arch_license in arch_license_file_needed:
            if package.recipe.license_file:
                package_ = pkgbuild.code("package")
                package_.append(INSTALL_POST, """
                install -Dm644 "%(license_file)s" \\
                    "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
                """, package.recipe)
            else:
                raise BuzzyError("Need a license_file for %s license" %
                                 arch_license)


#-----------------------------------------------------------------------
# OS interface

class ArchLinux(object):
    name = "linux (arch)"
    arch = arch

    @classmethod
    def detect(cls):
        if os.path.exists("/etc/arch-release"):
            return cls()
        else:
            return None

    def __init__(self):
        # Register a bunch of crap.
        buzzy.recipe.recipe_class = Recipe
        buzzy.build.add(Autotools)
        buzzy.build.add(Cmake)
        buzzy.build.add(buzzy.build.none.NoBuild)
        buzzy.build.add(Python)
        buzzy.source.add(Download)
        buzzy.source.add(Git)

    def setup(self):
        # Set a PKGDEST environment variable for our makepkg calls.
        global pkgdest
        pkgdest = os.path.abspath(buzzy.config.env.repo_dir)
        buzzy.utils.makedirs(pkgdest)
        os.environ["PKGDEST"] = pkgdest

        # And also one for the name of the packager.
        os.environ["PACKAGER"] = "%s <%s>" % \
            (buzzy.config.env.name, buzzy.config.env.email)

        # And construct the path to the repository database.
        global repo_db
        global repo_files_db
        repo_db = os.path.join(pkgdest, "%s.db.tar.xz" %
                               buzzy.config.env.repo_name)
        repo_files_db = os.path.join(pkgdest, "%s.files.tar.xz" %
                                     buzzy.config.env.repo_name)

    def build(self, recipe):
        self.setup()
        recipe.build_recipe(buzzy.config.force)

    def install(self, recipe):
        self.setup()
        recipe.install_recipe(buzzy.config.force)

    def configure(self):
        pass
