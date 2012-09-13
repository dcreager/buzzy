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
from buzzy.cmd import global_options

def run(args):
    global_options.print_version()
    print()
    print("Package DB:  %s" % config.pkgdb)
    print("OS:          %s (%s)" % (config.os_family, config.os))
    print("Arch:        %s" % config.arch)
