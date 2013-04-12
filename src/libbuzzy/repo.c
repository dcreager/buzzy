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

#include "buzzy/action.h"
#include "buzzy/callbacks.h"
#include "buzzy/env.h"
#include "buzzy/repo.h"


/*-----------------------------------------------------------------------
 * Repository variables
 */

bz_define_variables(repo)
{
    bz_repo_variable(
        repo_base_dir, "repo.base_dir",
        NULL,
        "The base path of the files defining the repository",
        ""
    );

    bz_repo_variable(
        repo_config_dir, "repo.config_dir",
        bz_interpolated_value_new("${repo.base_dir}/.buzzy"),
        "The base path of the files defining the repository",
        ""
    );

    bz_repo_variable(
        repo_yaml, "repo.repo_yaml",
        bz_interpolated_value_new("${repo.config_dir}/repo.yaml"),
        "The location of the YAML file defining the repository",
        ""
    );

    bz_repo_variable(
        repo_links_path, "repo.links_yaml",
        bz_interpolated_value_new("${repo.config_dir}/links.yaml"),
        "The location of the YAML file defining linked repositories",
        ""
    );

    bz_repo_variable(
        repo_package_yaml, "repo.package_yaml",
        bz_interpolated_value_new("${repo.config_dir}/package.yaml"),
        "The location of the YAML file defining the repository's package",
        ""
    );

    bz_repo_variable(
        repo_git_dir, "repo.git_dir",
        bz_interpolated_value_new("${repo.base_dir}/.git"),
        "The location of the .git directory in a git checkout",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Repositories
 */

struct bz_repo {
    struct bz_env  *env;
    void  *user_data;
    bz_free_f  user_data_free;
    bz_repo_load_f  load;
    bz_repo_update_f  update;
    struct bz_action  *load_action;
    struct bz_action  *update_action;
    struct bz_package  *default_package;
};

struct bz_repo *
bz_repo_new(struct bz_env *env,
            void *user_data, bz_free_f user_data_free,
            bz_repo_load_f load,
            bz_repo_update_f update)
{
    struct bz_repo  *repo = cork_new(struct bz_repo);
    repo->env = env;
    repo->user_data = user_data;
    repo->user_data_free = user_data_free;
    repo->load = load;
    repo->update = update;
    repo->load_action = NULL;
    repo->update_action = NULL;
    repo->default_package = NULL;
    return repo;
}

void
bz_repo_free(struct bz_repo *repo)
{
    if (repo->load_action != NULL) {
        bz_action_free(repo->load_action);
    }
    if (repo->update_action != NULL) {
        bz_action_free(repo->update_action);
    }
    bz_user_data_free(repo);
    bz_env_free(repo->env);
    free(repo);
}

struct bz_env *
bz_repo_env(struct bz_repo *repo)
{
    return repo->env;
}

struct bz_action *
bz_repo_load(struct bz_repo *repo)
{
    if (repo->load_action == NULL) {
        repo->load_action = repo->load(repo->user_data, repo->env);
    }
    return repo->load_action;
}

struct bz_action *
bz_repo_update(struct bz_repo *repo)
{
    if (repo->update_action == NULL) {
        repo->update_action = repo->update(repo->user_data, repo->env);
    }
    return repo->update_action;
}

struct bz_package *
bz_repo_default_package(struct bz_repo *repo)
{
    return repo->default_package;
}

void
bz_repo_set_default_package(struct bz_repo *repo, struct bz_package *package)
{
    repo->default_package = package;
}


/*-----------------------------------------------------------------------
 * Repository registry
 */

static bool  first_initialization = true;
static bool  repos_initialized = false;
static cork_array(struct bz_repo *)  repos;

static void
repos_free(void)
{
    if (repos_initialized) {
        size_t  i;
        for (i = 0; i < cork_array_size(&repos); i++) {
            struct bz_repo  *repo = cork_array_at(&repos, i);
            bz_repo_free(repo);
        }
        cork_array_done(&repos);
        repos_initialized = false;
    }
}

static void
repos_init(void)
{
    if (!repos_initialized) {
        cork_array_init(&repos);
        if (first_initialization) {
            cork_cleanup_at_exit(0, repos_free);
            first_initialization = false;
        }
        repos_initialized = true;
    }
}

void
bz_repo_registry_reset(void)
{
    repos_free();
    repos_init();
}

void
bz_repo_register(struct bz_repo *repo)
{
    repos_init();
    cork_array_append(&repos, repo);
}

size_t
bz_repo_registry_count(void)
{
    repos_init();
    return cork_array_size(&repos);
}

struct bz_repo *
bz_repo_registry_get(size_t index)
{
    repos_init();
    return cork_array_at(&repos, index);
}

struct bz_action_phase *
bz_repo_registry_load_all(void)
{
    size_t  i;
    struct bz_action_phase  *phase;

    repos_init();
    phase = bz_action_phase_new("Load repositories:");
    for (i = 0; i < cork_array_size(&repos); i++) {
        struct bz_repo  *repo = cork_array_at(&repos, i);
        struct bz_action  *action;
        ep_check(action = bz_repo_load(repo));
        bz_action_phase_add(phase, action);
    }
    return phase;

error:
    bz_action_phase_free(phase);
    return NULL;
}

struct bz_action_phase *
bz_repo_registry_update_all(void)
{
    size_t  i;
    struct bz_action_phase  *phase;

    repos_init();
    phase = bz_action_phase_new("Update repositories:");
    for (i = 0; i < cork_array_size(&repos); i++) {
        struct bz_repo  *repo = cork_array_at(&repos, i);
        struct bz_action  *action;
        ep_check(action = bz_repo_update(repo));
        bz_action_phase_add(phase, action);
    }
    return phase;

error:
    bz_action_phase_free(phase);
    return NULL;
}
