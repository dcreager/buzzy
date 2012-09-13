# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "global_options",
    "main",
    "run_command",
)

import importlib
import optparse
import logging
import sys

from buzzy import config
from buzzy import detect


global_options = optparse.OptionParser(
    prog="buzzy",
    version="%%prog %s" % config.version,
)
global_options.disable_interspersed_args()


def main(args):
    """
    The main entry point for the buzzy command-line program.
    """

    # Parse any global options
    logging.basicConfig(format="%(message)s")
    (options, cmd_args) = global_options.parse_args(args)

    # Detect the current OS
    detect.detect_os()

    if len(cmd_args) == 0:
        # If no command was given, default to the "info" command.
        run_command("info", [])
    else:
        run_command(cmd_args[0], cmd_args[1:])


import buzzy.command.info
commands = {
    "info": buzzy.command.info.run,
}

def run_command(command_name, args):
    # We can have several possible implementations of each command: a generic
    # one, which doesn't depend on the current OS, and implementations specific
    # to the current OS and OS family.  We want to run them all, in increasing
    # order of specificity.

    found = False

    if command_name in commands:
        commands[command_name](args)
        found = True

    if hasattr(config.os, "cmd_%s" % command_name):
        cmd = getattr(config.os, "cmd_%s" % command_name)
        cmd(args)
        found = True

    if not found:
        logging.error("No command named %s" % command_name)
        sys.exit(1)
