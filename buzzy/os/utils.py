# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "get_arch_from_uname",
)

import subprocess

def get_arch_from_uname():
    return subprocess.check_output(["uname", "-m"]).decode("ascii").rstrip()
