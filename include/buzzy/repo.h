/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_REPO_H
#define BUZZY_REPO_H

#include <libcork/core.h>

#include "buzzy/action.h"
#include "buzzy/callbacks.h"
#include "buzzy/env.h"


/*-----------------------------------------------------------------------
 * Repositories
 */

struct bz_repo;

typedef struct bz_action *
(*bz_repo_load_f)(void *user_data, struct bz_env *env);

typedef struct bz_action *
(*bz_repo_update_f)(void *user_data, struct bz_env *env);

/* Takes control of env */
struct bz_repo *
bz_repo_new(struct bz_env *env,
            void *user_data, bz_free_f user_data_free,
            bz_repo_load_f load,
            bz_repo_update_f update);

void
bz_repo_free(struct bz_repo *repo);

struct bz_env *
bz_repo_env(struct bz_repo *repo);

struct bz_action *
bz_repo_load(struct bz_repo *repo);

struct bz_action *
bz_repo_update(struct bz_repo *repo);


/*-----------------------------------------------------------------------
 * Built-in repository types
 */

/* Path should be a ".buzzy" directory containing a bunch of YAML files */
struct bz_repo *
bz_filesystem_repo_new(const char *path, struct bz_action *update);

/* Search for a buzzy repository in the current directory, or any of its parent
 * directories. */
struct bz_repo *
bz_filesystem_repo_find(const char *path);


#endif /* BUZZY_REPO_H */
