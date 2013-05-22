/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_VALUE_H
#define BUZZY_VALUE_H

#include <libcork/core.h>
#include <yaml.h>


struct bz_value;


/*-----------------------------------------------------------------------
 * Scalar values
 */

typedef const char *
(*bz_scalar_value_get_f)(void *user_data, struct bz_value *ctx);

struct bz_value *
bz_scalar_value_new(void *user_data, cork_free_f free_user_data,
                    bz_scalar_value_get_f get);

const char *
bz_scalar_value_get(struct bz_value *value, struct bz_value *ctx);


/*-----------------------------------------------------------------------
 * Array values
 */

typedef size_t
(*bz_array_value_count_f)(void *user_data);

typedef struct bz_value *
(*bz_array_value_get_f)(void *user_data, size_t index);

struct bz_value *
bz_array_value_new(void *user_data, cork_free_f free_user_data,
                   bz_array_value_count_f count,
                   bz_array_value_get_f get);

size_t
bz_array_value_count(struct bz_value *value);

struct bz_value *
bz_array_value_get(struct bz_value *value, size_t index);


/*-----------------------------------------------------------------------
 * Map values
 */

typedef struct bz_value *
(*bz_map_value_get_f)(void *user_data, const char *key);

typedef int
(*bz_map_value_add_f)(void *user_data, const char *key,
                      struct bz_value *element, bool overwrite);

struct bz_value *
bz_map_value_new(void *user_data, cork_free_f free_user_data,
                 bz_map_value_get_f get,
                 bz_map_value_add_f add);

struct bz_value *
bz_map_value_get(struct bz_value *value, const char *key);

/* Takes control of element */
int
bz_map_value_add(struct bz_value *value, const char *key,
                 struct bz_value *element, bool overwrite);


/*-----------------------------------------------------------------------
 * Values
 */

enum bz_value_kind {
    BZ_VALUE_SCALAR,
    BZ_VALUE_ARRAY,
    BZ_VALUE_MAP
};

void
bz_value_free(struct bz_value *value);

enum bz_value_kind
bz_value_kind(struct bz_value *value);

const char *
bz_value_kind_string(enum bz_value_kind kind);


struct bz_value *
bz_value_get_nested(struct bz_value *value, const char *key);

int
bz_value_set_nested(struct bz_value *value, const char *key,
                    struct bz_value *element, bool overwrite);


/* All of the following return an error if the value is missing, or if it's
 * malformed.  You are not responsible for freeing any results. */

bool
bz_value_get_bool(struct bz_value *value, const char *name);

long
bz_value_get_long(struct bz_value *value, const char *name);

struct cork_path *
bz_value_get_path(struct bz_value *value, const char *name);

const char *
bz_value_get_string(struct bz_value *value, const char *name);

struct bz_version *
bz_value_get_version(struct bz_value *value, const char *name);


/* Any relative paths in this value will be interpreted relative to this base
 * path.  Base path defaults to the current working directory. */
const char *
bz_value_base_path(struct bz_value *value);

void
bz_value_set_base_path(struct bz_value *value, const char *base_path);


/* The result is only valid as long as other is still alive. */
struct bz_value *
bz_value_copy(struct bz_value *other);


/*-----------------------------------------------------------------------
 * Built-in value types
 */

/* Takes control of path */
struct bz_value *
bz_path_value_new(struct cork_path *path);

struct bz_value *
bz_string_value_new(const char *value);

/* ${var} is substituted with the value of another variable */
struct bz_value *
bz_interpolated_value_new(const char *template_value);


/*-----------------------------------------------------------------------
 * Editable arrays
 */

struct bz_array *
bz_array_new(void);

/* Takes control of value */
void
bz_array_append(struct bz_array *array, struct bz_value *value);

struct bz_value *
bz_array_as_value(struct bz_array *array);


/*-----------------------------------------------------------------------
 * Editable maps
 */

struct bz_value *
bz_map_new(void);


/*-----------------------------------------------------------------------
 * Union of maps
 */

struct bz_union_map *
bz_union_map_new(void);

/* Takes control of element */
void
bz_union_map_add(struct bz_union_map *map, struct bz_value *element);

struct bz_value *
bz_union_map_as_value(struct bz_union_map *map);


/*-----------------------------------------------------------------------
 * YAML file of values
 */

/* Takes control of doc */
struct bz_value *
bz_yaml_value_new(yaml_document_t *doc);

struct bz_value *
bz_yaml_value_new_from_file(const char *path);

struct bz_value *
bz_yaml_value_new_from_string(const char *content);


#endif /* BUZZY_VALUE_H */
