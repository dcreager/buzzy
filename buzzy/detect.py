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
import subprocess
import sys

from buzzy import config

def get_arch_from_uname():
    return subprocess.check_output(["uname", "-m"]).decode("ascii").rstrip()

def detect_os():
    # Is this Arch Linux?
    if os.path.exists("/etc/arch-release"):
        config.os_family = "linux"
        config.os = "arch"
        config.arch = get_arch_from_uname()

    if config.os is None:
        logging.error("Cannot determine which OS this is!")
        sys.exit(1)
