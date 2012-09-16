# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import buzzy.config
import buzzy.distro

def run(args):
    buzzy.config.configure_env()
    # Allow the OS-specific code to configure things, too.
    buzzy.distro.this.configure()
