# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2011-2013, RedJack, LLC.
# All rights reserved.
#
# Please see the LICENSE.txt file in this distribution for license
# details.
# ----------------------------------------------------------------------

include_directories(../include)

#-----------------------------------------------------------------------
# buzzy

set(BUZZY_SRC
    buzzy/foo.c
)

add_executable(buzzy ${BUZZY_SRC})
install(TARGETS buzzy DESTINATION bin)
