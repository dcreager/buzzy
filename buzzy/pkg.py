# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "load",
)

import os.path
import sys
import yaml

import buzzy.config
from buzzy.errors import BuzzyError

packages = {}

def load(pkg_name):
    """
    Load in the package description for the package with the given name.
    """

    if pkg_name in packages:
        return packages[pkg_name]

    pkg_filename = os.path.join(buzzy.config.pkgdb, "%s.yaml" % pkg_name)
    try:
        pkg_file = open(pkg_filename, "r")
        pkg = yaml.safe_load(pkg_file)
    except IOError:
        raise BuzzyError("No package named %s" % pkg_name)
        sys.exit(1)

    if pkg['name'] != pkg_name:
        raise BuzzyError("Invalid package description: name must be %s" % pkg_name)
        sys.exit(1)

    packages[pkg_name] = pkg
    return pkg


def dependency_chain(pkg_names, depends_key):
    """
    Return a list of package objects, which will include all of the packages in
    pkg_names, as well as those packages' full dependency chains.  The
    depends_key parameter gives the name of the attribute in the package
    description that gives the list of dependencies.
    """

    started = set()
    finished = set()
    pkgs = []

    def visit(pkg_name):
        # If we've already finished processing this package, just return.
        if pkg_name in finished:
            return

        # If we've started processing this package, but haven't finished it,
        # then we've encountered a cycle in the dependency chain.
        if pkg_name in started:
            raise BuzzyError("Dependency chain when processing %s" % pkg_name)

        # Mark that we're starting to process this package.
        started.add(pkg_name)

        # Load in the package description.
        pkg = buzzy.pkg.load(pkg_name)

        # Process the dependencies first.
        if depends_key in pkg:
            for dep_pkg_name in pkg[depends_key]:
                visit(dep_pkg_name)

        # Add the package to the list.
        finished.add(pkg_name)
        pkgs.append(pkg)

    for pkg_name in pkg_names:
        visit(pkg_name)

    return pkgs
