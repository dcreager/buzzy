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


def run_command(command_name, args):
    # We can have several possible implementations of each command: a generic
    # one, which doesn't depend on the current OS, and implementations specific
    # to the current OS and OS family.  We want to run them all, in increasing
    # order of specificity.

    success = False
    if try_command("common", command_name, args):
        success = True

    if not success:
        logging.error("No command named %s" % command_name)
        sys.exit(1)


def try_command(flavor, command_name, args):
    # Try to load the module that implements this command.
    mod_name = "buzzy.%s.cmd.%s" % (flavor, command_name)
    try:
        mod = importlib.import_module(mod_name)
    except ImportError:
        # If the module doesn't exist, don't do anything
        return False

    # Then try to run it.
    mod.run(args)
    return True
