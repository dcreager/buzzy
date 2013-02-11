# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2011-2013, RedJack, LLC.
# All rights reserved.
#
# Please see the LICENSE.txt file in this distribution for license
# details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libcork/include)

add_subdirectory(libcork)
add_subdirectory(libyaml)

#-----------------------------------------------------------------------
# buzzy

set(BUZZY_SRC
    buzzy/foo.c
)

add_executable(buzzy ${BUZZY_SRC})
set_target_properties(buzzy PROPERTIES
    COMPILE_DEFINITIONS CORK_API=CORK_LOCAL
)
target_link_libraries(buzzy libcork libyaml)
install(TARGETS buzzy DESTINATION bin)
