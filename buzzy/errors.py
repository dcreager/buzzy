# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import sys

class BuzzyError(Exception):
    def __init__(self, msg, detail=None):
        self.msg = msg
        self.detail = detail

    def __str__(self):
        return self.msg
