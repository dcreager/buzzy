# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "install",
    "run",
)

import sys

import buzzy.config
import buzzy.pkg
from buzzy.errors import BuzzyError

def install(pkg_names):
    pkgs = buzzy.pkg.dependency_chain(pkg_names, "depends")
    buzzy.config.os.install(pkgs)


def run(args):
    if not args:
        raise BuzzyError("Must provide at least one package name")
        sys.exit(1)

    install(args)
