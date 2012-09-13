# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "ArchLinux",
)

import buzzy.os.utils
import buzzy.utils
from buzzy.log import log

class ArchLinux(object):
    name = "linux (arch)"
    arch = buzzy.os.utils.get_arch_from_uname()

    def __init__(self):
        self.installed = set()

    # Returns a list of native package names for a recipe.
    def native_packages(self, recipe):
        try:
            native = recipe["arch"]["native"]
            if isinstance(native, str):
                return [native]
            else:
                return native
        except KeyError:
            return []

    def install(self, recipes):
        native_packages = []
        built_recipes = []

        # Install all of the native packages first.
        for recipe in recipes:
            native = self.native_packages(recipe)
            native_packages.extend(native)
            if not native:
                built_recipes.append(recipe)

        if native_packages:
            log(0, "Installing native packages")
            cmd = ["pacman", "-S", "--noconfirm"]
            cmd.extend(native_packages)
            buzzy.utils.sudo(cmd)

        for recipe in built_recipes:
            log(0, "Installing built recipe %s" % recipe["name"])
