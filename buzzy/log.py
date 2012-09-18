# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import sys
import traceback

import buzzy.config
from buzzy.errors import BuzzyError

if bytes == str:
    # Python 2
    def log(verbosity, *args):
        if verbosity <= buzzy.config.verbosity:
            for arg in args:
                sys.stderr.write(str(arg))
            sys.stderr.write("\n")
else:
    # Python 3
    def log(verbosity, *args):
        if verbosity <= buzzy.config.verbosity:
            for arg in args:
                if isinstance(arg, bytes):
                    sys.stderr.buffer.write(arg)
                else:
                    sys.stderr.write(str(arg))
            sys.stderr.write("\n")

def log_error(error):
    if isinstance(error, BuzzyError):
        log(-2, error.msg)
        if error.detail is not None:
            log(-2, error.detail)
    elif isinstance(error, OSError):
        log(-2, error)
    else:
        traceback.print_exc()
