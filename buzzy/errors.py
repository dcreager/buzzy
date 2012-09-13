# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import sys

from buzzy.log import log

class BuzzyError(Exception):
    def __init__(self, msg, detail=None):
        self.msg = msg
        self.detail = detail

    def __str__(self):
        return self.msg

    def print(self):
        log(-2, self.msg)
        if self.detail is not None:
            log(-2, self.detail)
