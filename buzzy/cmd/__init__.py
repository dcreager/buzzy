# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "main",
)

import optparse
import logging

from buzzy import config

global_options = optparse.OptionParser(
    prog="buzzy",
    version="%%prog %s" % config.version,
)
global_options.disable_interspersed_args()

def main(args):
    """
    The main entry point for the buzzy command-line program.
    """

    logging.basicConfig(format="%(message)s")
    (options, cmd_args) = global_options.parse_args(args)
