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
bz_value_provider_new(void *user_data, bz_user_data_free_f user_data_free,
                      bz_provide_value_f provide_value);

void
bz_value_provider_free(struct bz_value_provider *provider);

const char *
bz_value_provider_get(struct bz_value_provider *provider, struct bz_env *env);


/*-----------------------------------------------------------------------
 * Built-in value providers
 */

struct bz_value_provider *
bz_string_value_new(const char *value);


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


#endif /* BUZZY_ENV_H */
