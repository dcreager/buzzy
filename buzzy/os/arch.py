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

import logging

import buzzy.os.utils

class ArchLinux(object):
    name = "linux (arch)"
    arch = buzzy.os.utils.get_arch_from_uname()

    def __init__(self):
        self.installed = set()

    def install(self, pkgs):
        for pkg in pkgs:
            logging.info("Installing %s" % pkg["name"])
