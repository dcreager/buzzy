# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import os.path
import sys
import yaml

import buzzy.utils
import buzzy.version
import buzzy.yaml
from buzzy.errors import BuzzyError


#-----------------------------------------------------------------------
# Command-line options

verbosity = 0
version = buzzy.version.version


#-----------------------------------------------------------------------
# Environment files

class EnvironmentVersion(buzzy.yaml.Fields):
    def ask(self, name, prompt, default=None):
        if not hasattr(self, name):
            while True:
                if default is None:
                    sys.stdout.write("%s " % prompt)
                else:
                    sys.stdout.write("%s [%s] " % (prompt, default))

                sys.stdout.flush()
                result = sys.stdin.readline().rstrip()
                if result == "":
                    if default is not None:
                        setattr(self, name, default)
                        return
                else:
                    setattr(self, name, result)
                    return

    @classmethod
    def copy(cls, prev):
        self = cls()
        for field_name, _ in prev.all_fields():
            setattr(self, field_name, getattr(prev, field_name))
        self.version = prev.version + 1
        return self


class Environment(object):
    env_path = ".buzzy"

    def is_current_version(self, version):
        return version == len(self.versions)-1

    def update(self):
        env_filename = os.path.join(self.env_path, self.env_filename)
        if os.path.exists(env_filename):
            env = self.load(False)
        else:
            env = self.versions[0]()
            env.version = 0

        while not self.is_current_version(env.version):
            next_class = self.versions[env.version + 1]
            env = next_class.upgrade_from(self, env)

        self.save(env)

    def load(self, check_version=True):
        env_filename = os.path.join(self.env_path, self.env_filename)

        try:
            env_file = open(env_filename, "r")
            content = yaml.safe_load(env_file)
        except IOError:
            raise BuzzyError("Please run \"buzzy configure\" first.")

        version = content["version"]
        if not self.is_current_version(version):
            raise BuzzyError \
                ("Outdated configuration, please rerun \"buzzy configure\".")

        env_class = self.versions[version]
        result = env_class.from_yaml(content, content)
        # Most of the time this will be already set, but let's make sure.
        result.version = version
        return result

    def save(self, env):
        env_class = self.versions[env.version]
        content = env_class.to_yaml(env)

        try:
            buzzy.utils.makedirs(self.env_path)
            env_filename = os.path.join(self.env_path, self.env_filename)
            env_file = open(env_filename, "w")
            yaml.dump(content, env_file, default_flow_style=False)
        except IOError as e:
            raise BuzzyError(str(e))


#-----------------------------------------------------------------------
# Default environment schema

class Environment_0(EnvironmentVersion):
    def __init__(self):
        self.version = 0


class Environment_1(EnvironmentVersion):
    def fields(self):
        yield "version"
        yield "build_dir"
        yield "package_dir"
        yield "recipe_database"

    @classmethod
    def upgrade_from(cls, env, prev):
        self = cls.copy(prev)
        self.ask("recipe_database", "Where is your recipe database?")
        self.ask("build_dir", "Where should I build packages?",
                 os.path.join(env.env_path, "build"))
        self.ask("package_dir", "Where should I put the packages that I build?",
                 os.path.join(env.env_path, "packages"))
        return self


class DefaultEnvironment(Environment):
    versions = [
        Environment_0,
        Environment_1,
    ]

    env_filename = "config.yaml"


env = None

def load_env(check_version=True):
    global env
    env = DefaultEnvironment().load(check_version)


def configure_env():
    DefaultEnvironment().update()
