# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------


from buzzy.errors import BuzzyError


class FromYaml(object):
    def __init__(self, *args, **kw):
        pass

    @classmethod
    def type_name(cls):
        return cls.__name__.lower()

    @classmethod
    def from_yaml(cls, yaml, root, *args, **kw):
        self = cls(*args, **kw)
        self.load_yaml(yaml, root)
        return self

    def load_yaml(self, yaml, root):
        pass


class Fields(FromYaml):
    def all_fields(self):
        while hasattr(self, "from_yaml"):
            if hasattr(self, "fields"):
                for x in self.fields():
                    yield x
            self = super(self.__class__, self)

    def validate(self):
        pass

    def set_fields_from_yaml(self, yaml, root):
        # Use an empty set of YAML content if none was given.
        yaml = yaml or {}

        # Extract fields from the YAML content.
        for tup in self.all_fields():
            if isinstance(tup, tuple):
                field, options = tup
            else:
                field = tup
                options = {}

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

        # Let the class perform any additional validation.
        self.validate()

    def load_yaml(self, yaml, root):
        super(Fields, self).load_yaml(yaml, root)
        self.set_fields_from_yaml(yaml, root)


class Types(FromYaml):
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
            kw = {}

        else:
            print(yaml)
            raise BuzzyError("Expected a string or map for %s" % cls.type_name())

        try:
            type_class = cls.types[type_name]
        except KeyError:
            raise BuzzyError("Don't know how to process a %s %s" %
                             (type_name, cls.type_name()))

        return type_class.from_yaml(yaml, root)


class Sequence(FromYaml):
    @classmethod
    def from_yaml(cls, yaml, root):
        if isinstance(yaml, list):
            return map(lambda x: cls.element_class.from_yaml(x, root), yaml)
        else:
            raise BuzzyError("Expected a list for %s" % cls.type_name())
