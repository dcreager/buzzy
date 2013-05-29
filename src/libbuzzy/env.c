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
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/value.h"
#include "buzzy/version.h"

#define CLOG_CHANNEL  "env"


/*-----------------------------------------------------------------------
 * Environments
 */

struct bz_env {
    const char  *name;
    struct bz_union_map  *env_map;
    struct bz_value  *value;
    struct bz_union_map  *sets_map;
    struct bz_value  *sets;
    struct bz_union_map  *backup_sets_map;
    struct bz_value  *backup_sets;
    struct bz_value  *override_map;
    struct bz_value  *backup_map;
};

struct bz_env *
bz_env_new(const char *name)
{
    struct bz_env  *env = cork_new(struct bz_env);
    env->name = cork_strdup(name);
    env->sets_map = bz_union_map_new();
    env->sets = bz_union_map_as_value(env->sets_map);
    env->backup_sets_map = bz_union_map_new();
    env->backup_sets = bz_union_map_as_value(env->backup_sets_map);

    /* Every environment comes with two map sets for free.  The first
     * takes precedence over every other value set, the other is overridden by
     * every other value set. */

    env->override_map = bz_map_new();
    bz_env_add_set(env, env->override_map);

    env->backup_map = bz_map_new();
    bz_env_add_backup_set(env, env->backup_map);

    env->env_map = bz_union_map_new();
    bz_union_map_add(env->env_map, env->sets);
    bz_union_map_add(env->env_map, env->backup_sets);
    env->value = bz_union_map_as_value(env->env_map);
    return env;
}

void
bz_env_free(struct bz_env *env)
{
    bz_value_free(env->value);
    cork_strfree(env->name);
    free(env);
}

const char *
bz_env_name(struct bz_env *env)
{
    return env->name;
}

const char *
bz_env_base_path(struct bz_env *env)
{
    return bz_value_base_path(env->value);
}

void
bz_env_set_base_path(struct bz_env *env, const char *base_path)
{
    bz_value_set_base_path(env->value, base_path);
}

void
bz_env_add_set(struct bz_env *env, struct bz_value *set)
{
    bz_union_map_add(env->sets_map, set);
}

void
bz_env_add_backup_set(struct bz_env *env, struct bz_value *set)
{
    bz_union_map_add(env->backup_sets_map, set);
}

void
bz_env_add_override(struct bz_env *env, const char *key, struct bz_value *value)
{
    bz_value_set_nested(env->override_map, key, value, true);
}

void
bz_env_add_backup(struct bz_env *env, const char *key, struct bz_value *value)
{
    bz_value_set_nested(env->backup_map, key, value, true);
}

struct bz_value *
bz_env_as_value(struct bz_env *env)
{
    return env->value;
}


/*-----------------------------------------------------------------------
 * Retrieving the value of a variable
 */

struct bz_value *
bz_env_get_value(struct bz_env *env, const char *name)
{
    return bz_value_get_nested(env->value, name);
}

bool
bz_env_get_bool(struct bz_env *env, const char *name, bool required)
{
    return bz_value_get_bool(env->value, name, required);
}

long
bz_env_get_long(struct bz_env *env, const char *name, bool required)
{
    return bz_value_get_long(env->value, name, required);
}

struct cork_path *
bz_env_get_path(struct bz_env *env, const char *name, bool required)
{
    return bz_value_get_path(env->value, name, required);
}

const char *
bz_env_get_string(struct bz_env *env, const char *name, bool required)
{
    return bz_value_get_string(env->value, name, required);
}

struct bz_version *
bz_env_get_version(struct bz_env *env, const char *name, bool required)
{
    return bz_value_get_version(env->value, name, required);
}


/*-----------------------------------------------------------------------
 * Global and package-specific environments
 */

static struct bz_var_doc *
bz_var_doc_new(const char *name, struct bz_value *value,
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
        bz_value_free(doc->value);
    }
    cork_strfree(doc->short_desc);
    cork_strfree(doc->long_desc);
    free(doc);
}

static struct cork_hash_table  global_docs;
static struct bz_value  *global_values = NULL;
static struct bz_env  *global = NULL;

static int
load_config_path_list(struct cork_path_list *paths, const char *rel_path)
{
    struct cork_file_list  *files = NULL;
    size_t  i;
    size_t  count;
    clog_debug("Searching for configuration files in %s",
               cork_path_list_to_string(paths));
    ep_check(files = cork_path_list_find_files(paths, rel_path));
    count = cork_file_list_size(files);
    for (i = 0; i < count; i++) {
        struct cork_file  *file = cork_file_list_get(files, i);
        const struct cork_path  *path = cork_file_path(file);
        struct bz_value  *value;
        clog_info("Loading configuration from %s", cork_path_get(path));
        ep_check(value = bz_yaml_value_new_from_file(cork_path_get(path)));
        bz_env_add_set(global, value);
    }
    cork_path_list_free(paths);
    cork_file_list_free(files);
    return 0;

error:
    cork_path_list_free(paths);
    if (files != NULL) {
        cork_file_list_free(files);
    }
    return -1;
}

static int
load_config_files(void)
{
    struct cork_path_list  *paths;
    /* Configuration directories first (e.g., $HOME/.config) */
    rip_check(paths = cork_path_config_paths());
    rii_check(load_config_path_list(paths, "buzzy.yaml"));
    /* Then data directories (e.g., $PREFIX/share) */
    rip_check(paths = cork_path_data_paths());
    rii_check(load_config_path_list(paths, "buzzy/config.yaml"));
    return 0;
}

static void
global_new(void)
{

    global = bz_env_new("global");
    cork_string_hash_table_init(&global_docs, 0);

    /* Check for buzzy.yaml files in a bunch of configuration directories. */
    if (CORK_UNLIKELY(load_config_files() != 0)) {
        fprintf(stderr, "%s\n", cork_error_message());
        exit(EXIT_FAILURE);
    }

    /* The precompiled default values */
    global_values = bz_map_new();
    bz_env_add_backup_set(global, global_values);
}

static enum cork_hash_table_map_result
free_doc(struct cork_hash_table_entry *entry, void *user_data)
{
    struct bz_var_doc  *doc = entry->value;
    bz_var_doc_free(doc);
    return CORK_HASH_TABLE_MAP_DELETE;
}

static void
global_free(void)
{
    cork_hash_table_map(&global_docs, free_doc, NULL);
    cork_hash_table_done(&global_docs);
    bz_env_free(global);
    global = NULL;
    global_values = NULL;
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
bz_repo_env_new_empty(void)
{
    struct bz_env  *env;
    struct bz_value  *global_default_set;
    ensure_global_created();
    env = bz_env_new("repository");
    global_default_set = bz_value_copy(global_values);
    bz_env_add_backup_set(env, global_default_set);
    return env;
}

struct bz_env *
bz_package_env_new_empty(struct bz_env *repo_env, const char *env_name)
{
    struct bz_env  *env;
    struct bz_value  *global_default_set;
    ensure_global_created();
    env = bz_env_new(env_name);
    if (repo_env != NULL) {
        struct bz_value  *repo_set = bz_env_as_value(repo_env);
        bz_env_add_backup_set(env, bz_value_copy(repo_set));
    }
    global_default_set = bz_value_copy(global_values);
    bz_env_add_backup_set(env, global_default_set);
    return env;
}

struct bz_env *
bz_package_env_new(struct bz_env *repo_env, const char *package_name,
                   struct bz_version *version)
{
    const char  *version_string = bz_version_to_string(version);
    struct bz_env  *env = bz_package_env_new_empty(repo_env, package_name);
    bz_env_add_override(env, "name", bz_string_value_new(package_name));
    bz_env_add_override(env, "version", bz_string_value_new(version_string));
    bz_version_free(version);
    return env;
}

int
bz_env_set_global_default(const char *key, struct bz_value *value,
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
        if (value == NULL) {
            return 0;
        } else {
            struct bz_value  *copy = bz_value_copy(value);
            return bz_value_set_nested(global_values, key, copy, false);
        }
    } else {
        bz_value_free(value);
        bz_bad_config("Variable %s defined twice", key);
        return -1;
    }
}

struct bz_var_doc *
bz_env_get_global_default(const char *name, bool required)
{
    struct bz_var_doc  *doc =
        cork_hash_table_get(&global_docs, (void *) name);
    if (required && CORK_UNLIKELY(doc == NULL)) {
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
    bz_load_variables(repo);

    /* builders */
    bz_load_variables(autotools);
    bz_load_variables(cmake);

    /* packagers */
    bz_load_variables(pacman);
    bz_load_variables(rpm);
    return 0;
}
