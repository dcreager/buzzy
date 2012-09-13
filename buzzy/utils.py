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

import subprocess
import sys

import buzzy.config
from buzzy.errors import BuzzyError
from buzzy.log import log

def _run(pretty_name, cmd, **kw):
    """
    Run a command in a subprocess, capturing the output.  If the process
    succeeds, don't print anything out.  If it fails, print out the output and
    raise an error.
    """

    log(0, "$ %s" % " ".join(cmd))

    try:
        if buzzy.config.verbosity >= 1:
            subprocess.call(cmd, **kw)
        else:
            subprocess.check_output(cmd, stderr=subprocess.STDOUT, **kw)
    except OSError as e:
        raise BuzzyError("Error running %s" % pretty_name)
    except subprocess.CalledProcessError as e:
        raise BuzzyError("Error running %s" % pretty_name, e.output)

def run(cmd, **kw):
    return _run(cmd[0], cmd, **kw)

def sudo(cmd, **kw):
    cmd.insert(0, "sudo")
    return _run(cmd[1], cmd, **kw)
