# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

from distutils.core import setup
from version import get_git_version

setup(
    name='buzzy',
    version=get_git_version(),
    description="OS-agnostic package builder",
    author="Douglas Creager",
    author_email="douglas.creager@redjack.com",
    url="https://github.com/redjack/buzzy/",
    packages=['buzzy'],
    requires=["PyYAML"],
)
