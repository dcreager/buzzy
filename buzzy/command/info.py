# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "run",
)

import buzzy.config

def run(args):
    print("buzzy %s" % buzzy.config.version)
    print()
    print("Package DB:  %s" % buzzy.config.db)
    print("OS:          %s" % buzzy.config.os.name)
    print("Arch:        %s" % buzzy.config.os.arch)
