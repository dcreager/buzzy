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
#include "buzzy/env.h"
#include "buzzy/package.h"


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
            void *user_data, cork_free_f free_user_data,
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

struct bz_package *
bz_repo_default_package(struct bz_repo *repo);

void
bz_repo_set_default_package(struct bz_repo *repo, struct bz_package *package);


/* Takes control of repo; it will be automatically freed for you. */
void
bz_repo_register(struct bz_repo *repo);

size_t
bz_repo_registry_count(void);

struct bz_repo *
bz_repo_registry_get(size_t index);

void
bz_repo_registry_reset(void);

struct bz_action_phase *
bz_repo_registry_load_all(void);

struct bz_action_phase *
bz_repo_registry_update_all(void);


/*-----------------------------------------------------------------------
 * Built-in repository types
 */

/* Any repository that eventually places a standard repository directory layout
 * in the local filesystem should use this function in its load action to
 * actually load in the contents of the repository directory.  The repository's
 * environment should make sure that "repo.base_path" points at the repository
 * directory. */
int
bz_filesystem_repo_load(struct bz_repo *repo);


/* A repository that already exists on the filesystem.  We won't ever try to
 * update it, since we don't know how.  "path" should point at the repository
 * directory (which should contain a directory called ".buzzy").  We assume that
 * the repository is embedded in some source checkout (so the "source_dir"
 * should also be set to "path"). */
struct bz_repo *
bz_local_filesystem_repo_new(const char *path);

/* Search for a buzzy repository in the current directory, or any of its parent
 * directories. */
struct bz_repo *
bz_local_filesystem_repo_find(const char *path);


#endif /* BUZZY_REPO_H */
