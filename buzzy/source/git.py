# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2012, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------


import os.path
import re

import buzzy.utils
import buzzy.yaml


class Git(buzzy.yaml.Fields):
    def fields(self):
        yield "url", {"templated": True}
        yield "branch", {"default": None, "templated": True}
        yield "tag", {"default": None, "templated": True}

    def validate(self):
        self.commit = buzzy.utils.any_of("git source",
                                         branch=self.branch,
                                         tag=self.tag)
        self.dev_build = self.branch is not None
        self.repo_name = re.sub("\.git$", "", os.path.basename(self.url))
