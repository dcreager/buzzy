# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "detect_os",
)

import os.path
import sys

import buzzy.config
from buzzy.errors import BuzzyError
from buzzy.os.arch import ArchLinux

def detect_os():
    # Is this Arch Linux?
    if os.path.exists("/etc/arch-release"):
        buzzy.config.os = ArchLinux()

    if buzzy.config.os is None:
        raise BuzzyError("Cannot determine which OS this is!")
        sys.exit(1)
