# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2011-2013, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libcork/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/clogger/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libyaml/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

#-----------------------------------------------------------------------
# Check for prerequisite libraries

find_package(PkgConfig)

pkg_check_modules(CHECK REQUIRED check)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

#-----------------------------------------------------------------------
# Build the test cases

macro(make_test test_name)
    add_executable(${test_name} ${test_name}.c)
    target_link_libraries(${test_name}
        ${CHECK_LIBRARIES}
        libbuzzy
        libcork
        libyaml
    )
    add_test(${test_name} ${test_name})
endmacro(make_test)

make_test(test-arch)
make_test(test-autotools)
make_test(test-cmake)
make_test(test-env)
make_test(test-git)
make_test(test-homebrew)
make_test(test-os)
make_test(test-repo)
make_test(test-rpm)
make_test(test-versions)

#-----------------------------------------------------------------------
# Command-line tests

find_program(
    CRAM_EXECUTABLE
    NAMES cram
    HINTS ENV CRAM_DIR
    PATH_SUFFIXES bin
    DOC "Cram test harness"
)

if (CRAM_EXECUTABLE)
    configure_file(ccram ${CMAKE_BINARY_DIR}/ccram COPYONLY)
    add_test(
        cram-tests
        ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/ccram
            --cram ${CRAM_EXECUTABLE}
            --tests ${CMAKE_CURRENT_SOURCE_DIR}
    )
else (CRAM_EXECUTABLE)
    message(WARNING "Unable to find cram test harness; skipping tests.")
endif (CRAM_EXECUTABLE)
