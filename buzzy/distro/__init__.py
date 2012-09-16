# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from buzzy.distro.arch import ArchLinux
from buzzy.errors import BuzzyError

distros = [
    ArchLinux,
]

this = None

def detect():
    global this
    for distro in distros:
        this = distro.detect()
        if this is not None:
            return

    raise BuzzyError("Cannot determine which OS distribution this is!")
