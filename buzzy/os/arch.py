# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

__all__ = (
    "ArchLinux",
)

from buzzy.os.utils import get_arch_from_uname

class ArchLinux(object):
    name = "linux (arch)"
    arch = get_arch_from_uname()
