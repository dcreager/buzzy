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

import buzzy.config
import buzzy.recipe
import buzzy.utils
from buzzy.errors import BuzzyError
from buzzy.log import log


arch = buzzy.utils.get_arch_from_uname()


#-----------------------------------------------------------------------
# General package functions

# Returns a list of packages for a recipe.
def recipe_packages(recipe):
    if "arch.native" in recipe:
        package = NativePackage(recipe)
        package.visit(recipe.yaml_content)
        return [package]
    else:
        # For now, we only allow one built package per recipe.  That will
        # probably change.
        package = BuiltPackage(recipe)
        package.visit(recipe.yaml_content)
        return [package]

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
    required = [("arch.native", "name")]
    optional = []

    def __init__(self, recipe):
        self.recipe = recipe
        self.installed = False
        super(NativePackage, self).__init__()

    def build(self):
        # Nothing to build for a native package
        pass

    def install(self):
        if not self.installed:
            install_deps(self.recipe, "depends")
            log(0, "Installing native package %s" % self.name)
            buzzy.utils.sudo(["pacman", "-S", "--noconfirm", self.name])
            self.installed = True


#-----------------------------------------------------------------------
# PKGBUILD file

# The directory that we build a package in.
def build_path(package_name):
    return os.path.join(buzzy.config.build_dir, package_name, "build")

# Some standard priorities for the various code blocks.
BUILD_CD           =  0
BUILD_PRE          = 10
BUILD_CONFIGURE    = 20
BUILD_MAKE         = 30
BUILD_POST         = 40

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
        def set(self, value, modifier=None):
            if value is not None:
                if modifier is None:
                    self.content = value
                else:
                    self.content = modifier(value)

        def __str__(self):
            return repr(self.content)


    class ListVariable(Variable):
        def __str__(self):
            elements = map(repr, self.content)
            return "(%s)" % " ".join(elements)

        def append(self, value, modifier=None):
            if value is not None:
                if self.content is None:
                    self.content = []
                if modifier is None:
                    self.content.append(value)
                else:
                    self.content.append(modifier(value))

        def extend(self, value, modifier=None):
            if value is not None:
                if self.content is None:
                    self.content = []
                if modifier is None:
                    for element in value:
                        self.content.append(element)
                else:
                    for element in value:
                        self.content.extend(modifier(element))


    class CodeVariable(Variable):
        def __str__(self):
            lines = []
            for priority in sorted(self.content.keys()):
                lines.extend(map(lambda x: "  %s" % x, self.content[priority]))
            return "\n".join(lines)

        def append(self, priority, value, modifier=None):
            if value is not None:
                if self.content is None:
                    self.content = collections.defaultdict(list)
                if modifier is None:
                    self.content[priority].append(value)
                else:
                    self.content[priority].append(modifier(value))

        def extend(self, value, modifier=None):
            if value is not None:
                if self.content is None:
                    self.content = collections.defaultdict(list)
                if modifier is None:
                    for element in value:
                        self.content[priority].append(element)
                else:
                    for element in value:
                        self.content[priority].extend(modifier(element))

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

    def __init__(self, recipe, pkgbuild):
        self.recipe = recipe
        self.pkgbuild = pkgbuild
        super(DownloadSource, self).__init__()

    def process(self):
        self.url = self.url % self.recipe.yaml_content
        if self.extracted is None:
            self.extracted = buzzy.utils.tarball_basename(self.url)

        self.pkgbuild.list("source").append(self.url)
        self.pkgbuild.list("md5sums").append(self.md5)

        build = self.pkgbuild.code("build")
        build.append(BUILD_CD, 'cd "$srcdir/%s"' % self.extracted)

        package = self.pkgbuild.code("package")
        package.append(INSTALL_CD, 'cd "$srcdir/%s"' % self.extracted)



class Source(buzzy.recipe.TableVisitor):
    kind = "source"

    def __init__(self, recipe, pkgbuild):
        self.types = {
            "download": DownloadSource(recipe, pkgbuild),
        }
        super(Source, self).__init__()


#-----------------------------------------------------------------------
# Builders

class AutotoolsBuild(buzzy.recipe.Visitor):
    kind = "autotools build"

    def __init__(self, recipe, pkgbuild):
        self.recipe = recipe
        self.pkgbuild = pkgbuild
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


class Build(buzzy.recipe.TableVisitor):
    kind = "build"

    def __init__(self, recipe, pkgbuild):
        self.types = {
            "autotools": AutotoolsBuild(recipe, pkgbuild),
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
        self.installed = False
        self.build_path = build_path(self.name)
        pkgbuild_path = os.path.join(self.build_path, "PKGBUILD")
        self.pkgbuild = Pkgbuild(pkgbuild_path)

        # Nested visitors
        self.visitors = {}
        source = Source(recipe, self.pkgbuild)
        self.visitors["sources"] = buzzy.recipe.SeqForeach("sources", source)
        self.visitors["build"] = Build(recipe, self.pkgbuild)

        super(BuiltPackage, self).__init__()

    def package_filename(self):
        return "%s-%s-%s-%s.pkg.tar.xz" % \
            (self.name, self.recipe.version, self.recipe.revision, arch)

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
        self.pkgbuild.list("depends").extend(value, recipe_package_names)

    def map_build_depends(self, value):
        self.pkgbuild.list("makedepends").extend(value, recipe_package_names)

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
        log(1, "Creating PKGBUILD file")
        self.pkgbuild.write()

    def clean_build_path(self):
        def rmdir(path):
            buzzy.utils.run(["rm", "-rf", os.path.join(self.build_path, path)])
        def rm(path):
            try:
                os.remove(os.path.join(self.build_path, path))
            except OSError:
                pass
        log(1, "Cleaning build directory")
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
        if not self.installed:
            install_deps(self.recipe, "depends")
            self.build()
            log(0, "Installing built package %s" % self.name)
            self.installed = True


#-----------------------------------------------------------------------
# OS interface

class ArchLinux(object):
    name = "linux (arch)"
    arch = arch

    def __init__(self):
        self.installed = set()

    def install(self, recipe):
        install_recipe(recipe)
