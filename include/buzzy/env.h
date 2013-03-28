/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_ENV_H
#define BUZZY_ENV_H

#include <libcork/core.h>

#include "buzzy/callbacks.h"


struct bz_env;
struct bz_value_provider;
struct bz_value_set;
struct bz_var_table;


/*-----------------------------------------------------------------------
 * Value providers
 */

typedef const char *
(*bz_provide_value_f)(void *user_data, struct bz_env *env);

struct bz_value_provider *
bz_value_provider_new(void *user_data, bz_free_f user_data_free,
                      bz_provide_value_f provide_value);

void
bz_value_provider_free(struct bz_value_provider *provider);

const char *
bz_value_provider_get(struct bz_value_provider *provider, struct bz_env *env);


/*-----------------------------------------------------------------------
 * Value sets
 */

/* Shouldn't raise an error if the key is missing. */
typedef struct bz_value_provider *
(*bz_value_set_get_f)(void *user_data, const char *key);

struct bz_value_set *
bz_value_set_new(const char *name, void *user_data, bz_free_f user_data_free,
                 bz_value_set_get_f get);

void
bz_value_set_free(struct bz_value_set *set);

struct bz_value_provider *
bz_value_set_get_provider(struct bz_value_set *set, const char *key);

const char *
bz_value_set_get(struct bz_value_set *set, const char *key, struct bz_env *env);


/*-----------------------------------------------------------------------
 * Environments
 */

struct bz_env *
bz_env_new(const char *name);

void
bz_env_free(struct bz_env *env);

/* Takes control of set */
void
bz_env_add_set(struct bz_env *env, struct bz_value_set *set);

/* Takes control of set */
void
bz_env_add_backup_set(struct bz_env *env, struct bz_value_set *set);

/* Each of the sets in env are checked for key.  The sets added with
 * bz_env_add_set are checked first, in order, followed by the sets added with
 * bz_env_add_backup_set, in order. */
struct bz_value_provider *
bz_env_get_provider(struct bz_env *env, const char *key);

const char *
bz_env_get(struct bz_env *env, const char *key);


/*-----------------------------------------------------------------------
 * Built-in value providers
 */

struct bz_value_provider *
bz_string_value_new(const char *value);

/* ${var} is substituted with the value of another variable */
struct bz_value_provider *
bz_interpolated_value_new(const char *template_value);


/*-----------------------------------------------------------------------
 * Internal hash tables of variables
 */

struct bz_var_table *
bz_var_table_new(const char *name);

void
bz_var_table_free(struct bz_var_table *table);

/* Takes control of provider; overwrites any existing entry for key */
void
bz_var_table_add(struct bz_var_table *table, const char *key,
                 struct bz_value_provider *value);

/* Not an error if key isn't present. */
struct bz_value_provider *
bz_var_table_get_provider(struct bz_var_table *table, const char *key);

/* Not an error if key isn't present. */
const char *
bz_var_table_get(struct bz_var_table *table, const char *key,
                 struct bz_env *env);

/* The new set controls table; you should not free it after calling this
 * function. */
struct bz_value_set *
bz_var_table_as_set(struct bz_var_table *table);


#endif /* BUZZY_ENV_H */
