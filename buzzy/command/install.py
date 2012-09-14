# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import sys

import buzzy.config
import buzzy.recipe
from buzzy.errors import BuzzyError


def run(args):
    if not args:
        raise BuzzyError("Must provide at least one recipe name")
        sys.exit(1)

    for recipe_name in args:
        recipe = buzzy.recipe.load(recipe_name)
        buzzy.config.os.install(recipe)
