/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/helpers/errors.h>
#include <yaml.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/yaml.h"

#define CLOG_CHANNEL  "yaml"


/*-----------------------------------------------------------------------
 * Parsers
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


/*-----------------------------------------------------------------------
 * Getters
 */

int
bz_yaml_get_scalar(yaml_document_t *doc, int node_id,
                   const char **content, size_t *size,
                   const char *context_name)
{
    yaml_node_t  *node = yaml_document_get_node(doc, node_id);
    if (CORK_LIKELY(node->type == YAML_SCALAR_NODE)) {
        *content = (const char *) node->data.scalar.value;
        *size = node->data.scalar.length;
        return 0;
    } else {
        bz_bad_config("Expected a scalar value for %s", context_name);
        return -1;
    }
}

const char *
bz_yaml_get_string(yaml_document_t *doc, int node_id, const char *context_name)
{
    const char  *content = NULL;
    size_t  size;
    rpi_check(bz_yaml_get_scalar(doc, node_id, &content, &size, context_name));
    return content;
}


static int
bz_yaml_process_mapping(yaml_document_t *doc, yaml_node_t *node,
                        struct bz_yaml_mapping_element *elements,
                        bool strict, const char *context_name)
{
    yaml_node_pair_t  *pair;

    /* If the node is a mapping, loop through all of the elements, pulling out
     * the ones that were requested. */
    for (pair = node->data.mapping.pairs.start;
         pair < node->data.mapping.pairs.top; pair++) {
        yaml_node_t  *key_node = yaml_document_get_node(doc, pair->key);
        const char  *key;
        bool  requested = false;
        size_t  i;

        if (CORK_LIKELY(key_node->type == YAML_SCALAR_NODE)) {
            key = (char *) key_node->data.scalar.value;
        } else {
            bz_bad_config("Mapping key must be a string in %s", context_name);
            return -1;
        }

        for (i = 0; elements[i].field_name != NULL; i++) {
            if (strcmp(elements[i].field_name, key) == 0) {
                /* The caller requested this field. */
                elements[i].value_id = pair->value;
                requested = true;
                break;
            }
        }

        if (CORK_UNLIKELY(strict && !requested)) {
            bz_bad_config("Unknown field \"%s\" in %s", key, context_name);
            return -1;
        }
    }

    return 0;
}

int
bz_yaml_get_mapping_elements(yaml_document_t *doc, int node_id,
                             struct bz_yaml_mapping_element *elements,
                             bool strict, const char *context_name)
{
    yaml_node_t  *node = yaml_document_get_node(doc, node_id);
    size_t  i;

    /* Start by assuming that each requested field is not present in the
     * mapping. */
    for (i = 0; elements[i].field_name != NULL; i++) {
        elements[i].value_id = -1;
    }

    /* If the node is null, treat that as an empty mapping. */
    if (node->type == YAML_MAPPING_NODE) {
        rii_check(bz_yaml_process_mapping
                  (doc, node, elements, strict, context_name));
    } else if (node->type == YAML_SCALAR_NODE) {
        /* A missing mapping is parsed as an empty scalar string.  Let's treat
         * that as an empty mapping. */
        if (node->data.scalar.length != 0) {
            bz_bad_config("Expected a mapping value for %s", context_name);
            return -1;
        }
    } else {
        bz_bad_config("Expected a mapping value for %s", context_name);
        return -1;
    }

    /* Verify that each required field was present in the mapping. */
    for (i = 0; elements[i].field_name != NULL; i++) {
        if (CORK_UNLIKELY(elements[i].required && elements[i].value_id == -1)) {
            bz_bad_config("Missing field \"%s\" in %s",
                          elements[i].field_name, context_name);
            return -1;
        }
    }

    return 0;
}


/*-----------------------------------------------------------------------
 * YAML values
 */

static struct bz_value *
bz_yaml_value_new_from_node(yaml_document_t *doc, yaml_node_t *node);


static struct bz_value *
bz_yaml_string_new(yaml_document_t *doc, yaml_node_t *node)
{
    const char  *content = (const char *) node->data.scalar.value;
    return bz_interpolated_value_new(content);
}


static struct bz_value *
bz_yaml_array_new(yaml_document_t *doc, yaml_node_t *node)
{
    struct bz_array  *array;
    yaml_node_item_t  *item;

    array = bz_array_new();
    for (item = node->data.sequence.items.start;
         item < node->data.sequence.items.top; item++) {
        yaml_node_t  *element_node = yaml_document_get_node(doc, *item);
        struct bz_value  *element;
        ep_check(element = bz_yaml_value_new_from_node(doc, element_node));
        bz_array_append(array, element);
    }
    return bz_array_as_value(array);

error:
    bz_value_free(bz_array_as_value(array));
    return NULL;
}


static struct bz_value *
bz_yaml_map_new(yaml_document_t *doc, yaml_node_t *node)
{
    struct bz_value  *map;
    yaml_node_pair_t  *pair;

    map = bz_map_new();
    for (pair = node->data.mapping.pairs.start;
         pair < node->data.mapping.pairs.top; pair++) {
        yaml_node_t  *key_node = yaml_document_get_node(doc, pair->key);
        yaml_node_t  *value_node = yaml_document_get_node(doc, pair->value);
        const char  *key;
        struct bz_value  *value;

        if (CORK_LIKELY(key_node->type == YAML_SCALAR_NODE)) {
            key = (char *) key_node->data.scalar.value;
        } else {
            bz_bad_config("Mapping key must be a string");
            return NULL;
        }

        ep_check(value = bz_yaml_value_new_from_node(doc, value_node));
        ei_check(bz_map_value_add(map, key, value, false));
    }
    return map;

error:
    bz_value_free(map);
    return NULL;
}



static struct bz_value *
bz_yaml_value_new_from_node(yaml_document_t *doc, yaml_node_t *node)
{
    if (node == NULL) {
        /* A missing node is treated just like an empty map. */
        return bz_map_new();
    } else if (node->type == YAML_SCALAR_NODE) {
        return bz_yaml_string_new(doc, node);
    } else if (node->type == YAML_SEQUENCE_NODE) {
        return bz_yaml_array_new(doc, node);
    } else if (node->type == YAML_MAPPING_NODE) {
        return bz_yaml_map_new(doc, node);
    } else {
        cork_unreachable();
    }
}

struct bz_value *
bz_yaml_value_new(yaml_document_t *doc)
{
    yaml_node_t  *root = yaml_document_get_root_node(doc);
    struct bz_value  *result = bz_yaml_value_new_from_node(doc, root);
    yaml_document_delete(doc);
    return result;
}

struct bz_value *
bz_yaml_value_new_from_file(const char *path)
{
    yaml_document_t  doc;
    clog_debug("Load YAML file %s", path);
    rpi_check(bz_load_yaml_file(&doc, path));
    return bz_yaml_value_new(&doc);
}

struct bz_value *
bz_yaml_value_new_from_string(const char *content)
{
    yaml_document_t  doc;
    rpi_check(bz_load_yaml_string(&doc, content));
    return bz_yaml_value_new(&doc);
}
