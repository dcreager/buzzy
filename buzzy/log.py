# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "log",
)

import sys

import buzzy.config

def log(verbosity, *args):
    if verbosity <= buzzy.config.verbosity:
        for arg in args:
            if isinstance(arg, bytes):
                sys.stderr.buffer.write(arg)
            else:
                sys.stderr.write(str(arg))
        sys.stderr.write("\n")
