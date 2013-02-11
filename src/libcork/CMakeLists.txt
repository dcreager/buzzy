# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2013, RedJack, LLC.
# All rights reserved.
#
# Please see the LICENSE.txt file in this distribution for license
# details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/lib/libcork/include)

#-----------------------------------------------------------------------
# Embedded libcork library

set(LIBCORK_SRC
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/cli/commands.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/allocator.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/error.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/gc.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/hash.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/ip-address.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/mempool.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/core/timestamp.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/array.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/buffer.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/dllist.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/file-stream.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/hash-table.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/managed-buffer.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/ring-buffer.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/ds/slice.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/posix/directory-walker.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/posix/process.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/posix/subprocess.c
    ${CMAKE_SOURCE_DIR}/lib/libcork/src/libcork/pthreads/thread.c
)

add_library(libcork STATIC ${LIBCORK_SRC})
target_link_libraries(libcork ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(libcork PROPERTIES
    OUTPUT_NAME cork
    COMPILE_DEFINITIONS CORK_API=CORK_LOCAL
)
