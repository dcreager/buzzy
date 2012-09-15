# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import collections
import os
import os.path

import buzzy.build
import buzzy.build.autotools
import buzzy.build.cmake
import buzzy.build.none
import buzzy.config
import buzzy.recipe
import buzzy.source
import buzzy.source.download
import buzzy.source.git
import buzzy.utils
from buzzy.errors import BuzzyError
from buzzy.log import log


arch = buzzy.utils.get_arch_from_uname()


#-----------------------------------------------------------------------
# General package functions

def is_native_package(package_name):
    return buzzy.utils.if_run(["pacman", "-Si", package_name])


#-----------------------------------------------------------------------
# Arch packages

class Arch(buzzy.yaml.Fields):
    def fields(self):
        yield "native", {"default": None}

class Recipe(buzzy.recipe.Recipe):
    def fields(self):
        yield "arch", {"custom": Arch, "default": {}}

    def validate(self):
        native = False

        if isinstance(self.arch.native, str):
            package = NativePackage(self.arch.native, self)
        elif is_native_package(self.recipe_name):
            package = NativePackage(self.recipe_name, self)
        else:
            # For now, we only allow one built package per recipe.  That will
            # probably change.
            package = BuiltPackage(self.recipe_name, self)

        self.packages = [package]

    def package_names(self):
        return map(lambda x: x.package_name, self.packages)

    def install(self):
        for package in self.packages:
            package.install()

    def install_depends(self):
        for dep_recipe_name in self.depends:
            dep_recipe = buzzy.recipe.load(dep_recipe_name)
            dep_recipe.install()

    def install_build_depends(self):
        for dep_recipe_name in self.build_depends:
            dep_recipe = buzzy.recipe.load(dep_recipe_name)
            dep_recipe.install()


#-----------------------------------------------------------------------
# Native packages are easy

class NativePackage(object):
    def __init__(self, package_name, recipe):
        self.package_name = package_name
        self.recipe = recipe

    def package_spec(self):
        return "%s=%s" % (self.package_name, self.recipe.version)

    def installed(self):
        return buzzy.utils.if_run(["pacman", "-T", self.package_spec()])

    def build(self):
        # Nothing to build for a native package
        pass

    def install(self):
        self.recipe.install_depends()
        if self.installed():
            log(0, "Native package %s already installed" % self.package_name)
        else:
            log(0, "Installing native package %s" % self.package_name)
            buzzy.utils.sudo(["pacman", "-S", "--noconfirm",
                              self.package_spec()])
            self.installed = True


#-----------------------------------------------------------------------
# PKGBUILD file

# The directory that we build a package in.
def build_path(package_name):
    return os.path.join(buzzy.config.env.build_dir, package_name, "build")

# Some standard priorities for the various code blocks.
BUILD_CD           =  0
BUILD_UNPACK       = 10
BUILD_PRE          = 20
BUILD_CONFIGURE    = 30
BUILD_MAKE         = 40
BUILD_POST         = 50

INSTALL_CD         =  0
INSTALL_PRE        = 10
INSTALL_STAGE      = 20
INSTALL_POST       = 30

class Pkgbuild(object):
    class Variable(object):
        def __init__(self, name):
            self.name = name
            self.content = None

        def output(self, out):
            if self.content is not None:
                out.write("%s=%s\n" % (self.name, self))


    class ScalarVariable(Variable):
        def set(self, value):
            if value is not None:
                self.content = value

        def __str__(self):
            return repr(self.content)


    class ListVariable(Variable):
        def __str__(self):
            elements = map(repr, self.content)
            return "(%s)" % " ".join(elements)

        def append(self, value):
            if value is not None:
                if self.content is None:
                    self.content = []
                self.content.append(value)

        def extend(self, value):
            if value is not None:
                if self.content is None:
                    self.content = []
                self.content.extend(value)


    class CodeVariable(Variable):
        def __str__(self):
            lines = []
            for priority in sorted(self.content.keys()):
                lines.extend(map(lambda x: "  %s" % x, self.content[priority]))
            return "\n".join(lines)

        def append(self, priority, value, *subs):
            if value is not None:
                self.extend(priority, [value], *subs)

        def extend(self, priority, value, *subs):
            if value is not None:
                if self.content is None:
                    self.content = collections.defaultdict(list)
                attrs = {}
                if subs:
                    for sub in subs:
                        if isinstance(sub, dict):
                            attrs.update(sub)
                        else:
                            attrs.update(buzzy.utils.extract_attrs(sub))
                for element in value:
                    element = element % attrs
                    lines = buzzy.utils.trim(element).splitlines()
                    self.content[priority].extend(lines)

        def output(self, out):
            if self.content is not None:
                out.write("%s () {\n%s\n}\n" % (self.name, self))


    def __init__(self, package):
        self.package = package
        self.recipe = package.recipe
        self.build_path = package.build_path
        self.path = os.path.join(self.build_path, "PKGBUILD")
        self.content = {}

    def scalar(self, name):
        if name not in self.content:
            self.content[name] = self.ScalarVariable(name)
        return self.content[name]

    def list(self, name):
        if name not in self.content:
            self.content[name] = self.ListVariable(name)
        return self.content[name]

    def code(self, name):
        if name not in self.content:
            self.content[name] = self.CodeVariable(name)
        return self.content[name]

    def write(self):
        buzzy.utils.makedirs(os.path.dirname(self.path))
        out = open(self.path, "w")
        for var_name in sorted(self.content.keys()):
            self.content[var_name].output(out)
        out.close()


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
    def validate(self):
        super(Git, self).validate()
        if self.dev_build:
            self.package_name = "%s-git" % self.package_name

    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("makedepends").append("git")

        if self.dev_build:
            pkgbuild.scalar("_gitroot").set(self.url)
            self.pkgbuild.scalar("_gitname").set(self.commit)

        build = pkgbuild.code("build")
        build.append(BUILD_UNPACK, """
        rm -rf "${srcdir}/%(repo_name)s"
        cd "${srcdir}"
        git clone --depth 1 --branch "%(commit)s" "%(url)s"
        cd "${srcdir}/%(repo_name)s"
        git checkout -b buzzy-build "%(commit)s"
        """, self)

        package = pkgbuild.code("package")
        package.append(INSTALL_CD, 'cd "$srcdir/%s"' % self.repo_name)


#-----------------------------------------------------------------------
# Builders

class Autotools(buzzy.build.autotools.Autotools):
    def make_pkgbuild(self, package, pkgbuild):
        pkgbuild.list("options").append("!libtool")

        build = pkgbuild.code("build")
        build.append(BUILD_CONFIGURE, './configure --prefix=/usr')
        build.append(BUILD_MAKE, 'make')

        package = pkgbuild.code("package")
        package.append(INSTALL_STAGE, 'make DESTDIR="$pkgdir" install')


class Cmake(buzzy.build.cmake.Cmake):
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


class NoBuild(buzzy.build.none.NoBuild):
    def make_pkgbuild(self, package, pkgbuild):
        raise BuzzyError("Cannot build %s" % package.package_name)


#-----------------------------------------------------------------------
# Built packages are hard

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


class BuiltPackage(object):
    def __init__(self, package_name, recipe):
        self.package_name = package_name
        self.recipe = recipe
        self.build_path = build_path(package_name)
        self.pkgbuild = Pkgbuild(self)

    def package_filename(self):
        return "%s-%s-%s-%s.pkg.tar.xz" % \
            (self.package_name, self.recipe.version, self.recipe.revision, arch)

    def package_path(self):
        return os.path.join(pkgdest, self.package_filename())

    def package_spec(self):
        return "%s=%s-%s" % \
            (self.package_name, self.recipe.version, self.recipe.revision)

    def built(self):
        return os.path.exists(self.package_path())

    def installed(self):
        return buzzy.utils.if_run(["pacman", "-T", self.package_spec()])

    def make_pkgbuild(self):
        log(0, "  Creating PKGBUILD file")

        self.pkgbuild.list("arch").append(arch)
        self.pkgbuild.scalar("pkgname").set(self.package_name)
        self.pkgbuild.scalar("pkgver").set(self.recipe.version)
        self.pkgbuild.scalar("pkgrel").set(self.recipe.revision)
        self.pkgbuild.scalar("pkgdesc").set(self.recipe.description.strip())
        self.pkgbuild.scalar("url").set(self.recipe.url)

        for dep_recipe_name in self.recipe.depends:
            dep_recipe = buzzy.recipe.load(dep_recipe_name)
            package_names = dep_recipe.package_names()
            self.pkgbuild.list("depends").extend(package_names)

        for dep_recipe_name in self.recipe.build_depends:
            dep_recipe = buzzy.recipe.load(dep_recipe_name)
            package_names = dep_recipe.package_names()
            self.pkgbuild.list("makedepends").extend(package_names)

        arch_license = arch_licenses[self.recipe.license]
        self.pkgbuild.list("license").append(arch_license)
        if self.recipe.license_file:
            package = self.pkgbuild.code("package")
            package.append(INSTALL_POST, """
            install -Dm644 "%(license_file)s" \\
                "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
            """, self.recipe)
        else:
            if arch_license in arch_license_file_needed:
                raise BuzzyError("Need a license_file for %s license" %
                                 self.arch_license)

        for source in self.recipe.sources:
            source.make_pkgbuild(self, self.pkgbuild)

        self.recipe.build.make_pkgbuild(self, self.pkgbuild)

        self.pkgbuild.write()

    def clean_build_path(self):
        def rmdir(path):
            buzzy.utils.run(["rm", "-rf",
                             os.path.join(self.build_path, path)])
        def rm(path):
            try:
                os.remove(os.path.join(self.build_path, path))
            except OSError:
                pass
        log(0, "  Cleaning build directory")
        rmdir("src")
        rm(self.package_filename())

    def build(self):
        if not self.built():
            self.recipe.install_build_depends()
            log(0, "Building %s" % self.package_name)
            self.clean_build_path()
            self.make_pkgbuild()
            log(0, "  Building package")
            buzzy.utils.run(["makepkg", "-s"], cwd=self.build_path)

    def install(self):
        self.recipe.install_depends()
        if self.installed():
            log(0, "Built package %s already installed" % self.package_name)
        else:
            self.build()
            log(0, "Installing built package %s" % self.package_name)
            buzzy.utils.sudo(["pacman", "-U", "--noconfirm",
                              self.package_path()])


#-----------------------------------------------------------------------
# OS interface

class ArchLinux(object):
    name = "linux (arch)"
    arch = arch

    def __init__(self):
        # Set a PKGDEST environment variable for our makepkg calls.
        global pkgdest
        pkgdest = os.path.abspath(buzzy.config.env.package_dir)
        buzzy.utils.makedirs(pkgdest)
        os.environ["PKGDEST"] = pkgdest

        # Register a bunch of crap.
        buzzy.recipe.recipe_class = Recipe
        buzzy.build.add(Autotools)
        buzzy.build.add(Cmake)
        buzzy.build.add(NoBuild)
        buzzy.source.add(Download)
        buzzy.source.add(Git)

    def install(self, recipe):
        recipe.install()

    def configure(self):
        pass
