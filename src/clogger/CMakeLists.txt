# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2013, RedJack, LLC.
# All rights reserved.
#
# Please see the LICENSE.txt file in this distribution for license
# details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/lib/clogger/include)

#-----------------------------------------------------------------------
# Embedded clogger library

set(CLOG_SRC
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/filter.c
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/formatter.c
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/levels.c
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/logging.c
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/null.c
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/stack.c
    ${CMAKE_SOURCE_DIR}/lib/clogger/src/libclogger/stream.c
)

add_library(libclogger STATIC ${CLOG_SRC})
target_link_libraries(libclogger libcork)
set_target_properties(libclogger PROPERTIES
    OUTPUT_NAME clogger
    COMPILE_DEFINITIONS CORK_API=CORK_LOCAL
)
