# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "arch",
    "os",
    "os_family",
    "pkgdb",
    "version",
)

import os
pkgdb = os.getcwd()

import buzzy.version
version = buzzy.version.version

os = None
os_family = None
arch = None
