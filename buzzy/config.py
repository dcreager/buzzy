# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "os",
    "db",
    "verbosity",
    "version",
)

import os as _os

import buzzy.version

db = _os.getcwd()
os = None
verbosity = 0
version = buzzy.version.version
