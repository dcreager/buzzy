/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/callbacks.h"
#include "buzzy/env.h"


/*-----------------------------------------------------------------------
 * Value providers
 */

struct bz_value_provider {
    void  *user_data;
    bz_user_data_free_f  user_data_free;
    bz_provide_value_f  provide_value;
};

struct bz_value_provider *
bz_value_provider_new(void *user_data, bz_user_data_free_f user_data_free,
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
    bz_user_data_free_f  user_data_free;
    bz_value_set_get_f  get;
};

struct bz_value_set *
bz_value_set_new(const char *name,
                 void *user_data, bz_user_data_free_f user_data_free,
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
};

struct bz_env *
bz_env_new(const char *name)
{
    struct bz_env  *env = cork_new(struct bz_env);
    env->name = cork_strdup(name);
    cork_array_init(&env->sets);
    cork_array_init(&env->backup_sets);
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

    for (i = 0; i < cork_array_size(&env->sets); i++) {
        struct bz_value_set  *set = cork_array_at(&env->sets, i);
        struct bz_value_provider  *provider =
            bz_value_set_get_provider(set, key);
        if (provider != NULL) {
            return provider;
        } else if (cork_error_occurred()) {
            return NULL;
        }
    }

    for (i = 0; i < cork_array_size(&env->backup_sets); i++) {
        struct bz_value_set  *set = cork_array_at(&env->backup_sets, i);
        struct bz_value_provider  *provider =
            bz_value_set_get_provider(set, key);
        if (provider != NULL) {
            return provider;
        } else if (cork_error_occurred()) {
            return NULL;
        }
    }

    return NULL;
}

const char *
bz_env_get(struct bz_env *env, const char *key)
{
    struct bz_value_provider  *provider;
    rpp_check(provider = bz_env_get_provider(env, key));
    return bz_value_provider_get(provider, env);
}


/*-----------------------------------------------------------------------
 * Built-in value providers
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
