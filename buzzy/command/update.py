# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import optparse
import re

import buzzy.config
import buzzy.recipe
from buzzy.errors import BuzzyError


cmd_options = optparse.OptionParser(
    prog="buzzy update",
)
cmd_options.add_option(
    "-f", "--force",
    dest="force", action="store_true", default=False,
    help="force packages to be rebuilt",
)

def run(args):
    buzzy.config.load_env()
    (options, args) = cmd_options.parse_args(args)
    buzzy.config.force = options.force
    buzzy.config.force_all = options.force

    for recipe_name in buzzy.recipe.all_recipe_names():
        recipe = buzzy.recipe.load(recipe_name)
        buzzy.distro.this.install(recipe)
