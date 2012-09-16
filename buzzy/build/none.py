# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------


from __future__ import absolute_import

import buzzy.yaml

class NoBuild(buzzy.yaml.Fields):
    @classmethod
    def type_name(cls):
        return "none"
