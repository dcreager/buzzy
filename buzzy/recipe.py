# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "load",
)

import os.path
import sys
import yaml

import buzzy.config
from buzzy.errors import BuzzyError

recipes = {}

def load(recipe_name):
    """
    Load in the recipe description for the recipe with the given name.
    """

    if recipe_name in recipes:
        return recipes[recipe_name]

    recipe_filename = os.path.join(buzzy.config.db, "%s.yaml" % recipe_name)
    try:
        recipe_file = open(recipe_filename, "r")
        recipe = yaml.safe_load(recipe_file)
    except IOError:
        raise BuzzyError("No recipe named %s" % recipe_name)
        sys.exit(1)

    if recipe['name'] != recipe_name:
        raise BuzzyError("Invalid recipe description: name must be %s" %
                         recipe_name)
        sys.exit(1)

    recipes[recipe_name] = recipe
    return recipe


def dependency_chain(recipe_names, depends_key):
    """
    Return a list of recipe objects, which will include all of the recipes in
    recipe_names, as well as those recipes' full dependency chains.  The
    depends_key parameter gives the name of the attribute in the recipe
    description that gives the list of dependencies.
    """

    started = set()
    finished = set()
    recipes = []

    def visit(recipe_name):
        # If we've already finished processing this recipe, just return.
        if recipe_name in finished:
            return

        # If we've started processing this recipe, but haven't finished it,
        # then we've encountered a cycle in the dependency chain.
        if recipe_name in started:
            raise BuzzyError("Dependency chain when processing %s" % recipe_name)

        # Mark that we're starting to process this recipe.
        started.add(recipe_name)

        # Load in the recipe description.
        recipe = buzzy.recipe.load(recipe_name)

        # Process the dependencies first.
        if depends_key in recipe:
            for dep_recipe_name in recipe[depends_key]:
                visit(dep_recipe_name)

        # Add the recipe to the list.
        finished.add(recipe_name)
        recipes.append(recipe)

    for recipe_name in recipe_names:
        visit(recipe_name)

    return recipes
