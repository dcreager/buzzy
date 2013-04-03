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
#include <string.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/callbacks.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/version.h"


#if !defined(BZ_DEBUG_ENV)
#define BZ_DEBUG_ENV  0
#endif

#if BZ_DEBUG_ENV
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


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

int
bz_env_get_bool(struct bz_env *env, const char *name, bool *dest,
                bool required)
{
    const char  *value = bz_env_get(env, name);
    assert(dest != NULL);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return -1;
    } else if (value == NULL) {
        if (required) {
            bz_bad_config("Missing %s in %s", name, bz_env_name(env));
            return -1;
        } else {
            return 0;
        }
    } else {
        size_t  i;
        for (i = 0; bool_values[i].s != NULL; i++) {
            if (strcasecmp(value, bool_values[i].s) == 0) {
                *dest = bool_values[i].b;
                return 0;
            }
        }
        bz_bad_config
            ("Invalid boolean for %s in %s: %s", name, bz_env_name(env), value);
        return -1;
    }
}

int
bz_env_get_long(struct bz_env *env, const char *name, long *dest,
                bool required)
{
    const char  *value = bz_env_get(env, name);
    assert(dest != NULL);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return -1;
    } else if (value == NULL) {
        if (required) {
            bz_bad_config("Missing %s in %s", name, bz_env_name(env));
            return -1;
        } else {
            return 0;
        }
    } else {
        char  *endptr = NULL;
        *dest = strtol(value, &endptr, 0);
        if (!isdigit(*dest) || *endptr != '\0') {
            bz_bad_config
                ("Invalid integer %s in %s: %s", name, bz_env_name(env), value);
            return -1;
        } else {
            return 0;
        }
    }
}

struct cork_path *
bz_env_get_path(struct bz_env *env, const char *name, bool required)
{
    const char  *value = bz_env_get(env, name);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return NULL;
    } else if (value == NULL) {
        if (required) {
            bz_bad_config("Missing %s in %s", name, bz_env_name(env));
        }
        return NULL;
    } else {
        return cork_path_new(value);
    }
}

const char *
bz_env_get_string(struct bz_env *env, const char *name, bool required)
{
    const char  *value = bz_env_get(env, name);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return NULL;
    } else if (value == NULL) {
        if (required) {
            bz_bad_config("Missing %s in %s", name, bz_env_name(env));
        }
        return NULL;
    } else {
        return value;
    }
}

struct bz_version *
bz_env_get_version(struct bz_env *env, const char *name, bool required)
{
    const char  *value = bz_env_get(env, name);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return NULL;
    } else if (value == NULL) {
        if (required) {
            bz_bad_config("Missing %s in %s", name, bz_env_name(env));
        }
        return NULL;
    } else {
        return bz_version_from_string(value);
    }
}


/*-----------------------------------------------------------------------
 * Value providers
 */

struct bz_value_provider {
    void  *user_data;
    bz_free_f  user_data_free;
    bz_provide_value_f  provide_value;
};

struct bz_value_provider *
bz_value_provider_new(void *user_data, bz_free_f user_data_free,
                      bz_provide_value_f provide_value)
{
    struct bz_value_provider  *provider = cork_new(struct bz_value_provider);
    provider->user_data = user_data;
    provider->user_data_free = user_data_free;
    provider->provide_value = provide_value;
    return provider;
}

void
bz_value_provider_free(struct bz_value_provider *provider)
{
    bz_user_data_free(provider);
    free(provider);
}

const char *
bz_value_provider_get(struct bz_value_provider *provider, struct bz_env *env)
{
    return provider->provide_value(provider->user_data, env);
}


/*-----------------------------------------------------------------------
 * Value sets
 */

struct bz_value_set {
    const char  *name;
    void  *user_data;
    bz_free_f  user_data_free;
    bz_value_set_get_f  get;
};

struct bz_value_set *
bz_value_set_new(const char *name, void *user_data, bz_free_f user_data_free,
                 bz_value_set_get_f get)
{
    struct bz_value_set  *set = cork_new(struct bz_value_set);
    set->name = cork_strdup(name);
    set->user_data = user_data;
    set->user_data_free = user_data_free;
    set->get = get;
    return set;
}

void
bz_value_set_free(struct bz_value_set *set)
{
    cork_strfree(set->name);
    bz_user_data_free(set);
    free(set);
}

struct bz_value_provider *
bz_value_set_get_provider(struct bz_value_set *set, const char *key)
{
    return set->get(set->user_data, key);
}

const char *
bz_value_set_get(struct bz_value_set *set, const char *key, struct bz_env *env)
{
    struct bz_value_provider  *provider;
    rpp_check(provider = bz_value_set_get_provider(set, key));
    return bz_value_provider_get(provider, env);
}


/*-----------------------------------------------------------------------
 * Environments
 */

struct bz_env {
    const char  *name;
    cork_array(struct bz_value_set *)  sets;
    cork_array(struct bz_value_set *)  backup_sets;
    struct bz_var_table  *override_table;
};

struct bz_env *
bz_env_new(const char *name)
{
    struct bz_env  *env = cork_new(struct bz_env);
    struct bz_value_set  *set;
    env->name = cork_strdup(name);
    cork_array_init(&env->sets);
    cork_array_init(&env->backup_sets);

    /* Every environment comes with one var_table set for free, which takes
     * precedence over every other value set. */
    env->override_table = bz_var_table_new(name);
    set = bz_var_table_as_set(env->override_table);
    bz_env_add_set(env, set);

    return env;
}

void
bz_env_free(struct bz_env *env)
{
    size_t  i;

    for (i = 0; i < cork_array_size(&env->sets); i++) {
        struct bz_value_set  *set = cork_array_at(&env->sets, i);
        bz_value_set_free(set);
    }

    for (i = 0; i < cork_array_size(&env->backup_sets); i++) {
        struct bz_value_set  *set = cork_array_at(&env->backup_sets, i);
        bz_value_set_free(set);
    }

    cork_strfree(env->name);
    free(env);
}

const char *
bz_env_name(struct bz_env *env)
{
    return env->name;
}

void
bz_env_add_set(struct bz_env *env, struct bz_value_set *set)
{
    cork_array_append(&env->sets, set);
}

void
bz_env_add_backup_set(struct bz_env *env, struct bz_value_set *set)
{
    cork_array_append(&env->backup_sets, set);
}

struct bz_value_provider *
bz_env_get_provider(struct bz_env *env, const char *key)
{
    size_t  i;

    DEBUG("=== Looking for %s in %s\n", key, env->name);

    for (i = 0; i < cork_array_size(&env->sets); i++) {
        struct bz_value_set  *set = cork_array_at(&env->sets, i);
        struct bz_value_provider  *provider;
        DEBUG("=== Looking for %s in %s:%s\n", key, env->name, set->name);
        provider = bz_value_set_get_provider(set, key);
        if (provider != NULL) {
            DEBUG("=== FOUND %s in %s:%s\n", key, env->name, set->name);
            return provider;
        } else if (cork_error_occurred()) {
            return NULL;
        }
    }

    for (i = 0; i < cork_array_size(&env->backup_sets); i++) {
        struct bz_value_set  *set = cork_array_at(&env->backup_sets, i);
        struct bz_value_provider  *provider;
        DEBUG("=== Looking for %s in %s:%s\n", key, env->name, set->name);
        provider = bz_value_set_get_provider(set, key);
        if (provider != NULL) {
            DEBUG("=== FOUND %s in %s:%s\n", key, env->name, set->name);
            return provider;
        } else if (cork_error_occurred()) {
            return NULL;
        }
    }

    DEBUG("=== Couldn't find %s in %s\n", key, env->name);
    return NULL;
}

const char *
bz_env_get(struct bz_env *env, const char *key)
{
    struct bz_value_provider  *provider;
    rpp_check(provider = bz_env_get_provider(env, key));
    return bz_value_provider_get(provider, env);
}

void
bz_env_add_override(struct bz_env *env, const char *key,
                    struct bz_value_provider *value)
{
    bz_var_table_add(env->override_table, key, value);
}


/*-----------------------------------------------------------------------
 * Path values
 */

static const char *
bz_path_value__provide(void *user_data, struct bz_env *env)
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

struct bz_value_provider *
bz_path_value_new(struct cork_path *path)
{
    return bz_value_provider_new
        (path, bz_path_value__free, bz_path_value__provide);
}


/*-----------------------------------------------------------------------
 * String values
 */

static const char *
bz_string_value__provide(void *user_data, struct bz_env *env)
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

struct bz_value_provider *
bz_string_value_new(const char *value)
{
    const char  *copy = cork_strdup(value);
    return bz_value_provider_new
        ((void *) copy, bz_string_value__free, bz_string_value__provide);
}


/*-----------------------------------------------------------------------
 * Internal hash tables of variables
 */

struct bz_var_table {
    const char  *name;
    struct cork_hash_table  table;
};

struct bz_var_table *
bz_var_table_new(const char *name)
{
    struct bz_var_table  *table = cork_new(struct bz_var_table);
    table->name = cork_strdup(name);
    cork_string_hash_table_init(&table->table, 0);
    return table;
}

static enum cork_hash_table_map_result
free_var(struct cork_hash_table_entry *entry, void *user_data)
{
    const char  *key = entry->key;
    struct bz_value_provider  *value = entry->value;
    cork_strfree(key);
    bz_value_provider_free(value);
    return CORK_HASH_TABLE_MAP_DELETE;
}

void
bz_var_table_free(struct bz_var_table *table)
{
    cork_strfree(table->name);
    cork_hash_table_map(&table->table, free_var, NULL);
    cork_hash_table_done(&table->table);
    free(table);
}

void
bz_var_table_add(struct bz_var_table *table, const char *key,
                 struct bz_value_provider *value)
{
    bool  is_new;
    struct cork_hash_table_entry  *entry;

    entry = cork_hash_table_get_or_create(&table->table, (void *) key, &is_new);
    if (is_new) {
        entry->key = (void *) cork_strdup(key);
    } else {
        struct bz_value_provider  *old_value = entry->value;
        bz_value_provider_free(old_value);
    }
    entry->value = value;
}

struct bz_value_provider *
bz_var_table_get_provider(struct bz_var_table *table, const char *key)
{
    return cork_hash_table_get(&table->table, (void *) key);
}

const char *
bz_var_table_get(struct bz_var_table *table, const char *key,
                 struct bz_env *env)
{
    struct bz_value_provider  *provider =
        bz_var_table_get_provider(table, key);
    return (provider == NULL)? NULL: bz_value_provider_get(provider, env);
}

static struct bz_value_provider *
bz_var_table__set__get(void *user_data, const char *key)
{
    struct bz_var_table  *table = user_data;
    return bz_var_table_get_provider(table, key);
}

static void
bz_var_table__set__free(void *user_data)
{
    struct bz_var_table  *table = user_data;
    bz_var_table_free(table);
}

struct bz_value_set *
bz_var_table_as_set(struct bz_var_table *table)
{
    return bz_value_set_new
        (table->name, table, bz_var_table__set__free, bz_var_table__set__get);
}


/*-----------------------------------------------------------------------
 * Global and package-specific environments
 */

static struct bz_var_doc *
bz_var_doc_new(const char *name, struct bz_value_provider *value,
               const char *short_desc, const char *long_desc)
{
    struct bz_var_doc  *doc = cork_new(struct bz_var_doc);
    doc->name = cork_strdup(name);
    doc->value = value;
    if (short_desc == NULL) {
        short_desc = "";
    }
    doc->short_desc = cork_strdup(short_desc);
    if (long_desc == NULL) {
        long_desc = "";
    }
    doc->long_desc = cork_strdup(long_desc);
    return doc;
}

static void
bz_var_doc_free(struct bz_var_doc *doc)
{
    cork_strfree(doc->name);
    if (doc->value != NULL) {
        bz_value_provider_free(doc->value);
    }
    cork_strfree(doc->short_desc);
    cork_strfree(doc->long_desc);
    free(doc);
}

static struct cork_hash_table  global_docs;

static struct bz_value_provider *
bz_global_docs__set__get(void *user_data, const char *key)
{
    struct bz_var_doc  *doc = cork_hash_table_get(&global_docs, (void *) key);
    return (doc == NULL)? NULL: doc->value;
}

static enum cork_hash_table_map_result
free_doc(struct cork_hash_table_entry *entry, void *user_data)
{
    struct bz_var_doc  *doc = entry->value;
    bz_var_doc_free(doc);
    return CORK_HASH_TABLE_MAP_DELETE;
}

static void
bz_global_docs__set__free(void *user_data)
{
    cork_hash_table_map(&global_docs, free_doc, NULL);
    cork_hash_table_done(&global_docs);
}

static struct bz_value_set *
bz_global_docs_set(const char *name)
{
    return bz_value_set_new
        (name, NULL, bz_global_docs__set__free, bz_global_docs__set__get);
}

static struct bz_value_set *
bz_global_docs_unowned_set(const char *name)
{
    return bz_value_set_new
        (name, NULL, NULL, bz_global_docs__set__get);
}


static struct bz_env  *global = NULL;

static void
global_new(void)
{
    struct bz_value_set  *global_default_set;
    global = bz_env_new("global");
    cork_string_hash_table_init(&global_docs, 0);
    global_default_set = bz_global_docs_set("global defaults");
    bz_env_add_backup_set(global, global_default_set);
}

static void
global_free(void)
{
    bz_env_free(global);
}

static void
ensure_global_created(void)
{
    if (CORK_UNLIKELY(global == NULL)) {
        global_new();
        cork_cleanup_at_exit(0, global_free);
    }
}

void
bz_global_env_reset(void)
{
    if (global != NULL) {
        global_free();
        global_new();
    }
}


struct bz_env *
bz_global_env(void)
{
    ensure_global_created();
    return global;
}

struct bz_env *
bz_package_env_new_empty(const char *env_name)
{
    struct bz_env  *env;
    struct bz_value_set  *global_default_set;
    ensure_global_created();
    env = bz_env_new(env_name);
    global_default_set = bz_global_docs_unowned_set("global defaults");
    bz_env_add_backup_set(env, global_default_set);
    return env;
}

struct bz_env *
bz_package_env_new(const char *package_name, struct bz_version *version)
{
    const char  *version_string = bz_version_to_string(version);
    struct bz_env  *env = bz_package_env_new_empty(package_name);
    bz_env_add_override(env, "name", bz_string_value_new(package_name));
    bz_env_add_override(env, "version", bz_string_value_new(version_string));
    bz_version_free(version);
    return env;
}

int
bz_env_set_global_default(const char *key, struct bz_value_provider *value,
                          const char *short_desc, const char *long_desc)
{
    bool  is_new;
    struct cork_hash_table_entry  *entry;

    ensure_global_created();
    entry = cork_hash_table_get_or_create(&global_docs, (void *) key, &is_new);
    if (is_new) {
        struct bz_var_doc  *doc =
            bz_var_doc_new(key, value, short_desc, long_desc);
        entry->key = (void *) doc->name;
        entry->value = doc;
        return 0;
    } else {
        bz_value_provider_free(value);
        bz_bad_config("Variable %s defined twice", key);
        return -1;
    }
}

struct bz_var_doc *
bz_env_get_global_default(const char *name)
{
    struct bz_var_doc  *doc =
        cork_hash_table_get(&global_docs, (void *) name);
    if (CORK_UNLIKELY(doc == NULL)) {
        bz_bad_config("No variable named %s", name);
    }
    return doc;
}


/*-----------------------------------------------------------------------
 * Documenting variables
 */

#define bz_load_variables(prefix) \
    do { \
        extern int \
        bz_vars__##prefix(void); \
        rii_check(bz_vars__##prefix()); \
    } while (0)

int
bz_load_variable_definitions(void)
{
    bz_load_variables(global);
    bz_load_variables(package);

    /* builders */
    bz_load_variables(cmake);

    /* packagers */
    bz_load_variables(pacman);
    return 0;
}
