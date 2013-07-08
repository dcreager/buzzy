/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_YAML_H
#define BUZZY_YAML_H

#include <libcork/core.h>
#include <yaml.h>

#include "buzzy/env.h"


/*-----------------------------------------------------------------------
 * Parsers
 */

int
bz_load_yaml_file(yaml_document_t *doc, const char *path);

int
bz_load_yaml_string(yaml_document_t *doc, const char *content);


/*-----------------------------------------------------------------------
 * Getters
 */

const char *
bz_yaml_get_string(yaml_document_t *doc, int node_id,
                   const char *context_name);

int
bz_yaml_get_scalar(yaml_document_t *doc, int node_id,
                   const char **content, size_t *size,
                   const char *context_name);

struct bz_yaml_mapping_element {
    const char  *field_name;
    int  value_id;
    bool  required;
};

int
bz_yaml_get_mapping_elements(yaml_document_t *doc, int node_id,
                             struct bz_yaml_mapping_element *elements,
                             bool strict, const char *context_name);


#endif /* BUZZY_YAML_H */
