# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import itertools
import os.path
import sys
import yaml

import buzzy.config
from buzzy.errors import BuzzyError
from buzzy.log import log

recipes = {}


#-----------------------------------------------------------------------
# Recipe objects

class Visitor(object):
    """
    Base class of all recipe visitors
    """

    def __init__(self):
        pass

    def scalar(self, value):
        """
        Called when the value of the current node is a scalar.
        """
        raise BuzzyError("Unexpected scalar value in %s" % self.kind)

    def seq(self, elements):
        """
        Called when the value of the current node is a sequence.
        """
        raise BuzzyError("Unexpected sequence value in %s" % self.kind)

    def map(self, elements):
        """
        Called when the value of the current node is a map.
        """
        raise BuzzyError("Unexpected map value in %s" % self.kind)

    def visit(self, value):
        log(3, "... %s %s" % (self.kind, value))
        self.yaml_content = value
        if isinstance(value, list):
            self.seq(value)
        elif isinstance(value, dict):
            self.map(value)
        else:
            self.scalar(value)


class SeqVisitor(Visitor):
    """
    A visitor implementation that knows how to iterate through the contents of a
    sequence, calling a method for each element.
    """

    def seq(self, elements):
        """
        The default implementation is to iterate through the contents of the
        sequence, calling the seq_element method for each one.
        """
        self.start_seq()
        for element in elements:
            self.seq_element(element)
        self.finish_seq()

    def start_seq(self):
        pass

    def finish_seq(self):
        pass


class SeqForeach(SeqVisitor):
    """
    A visitor implementation that applies a different visitor to each element in
    a sequence.
    """

    def __init__(self, kind, element_visitor):
        self.kind = kind
        self.element_visitor = element_visitor
        super(SeqVisitor, self).__init__()

    def seq_element(self, element):
        self.element_visitor.visit(element)


class MapVisitor(Visitor):
    """
    A visitor implementation that knows how to iterate through the contents of a
    map, calling methods for each key/value pair.
    """

    def map(self, elements):
        """
        The default implementation is to iterate through the contents of the
        map, calling the map_KEY method for each key/value pair.  If there is no
        map_KEY method for a key, we call the default_map_key method instead.
        """

        self.start_map()
        for key, value in elements.items():
            method_name = "map_%s" % key
            if hasattr(self, method_name):
                getattr(self, method_name)(value)
                continue

            else:
                self.default_map_key(key, value)
        self.finish_map()

    def start_map(self):
        pass

    def finish_map(self):
        pass

    def default_map_key(self, key, value):
        pass


class MapForeach(MapVisitor):
    """
    A visitor implementation that applies a different visitor to each element in
    a map.

    You must define a "visitors" field that maps key names to visitor objects.
    """

    def default_map_key(self, key, value):
        # Silently ignore unknown keys
        if key in self.visitors:
            self.visitors[key].visit(value)


class SchemaVisitor(Visitor):
    """
    A visitor implementation that knows how to fill in an expected set of
    fields.  Subclasses must define the following class attributes:

    kind: a string giving the kind of object that's being produced
    required: a list of fields
    optional: a list of fields


    The "required" and "optional" attributes will be used to initialize instance
    attributes.  Each element must either be a (yaml_field, python_field) tuple,
    or a simple string.  If it's a simple string, that value is used for both
    "yaml_field" and "python_field".  When the named yaml_field is encountered,
    its value will be used to fill in the named python_field.

    Both field names can contain nested dotted elements, such as "arch.native".
    We'll correctly parse (or create) the desired structure automatically.

    In a post-processing step, we'll verify that values were provided for each
    of the "required" fields.  Any "optional" fields that weren't provided will
    have None as their value.

    You can also provide a "process" method, which we'll call once the object
    has been filled in and validated.
    """

    class SimpleField(Visitor):
        def __init__(self, target, python_field, required):
            self.target = target
            self.python_field = python_field
            self.required = required
            self.value_provided = False

        def scalar(self, value):
            self.target.set_field(self.python_field, value)
            self.value_provided = True

        seq = scalar
        map = scalar

        def initialize(self):
            self.target.set_field(self.python_field, None)

        def finalize(self):
            if self.required and not self.value_provided:
                raise BuzzyError("Expected %s in %s" %
                        (self.full_yaml_field, self.target.kind))

    class NestedField(MapVisitor):
        def __init__(self, target):
            self.target = target
            self.fields = {}

        def add_field(self, name, field):
            if self.full_yaml_field == "":
                field.full_yaml_field = name
            else:
                field.full_yaml_field = "%s.%s" % (self.full_yaml_field, name)
            field.kind = field.full_yaml_field
            if name in self.fields:
                raise BuzzyError("Redefining field %s" % field.full_yaml_field)
            self.fields[name] = field

        def default_map_key(self, key, value):
            # Silently ignore any keys that don't correspond to expected fields.
            if key in self.fields:
                self.fields[key].visit(value)

        def initialize(self):
            for field in self.fields.values():
                field.initialize()

        def finalize(self):
            for field in self.fields.values():
                field.finalize()

    class Record(object):
        pass

    def __init__(self):
        self.root = self.NestedField(self)
        self.root.full_yaml_field = ""
        self.root.kind = self.kind

        for field_def in self.required:
            self.add_field(field_def, True)
        for field_def in self.optional:
            self.add_field(field_def, False)

    def add_field(self, field_def, required):
        if isinstance(field_def, str):
            full_yaml_name = field_def
            python_name = field_def
        else:
            full_yaml_name = field_def[0]
            python_name = field_def[1]

        yaml_parts = full_yaml_name.split(".")
        python_name = python_name.split(".")

        curr = self.root
        for yaml_part in yaml_parts[:-1]:
            if yaml_part in curr.fields:
                curr = curr.fields[yaml_part]
                if not isinstance(curr, self.NestedField):
                    raise BuzzyError("Conflicting definitions for %s" %
                                     full_yaml_name)
            else:
                field = self.NestedField(self)
                curr.add_field(yaml_part, field)
                curr = field

        field = self.SimpleField(self, python_name, required)
        curr.add_field(yaml_parts[-1], field)

    def set_field(self, python_name, value):
        curr = self
        for name in python_name[:-1]:
            if not hasattr(curr, name):
                setattr(curr, name, self.Record())
            curr = getattr(curr, name)
        setattr(curr, python_name[-1], value)

    def scalar(self, value):
        self.root.initialize()
        self.root.scalar(value)
        self.root.finalize()
        self.process()

    def list(self, value):
        self.root.initialize()
        self.root.list(value)
        self.root.finalize()
        self.process()

    def map(self, value):
        self.root.initialize()
        self.root.map(value)
        self.root.finalize()
        self.process()

    def process(self):
        pass


class TableVisitor(Visitor):
    """
    A common pattern is to have an element in a recipe that can come in multiple
    "types".  (Examples, the "sources" element contains a list, and each list
    element could be one of many different kinds of sources --- git, download,
    etc.)

    For these elements, the value can be a scalar, in which case that's the type
    name, or it can be a map, in which case there must be a "type" element
    giving the type name.  We then delegate to a type-specific Visitor instance
    to actually process the content.

    Subclasses must define a "types" attribute, which should be a dict mapping
    type names to the Visitor instance to process that type, and a "kind"
    attribute, which should be the string name of the generic category of item
    being processed.  (In the example above, that would be "source".)
    """

    def try_type(self, type_name, value):
        if type_name in self.types:
            self.types[type_name].visit(value)
        else:
            raise BuzzyError("Don't know how to process a %s %s" %
                             (type_name, self.kind))

    def scalar(self, value):
        self.try_type(value, value)

    def map(self, value):
        if "type" in value:
            self.try_type(value["type"], value)
        else:
            raise BuzzyError("Missing \"type\" attribute in %s" % self.kind)


#-----------------------------------------------------------------------
# Loading recipes from the package database

class Recipe(SchemaVisitor):
    kind = "recipe"
    required = [
        "package_name",
        "revision",
        "version",
    ]
    optional = [
        "build_depends",
        "depends",
        "description",
        "url",
    ]

    def __init__(self, recipe_name):
        self.recipe_name = recipe_name
        super(Recipe, self).__init__()

    def __contains__(self, key):
        try:
            parts = key.split(".")
            curr = self.yaml_content
            for part in parts:
                curr = curr[part]
            return True
        except KeyError:
            return False

    def __getitem__(self, key):
        parts = key.split(".")
        curr = self.yaml_content
        for part in parts:
            curr = curr[part]
        return curr


#-----------------------------------------------------------------------
# Loading recipes from the package database

def load(recipe_name):
    """
    Load in the recipe description for the recipe with the given name.
    """

    if recipe_name not in recipes:
        recipe_filename = os.path.join(buzzy.config.db, "%s.yaml" % recipe_name)
        try:
            recipe_file = open(recipe_filename, "r")
            content = yaml.safe_load(recipe_file)
        except IOError:
            raise BuzzyError("No recipe named %s" % recipe_name)

        recipe = Recipe(recipe_name)
        recipe.visit(content)
        recipes[recipe_name] = recipe

    return recipes[recipe_name]


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
