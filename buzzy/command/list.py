# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import re

import buzzy.config
import buzzy.recipe
from buzzy.errors import BuzzyError

def run(args):
    buzzy.config.load_env()

    if not args:
        recipe_names = list(buzzy.recipe.all_recipe_names())
    elif len(args) == 1:
        filter_re = re.compile(args[0])
        recipe_names = [name for name in buzzy.recipe.all_recipe_names()
                        if filter_re.search(name)]
    else:
        raise BuzzyError("Cannot provide more than one filter expression")

    recipe_names.sort()
    for recipe_name in recipe_names:
        print(recipe_name)
