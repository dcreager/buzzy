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
