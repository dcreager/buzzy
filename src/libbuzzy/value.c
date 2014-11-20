/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <ctype.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/value.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Values
 */

struct bz_value {
    enum bz_value_kind  kind;
    const char  *base_path;

    void  *user_data;
    cork_free_f  free_user_data;
    union {
        struct {
            bz_scalar_value_get_f  get;
        } scalar;
        struct {
            bz_array_value_count_f  count;
            bz_array_value_get_f  get;
        } array;
        struct {
            bz_map_value_get_f  get;
            bz_map_value_add_f  add;
        } map;
    } _;

    /* Some "clever" caching of parsing the scalar content as a particular Buzzy
     * type. */
    struct cork_path  *path;
    struct bz_version  *version;
};

void
bz_value_free(struct bz_value *value)
{
    if (value->path != NULL) {
        cork_path_free(value->path);
    }
    if (value->version != NULL) {
        bz_version_free(value->version);
    }
    cork_strfree(value->base_path);
    cork_free_user_data(value);
    free(value);
}

enum bz_value_kind
bz_value_kind(struct bz_value *value)
{
    return value->kind;
}

const char *
bz_value_kind_string(enum bz_value_kind kind)
{
    switch (kind) {
        case BZ_VALUE_SCALAR:
            return "scalar";
        case BZ_VALUE_ARRAY:
            return "array";
        case BZ_VALUE_MAP:
            return "map";
        default:
            cork_unreachable();
    }
}


struct bz_value *
bz_value_get_nested(struct bz_value *value, const char *key)
{
    struct bz_value  *curr = value;
    struct cork_buffer  buf;
    const char  *name = key;
    const char  *next;

    if (key == NULL) {
        return value;
    }

    /* Find children for each name up through the last dot. */
    cork_buffer_init(&buf);
    while ((next = strchr(name, '.')) != NULL) {
        ptrdiff_t  length = next - name;
        cork_buffer_set(&buf, name, length);
        ep_check(curr = bz_map_value_get(curr, buf.buf));
        name = next + 1;
    }

    /* Then find the child after the last dot. */
    cork_buffer_done(&buf);
    return bz_map_value_get(curr, name);

error:
    cork_buffer_done(&buf);
    return NULL;
}


int
bz_value_set_nested(struct bz_value *value, const char *key,
                    struct bz_value *element, bool overwrite)
{
    struct bz_value  *curr = value;
    struct cork_buffer  buf;
    const char  *name = key;
    const char  *next;

    if (key == NULL) {
        bz_bad_config("Must provide a key");
        bz_value_free(element);
        return -1;
    }

    /* Find children for each name up through the last dot. */
    cork_buffer_init(&buf);
    while ((next = strchr(name, '.')) != NULL) {
        struct bz_value  *next_value;
        ptrdiff_t  length = next - name;
        cork_buffer_set(&buf, name, length);
        ee_check(next_value = bz_map_value_get(curr, buf.buf));
        if (next_value == NULL) {
            /* If we encounter a missing map, add an empty one and keep going */
            next_value = bz_map_new();
            bz_map_value_add(curr, buf.buf, next_value, false);
        }
        name = next + 1;
        curr = next_value;
    }

    /* Then add to the final element */
    cork_buffer_done(&buf);
    return bz_map_value_add(curr, name, element, overwrite);

error:
    cork_buffer_done(&buf);
    bz_value_free(element);
    return -1;
}


const char *
bz_value_base_path(struct bz_value *value)
{
    return value->base_path;
}

void
bz_value_set_base_path(struct bz_value *value, const char *base_path)
{
    cork_strfree(value->base_path);
    value->base_path = cork_strdup(base_path);
}


/*-----------------------------------------------------------------------
 * Retrieving the value of a variable
 */

static struct { const char *s; bool b; }  bool_values[] = {
    { "1", true },
    { "true", true },
    { "yes", true },

    { "0", false },
    { "false", false },
    { "no", false },

    { NULL }
};

static int
bz_verify_exists(struct bz_value *value, const char *name, bool required)
{
    if (value == NULL) {
        if (required) {
            bz_bad_config("No value for %s", name);
        }
        return -1;
    } else {
        return 0;
    }
}

bool
bz_value_get_bool(struct bz_value *root, const char *name, bool required)
{
    struct bz_value  *value;
    const char  *content;
    size_t  i;
    xe_check(false, value = bz_value_get_nested(root, name));
    xi_check(false, bz_verify_exists(value, name, required));
    xp_check(false, content = bz_scalar_value_get(value, root));
    for (i = 0; bool_values[i].s != NULL; i++) {
        if (strcasecmp(content, bool_values[i].s) == 0) {
            return bool_values[i].b;
        }
    }
    bz_bad_config
        ("Invalid boolean \"%s\" for %s",
         content, (name == NULL)? "value": name);
    return false;
}

long
bz_value_get_long(struct bz_value *root, const char *name, bool required)
{
    struct bz_value  *value;
    const char  *content;
    long  result;
    char  *endptr = NULL;
    xe_check(0, value = bz_value_get_nested(root, name));
    xi_check(0, bz_verify_exists(value, name, required));
    xp_check(0, content = bz_scalar_value_get(value, root));
    result = strtol(content, &endptr, 0);
    if (!isdigit(*content) || *endptr != '\0') {
        bz_bad_config
            ("Invalid integer \"%s\" for %s",
             content, (name == NULL)? "value": name);
        return 0;
    } else {
        return result;
    }
}

struct cork_path *
bz_value_get_path(struct bz_value *root, const char *name, bool required)
{
    struct bz_value  *value;
    const char  *content;
    rpe_check(value = bz_value_get_nested(root, name));
    rpi_check(bz_verify_exists(value, name, required));
    if (value->path != NULL) {
        cork_path_free(value->path);
    }
    rpp_check(content = bz_scalar_value_get(value, root));
    value->path = cork_path_new(root->base_path);
    cork_path_append(value->path, content);
    return value->path;
}

const char *
bz_value_get_string(struct bz_value *root, const char *name, bool required)
{
    struct bz_value  *value;
    rpe_check(value = bz_value_get_nested(root, name));
    rpi_check(bz_verify_exists(value, name, required));
    return bz_scalar_value_get(value, root);
}

struct bz_version *
bz_value_get_version(struct bz_value *root, const char *name, bool required)
{
    struct bz_value  *value;
    const char  *content;
    rpe_check(value = bz_value_get_nested(root, name));
    rpi_check(bz_verify_exists(value, name, required));
    if (value->version != NULL) {
        bz_version_free(value->version);
    }
    rpp_check(content = bz_scalar_value_get(value, root));
    rpp_check(value->version = bz_version_from_string(content));
    return value->version;
}


/*-----------------------------------------------------------------------
 * Scalar values
 */

struct bz_value *
bz_scalar_value_new(void *user_data, cork_free_f free_user_data,
                    bz_scalar_value_get_f get)
{
    struct bz_value  *value = cork_new(struct bz_value);
    value->kind = BZ_VALUE_SCALAR;
    value->user_data = user_data;
    value->free_user_data = free_user_data;
    value->_.scalar.get = get;
    value->base_path = cork_strdup("");
    value->path = NULL;
    value->version = NULL;
    return value;
}

const char *
bz_scalar_value_get(struct bz_value *value, struct bz_value *ctx)
{
    if (CORK_LIKELY(value->kind == BZ_VALUE_SCALAR)) {
        return value->_.scalar.get(value->user_data, ctx);
    } else {
        bz_bad_config("Value must be a scalar");
        return NULL;
    }
}


/*-----------------------------------------------------------------------
 * Array values
 */

struct bz_value *
bz_array_value_new(void *user_data, cork_free_f free_user_data,
                   bz_array_value_count_f count,
                   bz_array_value_get_f get)
{
    struct bz_value  *value = cork_new(struct bz_value);
    value->kind = BZ_VALUE_ARRAY;
    value->user_data = user_data;
    value->free_user_data = free_user_data;
    value->_.array.count = count;
    value->_.array.get = get;
    value->base_path = cork_strdup("");
    value->path = NULL;
    value->version = NULL;
    return value;
}

size_t
bz_array_value_count(struct bz_value *value)
{
    if (CORK_LIKELY(value->kind == BZ_VALUE_ARRAY)) {
        return value->_.array.count(value->user_data);
    } else {
        bz_bad_config("Value must be an array");
        return 0;
    }
}

struct bz_value *
bz_array_value_get(struct bz_value *value, size_t index)
{
    if (CORK_LIKELY(value->kind == BZ_VALUE_ARRAY)) {
        return value->_.array.get(value->user_data, index);
    } else {
        bz_bad_config
            ("Can't get element %zu from a %s",
             index, bz_value_kind_string(value->kind));
        return NULL;
    }
}


int
bz_array_value_map(struct bz_value *value, void *user_data, bz_array_map_f map)
{
    size_t  i;
    size_t  count;
    rie_check(count = bz_array_value_count(value));
    for (i = 0; i < count; i++) {
        struct bz_value  *element;
        rie_check(element = bz_array_value_get(value, i));
        rii_check(map(user_data, element));
    }
    return 0;
}

int
bz_array_value_map_scalars(struct bz_value *value, void *user_data,
                           bz_array_map_f map)
{
    if (value->kind == BZ_VALUE_SCALAR) {
        return map(user_data, value);
    } else if (value->kind == BZ_VALUE_ARRAY) {
        size_t  i;
        size_t  count;
        rie_check(count = bz_array_value_count(value));
        for (i = 0; i < count; i++) {
            struct bz_value  *element;
            rie_check(element = bz_array_value_get(value, i));
            if (element->kind != BZ_VALUE_SCALAR) {
                bz_bad_config("Array must only contain scalars");
                return -1;
            }
            rii_check(map(user_data, element));
        }
        return 0;
    } else {
        bz_bad_config("Value must be an array or scalar");
        return -1;
    }
}


/*-----------------------------------------------------------------------
 * Map values
 */

static int
bz_map__default_add(void *user_data, const char *key, struct bz_value *element,
                    bool overwrite)
{
    bz_value_free(element);
    bz_bad_config("Cannot add %s to this map", key);
    return -1;
}

struct bz_value *
bz_map_value_new(void *user_data, cork_free_f free_user_data,
                 bz_map_value_get_f get,
                 bz_map_value_add_f add)
{
    struct bz_value  *value = cork_new(struct bz_value);
    value->kind = BZ_VALUE_MAP;
    value->user_data = user_data;
    value->free_user_data = free_user_data;
    value->_.map.get = get;
    value->_.map.add = (add == NULL)? bz_map__default_add: add;
    value->base_path = cork_strdup("");
    value->path = NULL;
    value->version = NULL;
    return value;
}

struct bz_value *
bz_map_value_get(struct bz_value *value, const char *key)
{
    if (CORK_LIKELY(value->kind == BZ_VALUE_MAP)) {
        return value->_.map.get(value->user_data, key);
    } else {
        bz_bad_config
            ("Can't get %s from a %s", key, bz_value_kind_string(value->kind));
        return NULL;
    }
}

int
bz_map_value_add(struct bz_value *value, const char *key,
                 struct bz_value *element, bool overwrite)
{
    if (CORK_LIKELY(value->kind == BZ_VALUE_MAP)) {
        return value->_.map.add(value->user_data, key, element, overwrite);
    } else {
        bz_value_free(element);
        bz_bad_config
            ("Can't get %s from a %s", key, bz_value_kind_string(value->kind));
        return -1;
    }
}


/*-----------------------------------------------------------------------
 * Copied values
 */

struct bz_value *
bz_value_copy(struct bz_value *other)
{
    struct bz_value  *value = cork_new(struct bz_value);
    value->kind = other->kind;
    value->base_path = cork_strdup(other->base_path);
    value->user_data = other->user_data;
    value->free_user_data = NULL;
    value->_ = other->_;
    value->path = NULL;
    value->version = NULL;
    return value;
}


/*-----------------------------------------------------------------------
 * Path values
 */

static const char *
bz_path_value__get(void *user_data, struct bz_value *ctx)
{
    struct cork_path  *path = user_data;
    return cork_path_get(path);
}

static void
bz_path_value__free(void *user_data)
{
    struct cork_path  *path = user_data;
    cork_path_free(path);
}

struct bz_value *
bz_path_value_new(struct cork_path *path)
{
    return bz_scalar_value_new(path, bz_path_value__free, bz_path_value__get);
}


/*-----------------------------------------------------------------------
 * String values
 */

static const char *
bz_string_value__get(void *user_data, struct bz_value *ctx)
{
    const char  *value = user_data;
    return value;
}

static void
bz_string_value__free(void *user_data)
{
    const char  *value = user_data;
    cork_strfree(value);
}

struct bz_value *
bz_string_value_new(const char *value)
{
    const char  *copy = cork_strdup(value);
    return bz_scalar_value_new
        ((void *) copy, bz_string_value__free, bz_string_value__get);
}


/*-----------------------------------------------------------------------
 * Editable arrays
 */

struct bz_array {
    cork_array(struct bz_value *)  elements;
    struct bz_value  *value;
};

static void
bz_array__free(void *user_data)
{
    struct bz_array  *array = user_data;
    cork_array_done(&array->elements);
    free(array);
}

static size_t
bz_array__count(void *user_data)
{
    struct bz_array  *array = user_data;
    return cork_array_size(&array->elements);
}

static struct bz_value *
bz_array__get(void *user_data, size_t index)
{
    struct bz_array  *array = user_data;
    return cork_array_at(&array->elements, index);
}

struct bz_array *
bz_array_new(void)
{
    struct bz_array  *array = cork_new(struct bz_array);
    cork_pointer_array_init(&array->elements, (cork_free_f) bz_value_free);
    array->value = bz_array_value_new
        (array, bz_array__free, bz_array__count, bz_array__get);
    return array;
}

void
bz_array_append(struct bz_array *array, struct bz_value *value)
{
    cork_array_append(&array->elements, value);
}

struct bz_value *
bz_array_as_value(struct bz_array *array)
{
    return array->value;
}


/*-----------------------------------------------------------------------
 * Editable maps
 */

struct bz_map {
    struct cork_hash_table  *table;
    struct bz_value  *value;
};

static void
bz_map__free(void *user_data)
{
    struct bz_map  *map = user_data;
    cork_hash_table_free(map->table);
    free(map);
}

static struct bz_value *
bz_map__get(void *user_data, const char *key)
{
    struct bz_map  *map = user_data;
    return cork_hash_table_get(map->table, (void *) key);
}

static int
bz_map__add(void *user_data, const char *key, struct bz_value *value,
            bool overwrite)
{
    struct bz_map  *map = user_data;
    bool  is_new;
    struct cork_hash_table_entry  *entry;

    entry = cork_hash_table_get_or_create(map->table, (void *) key, &is_new);
    if (is_new) {
        entry->key = (void *) cork_strdup(key);
        entry->value = value;
    } else {
        if (overwrite) {
            struct bz_value  *old_value = entry->value;
            bz_value_free(old_value);
            entry->value = value;
        } else {
            bz_value_free(value);
            bz_bad_config("Map already contains %s", key);
            return -1;
        }
    }
    return 0;
}

struct bz_value *
bz_map_new(void)
{
    struct bz_map  *map = cork_new(struct bz_map);
    map->table = cork_string_hash_table_new(0, 0);
    cork_hash_table_set_free_key(map->table, (cork_free_f) cork_strfree);
    cork_hash_table_set_free_value(map->table, (cork_free_f) bz_value_free);
    map->value = bz_map_value_new
        (map, bz_map__free, bz_map__get, bz_map__add);
    return map->value;
}


/*-----------------------------------------------------------------------
 * Union of maps
 */

struct bz_union_map {
    struct cork_hash_table  *cache;
    cork_array(struct bz_value *)  maps;
    cork_array(struct bz_value *)  child_maps;
    struct bz_value  *value;
};

static void
bz_union_map__free(void *user_data)
{
    struct bz_union_map  *map = user_data;
    cork_hash_table_free(map->cache);
    cork_array_done(&map->maps);
    cork_array_done(&map->child_maps);
    free(map);
}

static struct bz_value *
bz_union_map_find_key(struct bz_union_map *map, const char *key,
                      struct cork_hash_table_entry *entry)
{
    struct bz_value  *value = NULL;
    struct bz_union_map  *child_union_map;
    struct bz_value  *child_union_value;
    size_t  i;

    entry->key = (void *) cork_strdup(key);

    /* If any of the maps in the union have a (child) map as the value for key,
     * then we need to create a new union of all of those child maps, so that
     * their keys are merged together. */

    /* Find the first map that contains key. */
    for (i = 0; i < cork_array_size(&map->maps); i++) {
        struct bz_value  *element = cork_array_at(&map->maps, i);
        rpe_check(value = bz_map_value_get(element, key));
        if (value != NULL) {
            /* If the value isn't a map, then we have the final result. */
            if (value->kind == BZ_VALUE_SCALAR ||
                value->kind == BZ_VALUE_ARRAY) {
                entry->value = value;
                return value;
            }

            /* Otherwise, fall through to create the child map. */
            break;
        }
    }

    /* If we never found any values for this key, then that's also a final
     * result. */
    if (value == NULL) {
        return NULL;
    }

    /* Otherwise we've found a child map.  Create a child union map containing
     * it, and any other values that we find for this key. */
    child_union_map = bz_union_map_new();
    bz_union_map_add(child_union_map, bz_value_copy(value));
    child_union_value = bz_union_map_as_value(child_union_map);
    cork_array_append(&map->child_maps, child_union_value);

    for (i = i+1; i < cork_array_size(&map->maps); i++) {
        struct bz_value  *element = cork_array_at(&map->maps, i);
        rpe_check(value = bz_map_value_get(element, key));
        if (value != NULL) {
            if (value->kind == BZ_VALUE_MAP) {
                bz_union_map_add(child_union_map, bz_value_copy(value));
            } else {
                /* If this value isn't a map, then we have an inconsistency. */
                bz_bad_config
                    ("%s is both a %s and a map",
                     key, bz_value_kind_string(value->kind));
                return NULL;
            }
        }
    }

    entry->value = child_union_value;
    return child_union_value;
}

static struct bz_value *
bz_union_map__get(void *user_data, const char *key)
{
    struct bz_union_map  *map = user_data;
    struct cork_hash_table_entry  *entry;
    bool  is_new;

    entry = cork_hash_table_get_or_create(map->cache, (void *) key, &is_new);
    if (is_new) {
        /* We haven't tried to retrieve this key yet.  Look in through each of
         * the maps to see which ones define the key. */
        return bz_union_map_find_key(map, key, entry);
    } else {
        /* We've already seen this key, so just returned the cached result. */
        return entry->value;
    }
}

struct bz_union_map *
bz_union_map_new(void)
{
    struct bz_union_map  *map = cork_new(struct bz_union_map);
    map->cache = cork_string_hash_table_new(0, 0);
    cork_hash_table_set_free_key(map->cache, (cork_free_f) cork_strfree);
    cork_pointer_array_init(&map->maps, (cork_free_f) bz_value_free);
    cork_pointer_array_init(&map->child_maps, (cork_free_f) bz_value_free);
    map->value = bz_map_value_new
        (map, bz_union_map__free, bz_union_map__get, NULL);
    return map;
}

void
bz_union_map_add(struct bz_union_map *map, struct bz_value *element)
{
    assert(element->kind == BZ_VALUE_MAP);
    cork_array_append(&map->maps, element);
}

struct bz_value *
bz_union_map_as_value(struct bz_union_map *map)
{
    return map->value;
}
