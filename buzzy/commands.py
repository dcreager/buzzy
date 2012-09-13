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
import logging
import optparse
import os.path
import sys

import buzzy.config
import buzzy.detect
from buzzy.errors import BuzzyError


global_options = optparse.OptionParser(
    prog="buzzy",
    version="%%prog %s" % buzzy.config.version,
)
global_options.disable_interspersed_args()
global_options.add_option(
    "-v", "--verbose",
    dest="verbosity", action="count", default=logging.INFO,
    help="output more detailed progres information",
)
global_options.add_option(
    "-d", "--package-database",
    dest="pkgdb", default=None,
    help="location of the package database", metavar="DIR",
)


def main(args):
    """
    The main entry point for the buzzy command-line program.
    """

    # Parse any global options
    (options, cmd_args) = global_options.parse_args(args)
    logging.basicConfig(format="%(message)s", level=options.verbosity)

    if options.pkgdb is not None:
        buzzy.config.pkgdb = os.path.abspath(options.pkgdb)

    # Detect the current OS
    buzzy.detect.detect_os()

    if len(cmd_args) == 0:
        # If no command was given, default to the "info" command.
        run_command("info", [])
    else:
        run_command(cmd_args[0], cmd_args[1:])


import buzzy.command.info
import buzzy.command.install
commands = {
    "info": buzzy.command.info.run,
    "install": buzzy.command.install.run,
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

    if hasattr(buzzy.config.os, "cmd_%s" % command_name):
        cmd = getattr(buzzy.config.os, "cmd_%s" % command_name)
        cmd(args)
        found = True

    if not found:
        raise BuzzyError("No command named %s" % command_name)
