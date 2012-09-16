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
import os.path
import sys

import buzzy.config
import buzzy.distro
from buzzy.errors import BuzzyError


global_options = optparse.OptionParser(
    prog="buzzy",
    version="%%prog %s" % buzzy.config.version,
)
global_options.disable_interspersed_args()
global_options.add_option(
    "-v", "--verbose",
    dest="verbosity", action="count", default=0,
    help="output more detailed progress information",
)


def main(args):
    """
    The main entry point for the buzzy command-line program.
    """

    # Parse any global options
    (options, cmd_args) = global_options.parse_args(args)
    buzzy.config.verbosity = options.verbosity

    # If no command was given, default to the "info" command.
    if len(cmd_args) == 0:
        cmd_name = "info"
    else:
        cmd_name = cmd_args[0]
        cmd_args = cmd_args[1:]

    # Detect the current OS distribution
    buzzy.distro.detect()

    # Run the command
    run_command(cmd_name, cmd_args)


import buzzy.command.build
import buzzy.command.configure
import buzzy.command.info
import buzzy.command.install
commands = {
    "build": buzzy.command.build.run,
    "configure": buzzy.command.configure.run,
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

    if hasattr(buzzy.distro.this, "cmd_%s" % command_name):
        cmd = getattr(buzzy.distro.this, "cmd_%s" % command_name)
        cmd(args)
        found = True

    if not found:
        raise BuzzyError("No command named %s" % command_name)
