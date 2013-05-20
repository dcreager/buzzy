/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/helpers/errors.h>
#include <yaml.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/yaml.h"


/*-----------------------------------------------------------------------
 * YAML helpers
 */

int
bz_load_yaml_file(yaml_document_t *doc, const char *path)
{
    FILE  *file;
    yaml_parser_t  parser;

    file = fopen(path, "r");
    if (CORK_UNLIKELY(file == NULL)) {
        cork_system_error_set();
        return -1;
    }

    if (CORK_UNLIKELY(yaml_parser_initialize(&parser) == 0)) {
        fclose(file);
        bz_bad_config("Error reading %s", path);
        return -1;
    }

    yaml_parser_set_input_file(&parser, file);
    if (CORK_UNLIKELY(yaml_parser_load(&parser, doc) == 0)) {
        bz_bad_config("Error reading %s: %s", path, parser.problem);
        yaml_parser_delete(&parser);
        fclose(file);
        return -1;
    }

    yaml_parser_delete(&parser);
    fclose(file);
    return 0;
}

int
bz_load_yaml_string(yaml_document_t *doc, const char *content)
{
    yaml_parser_t  parser;

    if (CORK_UNLIKELY(yaml_parser_initialize(&parser) == 0)) {
        bz_bad_config("Error reading YAML");
        return -1;
    }

    yaml_parser_set_input_string
        (&parser, (const unsigned char *) content, strlen(content));
    if (CORK_UNLIKELY(yaml_parser_load(&parser, doc) == 0)) {
        bz_bad_config("Error reading YAML: %s", parser.problem);
        yaml_parser_delete(&parser);
        return -1;
    }

    yaml_parser_delete(&parser);
    return 0;
}
