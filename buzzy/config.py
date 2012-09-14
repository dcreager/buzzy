# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import os as _os
from os import path
import sys
import yaml

import buzzy.utils
import buzzy.version
from buzzy.errors import BuzzyError


#-----------------------------------------------------------------------
# Command-line options

os = None
verbosity = 0
version = buzzy.version.version


#-----------------------------------------------------------------------
# Environment files

class Environment(object):
    def __init__(self, yaml_content):
        self.yaml_content = yaml_content

    def __getattr__(self, name):
        try:
            return self.yaml_content[name]
        except KeyError:
            raise BuzzyError("Missing configuration variable %s" % name)

    def __contains__(self, name):
        return name in self.yaml_content

    def set(self, name, value):
        self.yaml_content[name] = value


env = Environment({})
env_path = ".buzzy"
env_filename = path.join(env_path, "config.yaml")

def load_env():
    try:
        env_file = open(env_filename, "r")
        content = yaml.safe_load(env_file)
    except IOError:
        raise BuzzyError("Please run \"buzzy configure\" first.")

    global env
    env = Environment(content)


def ask(name, prompt, default=None):
    if name not in env:
        while True:
            if default is None:
                sys.stdout.write("%s " % prompt)
            else:
                sys.stdout.write("%s [%s] " % (prompt, default))

            sys.stdout.flush()
            result = sys.stdin.readline().rstrip()
            if result == "":
                if default is not None:
                    env.set(name, default)
                    return
            else:
                env.set(name, result)
                return


def new_env():
    if path.exists(env_filename):
        load_env()

    ask("recipe_database", "Where is your recipe database?")
    ask("build_dir", "Where should I build packages?",
        path.join(env_path, "build"))
    ask("package_dir", "Where should I put the packages that I build?",
        path.join(env_path, "packages"))

    # Allow the OS-specific code to configure things, too.
    os.configure()

    try:
        buzzy.utils.makedirs(env_path)
        env_file = open(env_filename, "w")
        yaml.dump(env.yaml_content, env_file, default_flow_style=False)
    except IOError as e:
        raise BuzzyError(str(e))
