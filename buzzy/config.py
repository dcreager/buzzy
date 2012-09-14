# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

import os as _os
from os import path

import buzzy.version

build_dir = path.abspath(".buzzy")
db = _os.getcwd()
os = None
verbosity = 0
version = buzzy.version.version
