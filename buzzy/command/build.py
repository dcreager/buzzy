# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import optparse
import sys

import buzzy.config
import buzzy.distro
import buzzy.recipe
from buzzy.errors import BuzzyError


cmd_options = optparse.OptionParser(
    prog="buzzy build",
)
cmd_options.add_option(
    "-f", "--force",
    dest="force", action="store_true", default=False,
    help="force package to be built",
)
cmd_options.add_option(
    "--force-all",
    dest="force_all", action="store_true", default=False,
    help="force all dependent packages to be built",
)

def run(args):
    if not args:
        raise BuzzyError("Must provide at least one recipe name")
        sys.exit(1)

    buzzy.config.load_env()
    (options, args) = cmd_options.parse_args(args)
    buzzy.config.force = options.force or options.force_all
    buzzy.config.force_all = options.force_all

    for recipe_name in args:
        recipe = buzzy.recipe.load(recipe_name)
        buzzy.distro.this.build(recipe)
