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

    # Returns a list of native package names for pkg.
    def native_packages(self, pkg):
        try:
            native = pkg["arch"]["native"]
            if isinstance(native, str):
                return [native]
            else:
                return native
        except KeyError:
            return []

    def install(self, pkgs):
        native_packages = []
        built_packages = []

        # Install all of the native packages first.
        for pkg in pkgs:
            native = self.native_packages(pkg)
            native_packages.extend(native)
            if not native:
                built_packages.append(pkg)

        if native_packages:
            log(0, "Installing native packages")
            cmd = ["pacman", "-S", "--noconfirm"]
            cmd.extend(native_packages)
            buzzy.utils.sudo(cmd)

        for pkg in built_packages:
            log(0, "Installing built package %s" % pkg["name"])
