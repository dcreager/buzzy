# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2011-2015, RedJack, LLC.
# All rights reserved.
#
# Please see the COPYING file in this distribution for license details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/clogger/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libcork/include)
include_directories(${CMAKE_BINARY_DIR}/lib/libcork/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/libyaml/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

#-----------------------------------------------------------------------
# Check for prerequisite libraries

find_package(PkgConfig)

pkg_check_modules(CHECK check)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

#-----------------------------------------------------------------------
# Build the test cases

macro(make_test test_name)
    if(CHECK_FOUND)
        add_executable(${test_name} ${test_name}.c)
        target_link_libraries(${test_name}
            ${CHECK_LIBRARIES}
            libbuzzy
            libcork
            libyaml
        )
        add_test(${test_name} ${test_name})
    endif(CHECK_FOUND)
endmacro(make_test)

make_test(test-arch)
make_test(test-autotools)
make_test(test-cmake)
make_test(test-debian)
make_test(test-env)
make_test(test-git)
make_test(test-homebrew)
make_test(test-os)
make_test(test-repo)
make_test(test-rpm)
make_test(test-versions)

#-----------------------------------------------------------------------
# Command-line tests

find_package(PythonInterp)

if (PYTHON_EXECUTABLE)
    configure_file(ccram ${CMAKE_BINARY_DIR}/ccram COPYONLY)
    file(GLOB_RECURSE TESTS "${CMAKE_CURRENT_SOURCE_DIR}/*.t")
    foreach(TEST ${TESTS})
        get_filename_component(TEST_NAME "${TEST}" NAME_WE)
        add_test(
            ${TEST_NAME}
            ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}
            ${CMAKE_BINARY_DIR}/ccram
                --python ${PYTHON_EXECUTABLE}
                --root ${CMAKE_SOURCE_DIR}
                --tests ${TEST}
        )
    endforeach(TEST)
else (PYTHON_EXECUTABLE)
    message(WARNING "Unable to find Python; skipping cram tests.")
endif (PYTHON_EXECUTABLE)
