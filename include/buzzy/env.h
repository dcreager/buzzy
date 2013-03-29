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
#include <libcork/os.h>

#include "buzzy/callbacks.h"


struct bz_env;
struct bz_value_provider;
struct bz_value_set;
struct bz_var_table;


/*-----------------------------------------------------------------------
 * Global and package-specific environments
 */

struct bz_env *
bz_global_env(void);

/* env_name is usually the same as the package name, but if you don't know the
 * package name yet, you can use something like "new package". */
struct bz_env *
bz_package_env_new(const char *env_name);

/* Every global and package-specific environment will use these default values
 * for any variables that aren't explicitly defined. */
void
bz_env_set_global_default(const char *key, struct bz_value_provider *value);

/* Only needed for reproducible test cases */
void
bz_global_env_reset(void);


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

/* Takes control of path */
struct bz_value_provider *
bz_path_value_new(struct cork_path *path);

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


/*-----------------------------------------------------------------------
 * Documenting variables
 */

int
bz_load_variable_definitions(void);

void
bz_define_global_(const char *name, struct bz_value_provider *default_value,
                  const char *short_desc, const char *long_desc);

void
bz_define_package_(const char *name, struct bz_value_provider *default_value,
                   const char *short_desc, const char *long_desc);

#define bz_define_variables(prefix) \
static void \
bz_define_vars__##prefix(void); \
\
int \
bz_vars__##prefix(void) \
{ \
    cork_error_clear(); \
    bz_define_vars__##prefix(); \
    if (CORK_UNLIKELY(cork_error_occurred())) { \
        return -1; \
    } else { \
        return 0; \
    } \
} \
\
static void \
bz_define_vars__##prefix(void) \

#define bz_global_variable(c_name, name, default_value, short_desc, long_desc) \
    do { \
        struct bz_value_provider  *__dv = default_value; \
        if (CORK_UNLIKELY(__dv == NULL)) { \
            return; \
        } \
        bz_define_global_(name, __dv, short_desc, long_desc); \
    } while (0)

#define bz_package_variable(c_name, name, default_value, short_desc, long_desc) \
    do { \
        struct bz_value_provider  *__dv = default_value; \
        if (CORK_UNLIKELY(__dv == NULL)) { \
            return; \
        } \
        bz_define_package_(name, __dv, short_desc, long_desc); \
    } while (0)


#endif /* BUZZY_ENV_H */
