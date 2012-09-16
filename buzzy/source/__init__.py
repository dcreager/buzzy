# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------


from __future__ import absolute_import

import buzzy.yaml

class Source(buzzy.yaml.Types):
    types = {}

class SourceList(buzzy.yaml.Sequence):
    element_class = Source

def add(cls):
    Source.types[cls.type_name()] = cls
