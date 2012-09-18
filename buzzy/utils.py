# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from __future__ import absolute_import

import os
import os.path
import re
import shutil
import subprocess
import sys

import buzzy.config
from buzzy.errors import BuzzyError
from buzzy.log import log


def _if_run(pretty_name, cmd, **kw):
    """
    Run a command in a subprocess and return whether it exits successfully.
    Ignore any output.
    """

    log(1, "$ %s" % " ".join(cmd))

    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, **kw)
        return True
    except OSError:
        raise BuzzyError("Error running %s" % pretty_name)
    except subprocess.CalledProcessError:
        return False

def if_run(cmd, **kw):
    return _if_run(cmd[0], cmd, **kw)

def if_sudo(cmd, **kw):
    cmd.insert(0, "sudo")
    return _if_run(cmd[1], cmd, **kw)


def _run(pretty_name, cmd, **kw):
    """
    Run a command in a subprocess, capturing the output.  If the process
    succeeds, don't print anything out.  If it fails, print out the output and
    raise an error.
    """

    log(1, "$ %s" % " ".join(cmd))

    try:
        if buzzy.config.verbosity >= 1:
            subprocess.check_call(cmd, **kw)
        else:
            subprocess.check_output(cmd, stderr=subprocess.STDOUT, **kw)
    except OSError:
        raise BuzzyError("Error running %s" % pretty_name)
    except subprocess.CalledProcessError as e:
        raise BuzzyError("Error running %s" % pretty_name, e.output)

def run(cmd, **kw):
    return _run(cmd[0], cmd, **kw)

def sudo(cmd, **kw):
    cmd.insert(0, "sudo")
    return _run(cmd[1], cmd, **kw)


# A version of subprocess.check_output that always returns a str instance on
# both Python 2 and Python 3.
def check_output(*args, **kw):
    result = subprocess.check_output(*args, **kw)
    if bytes == str:
        # Python 2
        return str(result)
    else:
        # Python 3
        return str(result, "ascii")


def any_of(kind, **choices):
    choice_names = []
    given = []
    for k, v in choices.items():
        choice_names.append(k)
        if v is not None:
            given.append(k)

    if not given:
        choice_names.sort()
        raise BuzzyError("Must give one of (%s) for %s" %
                         (", ".join(choice_names), kind))
    elif len(given) == 1:
        return choices[given[0]]
    else:
        given.sort()
        raise BuzzyError("Cannot give (%s) together for %s" %
                         (", ".join(choice_names), kind))


def extract_attrs(obj, names=None):
    if names is None:
        names = []
        for name in dir(obj):
            if not name.startswith("_") and not callable(getattr(obj, name)):
                names.append(name)
    return { name: getattr(obj, name) for name in names }


def get_arch_from_uname():
    return check_output(["uname", "-m"]).rstrip()


def makedirs(path):
    if not os.path.isdir(path):
        os.makedirs(path)

def rmtree(path):
    shutil.rmtree(path, ignore_errors=True)

def rm(path):
    try:
        os.remove(path)
    except OSError:
        pass


TARBALL_EXTS = re.compile(r"""
    (
        ((\.tar)?(\.bz2|\.gz|\.xz|\.Z)?)
      | (\.zip)
    )$
""", re.VERBOSE)

def tarball_basename(path):
    return TARBALL_EXTS.sub("", os.path.basename(path))


def trim(docstring):
    """From PEP 257"""
    if not docstring:
        return ''
    # Convert tabs to spaces (following the normal Python rules)
    # and split into a list of lines:
    lines = docstring.expandtabs().splitlines()
    # Determine minimum indentation (first line doesn't count):
    indent = None
    for line in lines[1:]:
        stripped = line.lstrip()
        if stripped:
            this_indent = len(line) - len(stripped)
            if indent is None:
                indent = this_indent
            else:
                indent = min(indent, this_indent)
    # Remove indentation (first line is special):
    trimmed = [lines[0].strip()]
    if indent is not None:
        for line in lines[1:]:
            trimmed.append(line[indent:].rstrip())
    # Strip off trailing and leading blank lines:
    while trimmed and not trimmed[-1]:
        trimmed.pop()
    while trimmed and not trimmed[0]:
        trimmed.pop(0)
    # Return a single string:
    return '\n'.join(trimmed)
