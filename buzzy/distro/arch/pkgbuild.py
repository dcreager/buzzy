# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

__all__ = (
    "BUILD_CD",
    "BUILD_UNPACK",
    "BUILD_PRE",
    "BUILD_CONFIGURE",
    "BUILD_MAKE",
    "BUILD_POST",
    "INSTALL_CD",
    "INSTALL_PRE",
    "INSTALL_STAGE",
    "INSTALL_POST",
    "Pkgbuild",
)

import collections
import os
import os.path

import buzzy.utils


#-----------------------------------------------------------------------
# PKGBUILD file

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
    def __init__(self, package):
        self.package = package
        self.recipe = package.recipe
        self.build_path = package.build_path
        self.path = os.path.join(self.build_path, "PKGBUILD")
        self.producers = []
        self.content = {}

    def add_producer(self, producer):
        self.producers.append(producer)

    def make(self, package):
        for producer in self.producers:
            producer.make_pkgbuild(package, self)

    def write(self):
        buzzy.utils.makedirs(self.build_path)
        out = open(self.path, "w")
        for var_name in sorted(self.content.keys()):
            self.content[var_name].output(out)
        out.close()


    # A bunch of different kinds of PKGBUILD variables

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

    def scalar(self, name):
        if name not in self.content:
            self.content[name] = self.ScalarVariable(name)
        return self.content[name]


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

    def list(self, name):
        if name not in self.content:
            self.content[name] = self.ListVariable(name)
        return self.content[name]


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

    def code(self, name):
        if name not in self.content:
            self.content[name] = self.CodeVariable(name)
        return self.content[name]
