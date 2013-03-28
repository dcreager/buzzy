# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2011-2013, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libcork/include)

add_subdirectory(libcork)
add_subdirectory(libyaml)

#-----------------------------------------------------------------------
# libbuzzy

set(LIBBUZZY_RAGEL
    libbuzzy/dependency.c.rl
    libbuzzy/interpolated.c.rl
    libbuzzy/version.c.rl
    libbuzzy/distro/arch.c.rl
)

set_property(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY CLEAN_NO_CUSTOM true
)

foreach(RAGEL_INPUT ${LIBBUZZY_RAGEL})
    string(REGEX REPLACE "\\.rl$" "" RAGEL_OUTPUT "${RAGEL_INPUT}")
    if (RAGEL)
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${RAGEL_OUTPUT}"
            COMMAND ragel -G2 -o "${RAGEL_OUTPUT}" "${RAGEL_INPUT}"
            MAIN_DEPENDENCY "${RAGEL_INPUT}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Ragel ${RAGEL_INPUT}"
        )
    else (RAGEL)
        # Ragel is not present; if the Ragel source files get out of date,
        # that's an error.
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${RAGEL_OUTPUT}"
            COMMAND false
            MAIN_DEPENDENCY "${RAGEL_INPUT}"
            COMMENT "Ragel not found; cannot update ${RAGEL_OUTPUT}"
        )
    endif (RAGEL)
endforeach(RAGEL_INPUT)

set(LIBBUZZY_SRC
    libbuzzy/action.c
    libbuzzy/dependency.c
    libbuzzy/env.c
    libbuzzy/interpolated.c
    libbuzzy/mock.c
    libbuzzy/native.c
    libbuzzy/os.c
    libbuzzy/package.c
    libbuzzy/version.c
    libbuzzy/distro/arch.c
    libbuzzy/distro/packager.c
    libbuzzy/distro/pdb.c
)

add_library(libbuzzy STATIC ${LIBBUZZY_SRC})
target_link_libraries(libbuzzy ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(libbuzzy PROPERTIES
    OUTPUT_NAME buzzy
)

#-----------------------------------------------------------------------
# buzzy

set(BUZZY_SRC
    buzzy/buzzy.c
    buzzy/raw.c
    buzzy/raw-pkg.c
    buzzy/root.c
    buzzy/vercmp.c
)

add_executable(buzzy ${BUZZY_SRC})
set_target_properties(buzzy PROPERTIES
    COMPILE_DEFINITIONS CORK_API=CORK_LOCAL
)
target_link_libraries(buzzy libbuzzy libcork libyaml)
install(TARGETS buzzy DESTINATION bin)
