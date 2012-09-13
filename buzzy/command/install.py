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
import buzzy.recipe
from buzzy.errors import BuzzyError

def install(recipe_names):
    recipes = buzzy.recipe.dependency_chain(recipe_names, "depends")
    buzzy.config.os.install(recipes)


def run(args):
    if not args:
        raise BuzzyError("Must provide at least one recipe name")
        sys.exit(1)

    install(args)
