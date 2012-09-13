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

import logging
import os.path
import sys

from buzzy import config
from buzzy.os.arch import ArchLinux

def detect_os():
    # Is this Arch Linux?
    if os.path.exists("/etc/arch-release"):
        config.os = ArchLinux()

    if config.os is None:
        logging.error("Cannot determine which OS this is!")
        sys.exit(1)
