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
import re

import buzzy.config
import buzzy.recipe
import buzzy.utils
from buzzy.errors import BuzzyError
from buzzy.log import log


arch = buzzy.utils.get_arch_from_uname()


#-----------------------------------------------------------------------
# General package functions

def is_native_package(package_name):
    return buzzy.utils.if_run(["pacman", "-Si", package_name])

# Returns a list of packages for a recipe.
def recipe_packages(recipe):
    if not hasattr(recipe, "packages"):
        native = False

        if "arch.native" in recipe:
            native = isinstance(recipe["arch.native"], str)
        elif is_native_package(recipe.package_name):
            native = True

        if native:
            package = NativePackage(recipe)
            package.visit(recipe.yaml_content)
            recipe.packages = [package]
        else:
            # For now, we only allow one built package per recipe.  That will
            # probably change.
            package = BuiltPackage(recipe)
            package.visit(recipe.yaml_content)
            recipe.packages = [package]

    return recipe.packages

# Get the list of package names for a recipe.
def recipe_package_names(recipe_name):
    recipe = buzzy.recipe.load(recipe_name)
    packages = recipe_packages(recipe)
    return map(lambda x: x.name, packages)

def install_recipe(recipe):
    for package in recipe_packages(recipe):
        package.install()

def install_deps(recipe, attr):
    if attr in recipe.yaml_content:
        for dep_recipe_name in recipe.yaml_content[attr]:
            dep_recipe = buzzy.recipe.load(dep_recipe_name)
            install_recipe(dep_recipe)


#-----------------------------------------------------------------------
# Native packages are easy

class NativePackage(buzzy.recipe.SchemaVisitor):
    kind = "Arch native package"
    required = []
    optional = [("arch.native", "name")]

    def __init__(self, recipe):
        self.recipe = recipe
        super(NativePackage, self).__init__()

    def full_package_spec(self):
        return "%s=%s-%s" % \
            (self.name, self.recipe.version, self.recipe.revision)

    def installed(self):
        return buzzy.utils.if_run(["pacman", "-T", self.full_package_spec()])

    def initialize(self):
        super(NativePackage, self).initialize()
        # This will be overridden by an arch.native argument, if present.
        self.name = self.recipe.package_name

    def build(self):
        # Nothing to build for a native package
        pass

    def install(self):
        install_deps(self.recipe, "depends")
        if self.installed():
            log(0, "Native package %s already installed" % self.name)
        else:
            log(0, "Installing native package %s" % self.name)
            buzzy.utils.sudo(["pacman", "-S", "--noconfirm",
                              self.full_package_spec()])
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

        def append(self, priority, value):
            if value is not None:
                self.extend(priority, [value])

        def extend(self, priority, value):
            if value is not None:
                if self.content is None:
                    self.content = collections.defaultdict(list)
                for element in value:
                    lines = buzzy.utils.trim(element).splitlines()
                    self.content[priority].extend(lines)

        def output(self, out):
            if self.content is not None:
                out.write("%s () {\n%s\n}\n" % (self.name, self))


    def __init__(self, path):
        self.path = path
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

class DownloadSource(buzzy.recipe.SchemaVisitor):
    kind = "download source"
    required = ["url", "md5"]
    optional = ["extracted"]

    def __init__(self, package):
        self.package = package
        self.recipe = package.recipe
        self.pkgbuild = package.pkgbuild
        super(DownloadSource, self).__init__()

    def process(self):
        self.url = self.recipe.subst(self.url)
        if self.extracted is None:
            self.extracted = buzzy.utils.tarball_basename(self.url)

        self.pkgbuild.list("source").append(self.url)
        self.pkgbuild.list("md5sums").append(self.md5)

        build = self.pkgbuild.code("build")
        build.append(BUILD_CD, 'cd "$srcdir/%s"' % self.extracted)

        package = self.pkgbuild.code("package")
        package.append(INSTALL_CD, 'cd "$srcdir/%s"' % self.extracted)


class GitSource(buzzy.recipe.SchemaVisitor):
    kind = "git source"
    required = ["url"]
    optional = ["tag", "branch"]

    def __init__(self, package):
        self.package = package
        self.recipe = package.recipe
        self.pkgbuild = package.pkgbuild
        super(GitSource, self).__init__()

    def process(self):
        if self.branch is not None and self.tag is not None:
            raise BuzzyError("Cannot specify both branch and tag")
        if self.branch is not None:
            self.commit = self.branch
            self.dev_build = True
        else:
            self.commit = self.tag
            self.dev_build = False

        self.url = self.recipe.subst(self.url)
        self.commit = self.recipe.subst(self.commit)
        self.repo_name = re.sub("\.git$", "", os.path.basename(self.url))

        self.pkgbuild.list("makedepends").append("git")

        if self.dev_build:
            self.package.name = "%s-git" % self.package.name
            self.pkgbuild.scalar("_gitroot").set(self.url)
            self.pkgbuild.scalar("_gitname").set(self.commit)

        build = self.pkgbuild.code("build")
        build.append(BUILD_UNPACK, """
        rm -rf "${srcdir}/%(repo_name)s"
        cd "${srcdir}"
        git clone --depth 1 --branch "%(commit)s" "%(url)s"
        cd "${srcdir}/%(repo_name)s"
        git checkout -b buzzy-build "%(commit)s"
        """ % buzzy.utils.extract_attrs(self))

        package = self.pkgbuild.code("package")
        package.append(INSTALL_CD, 'cd "$srcdir/%s"' % self.repo_name)


class Source(buzzy.recipe.TableVisitor):
    kind = "source"

    def __init__(self, package):
        self.types = {
            "download": DownloadSource(package),
            "git": GitSource(package),
        }
        super(Source, self).__init__()


#-----------------------------------------------------------------------
# Builders

class AutotoolsBuild(buzzy.recipe.Visitor):
    kind = "autotools build"

    def __init__(self, package):
        self.package = package
        self.recipe = package.recipe
        self.pkgbuild = package.pkgbuild
        super(AutotoolsBuild, self).__init__()

    def scalar(self, value):
        self.process()

    seq = scalar
    map = scalar

    def process(self):
        self.pkgbuild.list("options").append("!libtool")

        build = self.pkgbuild.code("build")
        build.append(BUILD_CONFIGURE, './configure --prefix=/usr')
        build.append(BUILD_MAKE, 'make')

        package = self.pkgbuild.code("package")
        package.append(INSTALL_STAGE, 'make DESTDIR="$pkgdir" install')


class CmakeBuild(buzzy.recipe.Visitor):
    kind = "cmake build"

    def __init__(self, package):
        self.package = package
        self.recipe = package.recipe
        self.pkgbuild = package.pkgbuild
        super(CmakeBuild, self).__init__()

    def scalar(self, value):
        self.process()

    seq = scalar
    map = scalar

    def process(self):
        self.pkgbuild.list("makedepends").append("cmake")

        build = self.pkgbuild.code("build")
        build.append(BUILD_CONFIGURE, """
        cmake_src=$(pwd)
        mkdir -p "${startdir}/cmake-build"
        pushd "${startdir}/cmake-build"
        cmake -DCMAKE_INSTALL_PREFIX=/usr "${cmake_src}"
        popd
        """ % buzzy.utils.extract_attrs(self))

        build.append(BUILD_MAKE, """
        pushd "${startdir}/cmake-build"
        make
        popd
        """ % buzzy.utils.extract_attrs(self))

        package = self.pkgbuild.code("package")
        package.append(INSTALL_STAGE, """
        pushd "${startdir}/cmake-build"
        make DESTDIR="$pkgdir" install
        popd
        """ % buzzy.utils.extract_attrs(self))


class Build(buzzy.recipe.TableVisitor):
    kind = "build"

    def __init__(self, package):
        self.types = {
            "autotools": AutotoolsBuild(package),
            "cmake": CmakeBuild(package),
        }
        super(Build, self).__init__()


#-----------------------------------------------------------------------
# Built packages are hard

class BuiltPackage(buzzy.recipe.MapForeach):
    kind = "Arch built package"

    def __init__(self, recipe):
        self.name = recipe.package_name
        self.recipe = recipe
        self.built = False
        self.build_path = build_path(self.name)
        pkgbuild_path = os.path.join(self.build_path, "PKGBUILD")
        self.pkgbuild = Pkgbuild(pkgbuild_path)

        # Nested visitors
        self.visitors = {}
        source = Source(self)
        self.visitors["sources"] = buzzy.recipe.SeqForeach("sources", source)
        self.visitors["build"] = Build(self)

        super(BuiltPackage, self).__init__()

    def package_filename(self):
        return "%s-%s-%s-%s.pkg.tar.xz" % \
            (self.name, self.recipe.version, self.recipe.revision, arch)

    def package_path(self):
        return os.path.join(self.build_path, self.package_filename())

    def full_package_spec(self):
        return "%s=%s-%s" % \
            (self.name, self.recipe.version, self.recipe.revision)

    def installed(self):
        return buzzy.utils.if_run(["pacman", "-T", self.full_package_spec()])

    def start_map(self):
        self.pkgbuild.list("arch").append(arch)
        self.need_license_file = False
        self.have_license_file = False

    def map_package_name(self, value):
        self.pkgbuild.scalar("pkgname").set(value)

    def map_version(self, value):
        self.pkgbuild.scalar("pkgver").set(value)

    def map_revision(self, value):
        self.pkgbuild.scalar("pkgrel").set(value)

    def map_description(self, value):
        self.pkgbuild.scalar("pkgdesc").set(value.strip())

    def map_license(self, value):
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

        self.arch_license = arch_licenses[value]
        self.pkgbuild.list("license").append(self.arch_license)
        if self.arch_license in arch_license_file_needed:
            self.need_license_file = True

    def map_url(self, value):
        self.pkgbuild.scalar("url").set(value)

    def map_depends(self, value):
        for recipe_name in value:
            packages = recipe_package_names(recipe_name)
            self.pkgbuild.list("depends").extend(packages)

    def map_build_depends(self, value):
        for recipe_name in value:
            packages = [x for x in recipe_package_names(recipe_name)]
            self.pkgbuild.list("makedepends").extend(packages)

    def map_license_file(self, value):
        self.have_license_file = True
        package = self.pkgbuild.code("package")
        package.append(INSTALL_POST, 'install -Dm644 %s %s' %
                       (value, '"$pkgdir/usr/share/licenses/$pkgname/LICENSE"'))

    def finish_map(self):
        if self.need_license_file and not self.have_license_file:
            raise BuzzyError("Need a license_file for %s license" %
                             self.arch_license)

    def make_pkgbuild(self):
        log(0, "Creating PKGBUILD file")
        self.pkgbuild.write()

    def clean_build_path(self):
        def rmdir(path):
            buzzy.utils.run(["rm", "-rf", os.path.join(self.build_path, path)])
        def rm(path):
            try:
                os.remove(os.path.join(self.build_path, path))
            except OSError:
                pass
        log(0, "Cleaning build directory")
        rmdir("src")
        rm(self.package_filename())

    def build(self):
        if not self.built:
            install_deps(self.recipe, "build_depends")
            log(0, "Building package %s" % self.name)
            self.clean_build_path()
            self.make_pkgbuild()
            buzzy.utils.run(["makepkg", "-s"], cwd=self.build_path)
            self.built = True

    def install(self):
        install_deps(self.recipe, "depends")
        if self.installed():
            log(0, "Built package %s already installed" % self.name)
        else:
            self.build()
            log(0, "Installing built package %s" % self.name)
            buzzy.utils.sudo(["pacman", "-U", "--noconfirm",
                              self.package_path()])


#-----------------------------------------------------------------------
# OS interface

class ArchLinux(object):
    name = "linux (arch)"
    arch = arch

    def __init__(self):
        self.installed = set()

    def install(self, recipe):
        install_recipe(recipe)
