# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------


from __future__ import absolute_import

import buzzy.utils
import buzzy.yaml

class Download(buzzy.yaml.Fields):
    def fields(self):
        yield "type"
        yield "url", {"templated": True}
        yield "md5"
        yield "sha1"
        yield "extracted", {"default": None}

    def update_packages(self, recipe, packages):
        pass

    def validate(self):
        if self.extracted is None:
            self.extracted = buzzy.utils.tarball_basename(self.url)
