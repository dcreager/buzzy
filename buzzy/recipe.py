# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import itertools
import os
import os.path
import yaml

import buzzy.build
import buzzy.config
import buzzy.source
import buzzy.yaml
from buzzy.errors import BuzzyError


#-----------------------------------------------------------------------
# Recipe objects

class Recipe(buzzy.yaml.Fields):
    def fields(self):
        yield "license"
        yield "revision"
        yield "version"
        yield "build_depends", {"default": []}
        yield "depends", {"default": []}
        yield "description", {"default": ""}
        yield "license_file", {"default": None}
        yield "url", {"default": None}
        yield "build", {"custom": buzzy.build.Build, "default": "none"}
        yield "sources", {"custom": buzzy.source.SourceList, "default": []}

    def __init__(self, recipe_name):
        self.recipe_name = recipe_name
        super(Recipe, self).__init__()


#-----------------------------------------------------------------------
# Loading recipes from the package database

recipes = {}
recipe_class = Recipe

def load(recipe_name):
    """
    Load in the recipe description for the recipe with the given name.
    """

    if recipe_name not in recipes:
        recipe_filename = os.path.join(buzzy.config.env.recipe_database,
                                       "%s.yaml" % recipe_name)
        try:
            recipe_file = open(recipe_filename, "r")
            content = yaml.safe_load(recipe_file)
        except IOError:
            raise BuzzyError("No recipe named %s" % recipe_name)

        recipe = recipe_class.from_yaml(content, content, recipe_name)
        recipes[recipe_name] = recipe

    return recipes[recipe_name]

def all_recipe_names():
    db_path = buzzy.config.env.recipe_database
    prefix = "%s%s" % (db_path, os.path.sep)
    for dirpath, _, filenames in os.walk(db_path):
        for filename in filenames:
            if filename.endswith(".yaml"):
                recipe_name = "%s/%s" % (dirpath, filename[:-5])
                recipe_name = recipe_name.replace(prefix, "", 1)
                yield recipe_name


#-----------------------------------------------------------------------
# Calculating dependency chains

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
