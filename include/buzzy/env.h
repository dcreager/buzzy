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

#include "buzzy/value.h"


struct bz_env;


/*-----------------------------------------------------------------------
 * Retrieving the value of a variable
 */

struct bz_value *
bz_env_get_value(struct bz_env *env, const char *name);

/* All of the following return an error if the value is malformed.  If required
 * is true, they also return an error if the value is missing.  You are not
 * responsible for freeing any results. */

bool
bz_env_get_bool(struct bz_env *env, const char *name, bool required);

long
bz_env_get_long(struct bz_env *env, const char *name, bool required);

struct cork_path *
bz_env_get_path(struct bz_env *env, const char *name, bool required);

const char *
bz_env_get_string(struct bz_env *env, const char *name, bool required);

struct bz_version *
bz_env_get_version(struct bz_env *env, const char *name, bool required);


/*-----------------------------------------------------------------------
 * Global and package-specific environments
 */

struct bz_env *
bz_global_env(void);

struct bz_env *
bz_repo_env_new_empty(void);

/* env_name is usually the same as the package name, but if you don't know the
 * package name yet, you can use something like "new package". */
struct bz_env *
bz_package_env_new_empty(struct bz_env *repo_env, const char *env_name);

/* Takes control of version */
struct bz_env *
bz_package_env_new(struct bz_env *repo_env, const char *package_name,
                   struct bz_version *version);

/* Every global and package-specific environment will use these default values
 * for any variables that aren't explicitly defined. */
int
bz_env_set_global_default(const char *name, struct bz_value *value,
                          const char *short_desc, const char *long_desc);

struct bz_var_doc {
    const char  *name;
    const char  *short_desc;
    const char  *long_desc;
    struct bz_value  *value;
};

struct bz_var_doc *
bz_env_get_global_default(const char *name);

/* Only needed for reproducible test cases */
void
bz_global_env_reset(void);


/*-----------------------------------------------------------------------
 * Environments
 */

struct bz_env *
bz_env_new(const char *name);

void
bz_env_free(struct bz_env *env);

const char *
bz_env_name(struct bz_env *env);

const char *
bz_env_base_path(struct bz_env *env);

void
bz_env_set_base_path(struct bz_env *env, const char *base_path);

/* Takes control of set */
void
bz_env_add_set(struct bz_env *env, struct bz_value *set);

/* Takes control of set */
void
bz_env_add_backup_set(struct bz_env *env, struct bz_value *set);

/* Each of the sets in env are checked for key.  The sets added with
 * bz_env_add_set are checked first, in order, followed by the sets added with
 * bz_env_add_backup_set, in order.  If "set" is not-NULL, it will be filled in
 * with the value set that provided the result. */
struct bz_value *
bz_env_get_value(struct bz_env *env, const char *key);

const char *
bz_env_get(struct bz_env *env, const char *key);

/* Every environment comes with two var_table sets for free.  The first takes
 * precedence over every other value set, the other is overridden by every other
 * value set. */

void
bz_env_add_override(struct bz_env *env, const char *key,
                    struct bz_value *value);

void
bz_env_add_backup(struct bz_env *env, const char *key,
                  struct bz_value *value);


/* A value that tries to find variables in env.  This lets you "nest" env inside
 * of some other environment.  We do not take control of env; it's your
 * responsibility to make sure that it's valid whenever this set is used. */
struct bz_value *
bz_env_as_value(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Documenting variables
 */

int
bz_load_variable_definitions(void);

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
        int  __rc; \
        struct bz_value  *__dv = default_value; \
        if (CORK_UNLIKELY(cork_error_occurred())) { \
            return; \
        } \
        __rc = bz_env_set_global_default(name, __dv, short_desc, long_desc); \
        if (CORK_UNLIKELY(__rc != 0)) { \
            return; \
        } \
    } while (0)

#define bz_package_variable(c_name, name, default_value, short_desc, long_desc) \
    bz_global_variable(c_name, name, default_value, short_desc, long_desc)

#define bz_repo_variable(c_name, name, default_value, short_desc, long_desc) \
    bz_global_variable(c_name, name, default_value, short_desc, long_desc)


#endif /* BUZZY_ENV_H */
