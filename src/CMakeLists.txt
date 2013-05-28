# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2011-2013, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/clogger/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libcork/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libyaml/include)

add_subdirectory(clogger)
add_subdirectory(libcork)
add_subdirectory(libyaml)

#-----------------------------------------------------------------------
# libbuzzy

set(LIBBUZZY_RAGEL
    libbuzzy/dependency.c.rl
    libbuzzy/interpolated.c.rl
    libbuzzy/version.c.rl
    libbuzzy/distro/arch.c.rl
    libbuzzy/distro/git.c.rl
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
    libbuzzy/builder.c
    libbuzzy/built.c
    libbuzzy/dependency.c
    libbuzzy/env.c
    libbuzzy/global-vars.c
    libbuzzy/interpolated.c
    libbuzzy/logging.c
    libbuzzy/mock.c
    libbuzzy/native.c
    libbuzzy/os.c
    libbuzzy/package.c
    libbuzzy/packager.c
    libbuzzy/repo.c
    libbuzzy/value.c
    libbuzzy/version.c
    libbuzzy/yaml.c
    libbuzzy/builders/cmake.c
    libbuzzy/builders/noop.c
    libbuzzy/distro/arch.c
    libbuzzy/distro/git.c
    libbuzzy/distro/packager.c
    libbuzzy/distro/pdb.c
    libbuzzy/packagers/noop.c
    libbuzzy/packagers/pacman.c
    libbuzzy/repos/filesystem.c
    libbuzzy/repos/git.c
    libbuzzy/repos/local.c
    libbuzzy/repos/url.c
)

add_library(libbuzzy STATIC ${LIBBUZZY_SRC})
target_link_libraries(libbuzzy
    ${CMAKE_THREAD_LIBS_INIT}
    libcork
    libclogger
    libyaml
)
set_target_properties(libbuzzy PROPERTIES
    OUTPUT_NAME buzzy
)

#-----------------------------------------------------------------------
# buzzy

set(BUZZY_SRC
    buzzy/build.c
    buzzy/buzzy.c
    buzzy/doc.c
    buzzy/info.c
    buzzy/install.c
    buzzy/raw.c
    buzzy/raw-build.c
    buzzy/raw-pkg.c
    buzzy/root.c
    buzzy/test.c
    buzzy/uninstall.c
    buzzy/update.c
    buzzy/vercmp.c
)

add_executable(buzzy ${BUZZY_SRC})
set_target_properties(buzzy PROPERTIES
    COMPILE_DEFINITIONS CORK_API=CORK_LOCAL
)
target_link_libraries(buzzy libbuzzy libcork libclogger)
install(TARGETS buzzy DESTINATION bin)
