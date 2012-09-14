# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "run",
)

import buzzy.config
import buzzy.recipe

def run(args):
    if len(args) == 0:
        print("buzzy %s" % buzzy.config.version)
        print()
        print("Package DB:  %s" % buzzy.config.db)
        print("OS:          %s" % buzzy.config.os.name)
        print("Arch:        %s" % buzzy.config.os.arch)
        return

    first = True
    for recipe_name in args:
        if first:
            first = False
        else:
            print("")

        recipe = buzzy.recipe.load(recipe_name)
        print("%s:" % recipe.name)

        if recipe.depends is not None:
            print("  Dependencies:")
            for dep in recipe.depends:
                print("    %s" % dep)

        if recipe.build_depends is not None:
            print("  Build dependencies:")
            for dep in recipe.build_depends:
                print("    %s" % dep)
