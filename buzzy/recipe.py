# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import collections
import itertools
import os
import os.path
import yaml

import buzzy.build
import buzzy.config
import buzzy.source
import buzzy.yaml
from buzzy.errors import BuzzyError
from buzzy.log import log


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
        self._packages = None
        super(Recipe, self).__init__()

    def create_packages(self):
        # The builder contains most of the logic for deciding what kind of
        # package this is.  We also let the source objects modify the package
        # that was built.
        packages = self.build.create_packages(self)
        for source in self.sources:
            source.update_packages(self, packages)
        return packages

    def packages(self):
        if self._packages is None:
            self._packages = self.create_packages()
        return self._packages

    def packages_with_tag(self, tag=None):
        result = []
        found_package = False

        # If a specific tag was requested, look for any packages with that tag
        # first.
        if tag is not None:
            for package in self.packages():
                if tag in package.tags:
                    result.append(package)
                    found_package = True

        # If we couldn't find any packages with the requested tag, fall back on
        # looking for a "default" package.
        if not found_package:
            for package in self.packages():
                if "default" in package.tags:
                    result.append(package)
                    found_package = True

        # If we still haven't found a package, that's an error.
        if found_package:
            return result
        else:
            raise BuzzyError("Can't find %s package for %s" %
                             (tag or "default", self.recipe_name))

    def build_recipe(self, force):
        for package in self.packages():
            package.build_package(force)

    def install_recipe(self, force):
        for package in self.packages():
            package.install_package(force)


#-----------------------------------------------------------------------
# Packages

class Package(object):
    def __init__(self, package_name, recipe, tags, dep_tag=None):
        log(2, "Creating package %s %s" % (package_name, tags))
        self.package_name = package_name
        self.recipe = recipe
        self.full_name = "%s: %s" % (self.recipe.recipe_name, package_name)
        self.tags = tags
        self.depends = self.calculate_deps(recipe.depends, dep_tag)
        self.build_depends = self.calculate_deps(recipe.build_depends, dep_tag)

    def calculate_deps(self, dep_recipes, dep_tag=None):
        result = []
        for dep_recipe_name in dep_recipes:
            dep_recipe = load(dep_recipe_name)
            result.extend(dep_recipe.packages_with_tag(dep_tag))
        return result

    def built(self):
        raise NotImplementedError

    def installed(self):
        raise NotImplementedError

    def perform_build(self):
        raise NotImplementedError

    def perform_install(self):
        raise NotImplementedError

    def build_package(self, force):
        if not force and self.built():
            return
        else:
            log(0, "[%s] Installing build dependencies" % self.full_name)
            for package in self.build_depends:
                package.install_package(buzzy.config.force_all)
            log(0, "[%s] Building" % self.full_name)
            self.perform_build()

    def install_package(self, force):
        if not force and self.installed():
            return
        else:
            log(0, "[%s] Installing run-time dependencies" % self.full_name)
            for package in self.depends:
                package.install_package(buzzy.config.force_all)
            self.build_package(force)
            log(0, "[%s] Installing" % self.full_name)
            self.perform_install()


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
