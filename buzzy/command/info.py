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

from buzzy import config

def run(args):
    print("buzzy %s" % config.version)
    print()
    print("Package DB:  %s" % config.pkgdb)
    print("OS:          %s" % config.os.name)
    print("Arch:        %s" % config.os.arch)
