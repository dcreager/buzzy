# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------
# Copyright © 2013, RedJack, LLC.
# All rights reserved.
#
# Please see the LICENSE.txt file in this distribution for license
# details.
# ----------------------------------------------------------------------

include_directories(${CMAKE_SOURCE_DIR}/lib/libyaml/include)

set(LIBYAML_SRC
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/api.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/dumper.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/emitter.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/loader.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/parser.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/reader.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/scanner.c
    ${CMAKE_SOURCE_DIR}/lib/libyaml/src/writer.c
)

list(APPEND LIBYAML_DEFINITIONS
    YAML_VERSION_MAJOR=0
    YAML_VERSION_MINOR=1
    YAML_VERSION_PATCH=4
    YAML_VERSION_STRING=\"0.1.4\"
)

if(CMAKE_COMPILER_IS_GNUCC)
    remove_definitions(-Wall -Werror)
endif(CMAKE_COMPILER_IS_GNUCC)

add_library(libyaml STATIC ${LIBYAML_SRC})
set_target_properties(libyaml PROPERTIES
    OUTPUT_NAME yaml
    COMPILE_DEFINITIONS "${LIBYAML_DEFINITIONS}"
)
