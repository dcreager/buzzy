# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------


from __future__ import absolute_import

from buzzy.errors import BuzzyError


class Yaml(object):
    def __init__(self, *args, **kw):
        pass

    @classmethod
    def type_name(cls):
        return cls.__name__.lower()

    @classmethod
    def from_yaml(cls, yaml, root, *args, **kw):
        self = cls(*args, **kw)
        self.load_yaml(yaml, root)
        self.validate()
        return self

    def load_yaml(self, yaml, root):
        pass

    def validate(self):
        pass

    @classmethod
    def to_yaml(cls, self):
        raise NotImplementedError


class Fields(Yaml):
    def all_fields(self):
        while hasattr(self, "from_yaml"):
            if hasattr(self, "fields"):
                for tup in self.fields():
                    if isinstance(tup, tuple):
                        yield tup
                    else:
                        yield tup, {}
            self = super(self.__class__, self)

    def __init__(self, *args, **kw):
        super(Fields, self).__init__(*args, **kw)
        for field, _ in self.all_fields():
            if not hasattr(self, field):
                setattr(self, field, None)

    def load_yaml(self, yaml, root):
        # Use an empty set of YAML content if none was given.
        yaml = yaml or {}

        # Extract fields from the YAML content.
        for field, options in self.all_fields():
            # Extract the value from the YAML, if present.
            value = None
            if field in yaml:
                value = yaml[field]
            elif "default" in options:
                value = options["default"]
            else:
                raise BuzzyError("Expected %s in %s" % (field, self.type_name()))

            # Delegate to a custom class, if requested.
            if "custom" in options:
                value = options["custom"].from_yaml(value, root)

            # Substitute in value references if the field is marked as
            # templated.
            if "templated" in options and value is not None:
                value = value % root

            setattr(self, field, value)

    @classmethod
    def to_yaml(cls, self):
        yaml = {}

        # Append fields into the YAML content.
        for field, options in self.all_fields():
            skip = False
            value = getattr(self, field)

            if "custom" in options:
                value = options["custom"].to_yaml(value)

            if "default" in options:
                if value == options["default"]:
                    # Skip this field if it has the default value.
                    skip = True

            if not skip:
                yaml[field] = value

        return yaml


class Types(Yaml):
    @classmethod
    def from_yaml(cls, yaml, root):
        if isinstance(yaml, dict):
            try:
                type_name = yaml["type"]
                kw = yaml
            except KeyError:
                raise BuzzyError("Missing \"type\" in %s" % cls.type_name())

        elif isinstance(yaml, str):
            type_name = yaml
            kw = {"type": type_name}

        else:
            print(yaml)
            raise BuzzyError("Expected a string or map for %s" % cls.type_name())

        try:
            type_class = cls.types[type_name]
        except KeyError:
            raise BuzzyError("Don't know how to process a %s %s" %
                             (type_name, cls.type_name()))

        return type_class.from_yaml(yaml, root)

    @classmethod
    def to_yaml(cls, self):
        type_class = cls.types[self.type]
        yaml = type_class.to_yaml(self)
        if "type" not in yaml:
            raise BuzzyError("Missing \"type\" in %s" % cls.type_name())

        if len(yaml) == 1:
            # If there's only a "type" field, render the object as a string
            # instead of a map.
            return yaml["type"]
        else:
            return yaml


class Sequence(Yaml):
    @classmethod
    def from_yaml(cls, yaml, root):
        if isinstance(yaml, list):
            return map(lambda x: cls.element_class.from_yaml(x, root), yaml)
        else:
            raise BuzzyError("Expected a list for %s" % cls.type_name())

    @classmethod
    def to_yaml(cls, self):
        return map(lambda x: cls.element_class.to_yaml(x), self);
