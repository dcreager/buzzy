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
#include "buzzy/repo.h"

#define CLOG_CHANNEL  "repo"


/*-----------------------------------------------------------------------
 * Repository variables
 */

bz_define_variables(repo)
{
    bz_global_variable(
        repo_dir, "repo_dir",
        bz_interpolated_value_new("${work_dir}/repos"),
        "Where cloned copied of remote repositories should be placed",
        ""
    );

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
    struct bz_package  *default_package;
    cork_array(struct bz_repo *)  links;
    const char  *name;

    void  *user_data;
    cork_free_f  free_user_data;
    bz_repo_load_f  load;
    bz_repo_update_f  update;
    bool  loaded;
    bool  updated;
};

struct bz_repo *
bz_repo_new(struct bz_env *env,
            void *user_data, cork_free_f free_user_data,
            bz_repo_load_f load,
            bz_repo_update_f update)
{
    struct bz_repo  *repo = cork_new(struct bz_repo);
    repo->env = env;
    repo->name = NULL;
    repo->default_package = NULL;
    cork_array_init(&repo->links);
    repo->user_data = user_data;
    repo->free_user_data = free_user_data;
    repo->load = load;
    repo->update = update;
    repo->loaded = false;
    repo->updated = false;
    return repo;
}

void
bz_repo_free(struct bz_repo *repo)
{
    cork_free_user_data(repo);
    cork_array_done(&repo->links);
    bz_env_free(repo->env);
    free(repo);
}

const char *
bz_repo_name(struct bz_repo *repo)
{
    if (repo->name == NULL) {
        repo->name = bz_env_get_string(repo->env, "repo.name", true);
    }
    return repo->name;
}

struct bz_env *
bz_repo_env(struct bz_repo *repo)
{
    return repo->env;
}

size_t
bz_repo_link_count(struct bz_repo *repo)
{
    return cork_array_size(&repo->links);
}

struct bz_repo *
bz_repo_link(struct bz_repo *repo, size_t index)
{
    return cork_array_at(&repo->links, index);
}

void
bz_repo_add_link(struct bz_repo *repo, struct bz_repo *other)
{
    struct bz_value  *other_value;
    cork_array_append(&repo->links, other);
    /* Each child repository should be able to see any variables defined in its
     * parent repositories. */
    other_value = bz_value_copy(bz_env_as_value(other->env));
    bz_env_add_backup_set(repo->env, other_value);
}

int
bz_repo_load(struct bz_repo *repo)
{
    if (!repo->loaded) {
        repo->loaded = true;
        clog_info("Load repository %s", bz_repo_name(repo));
        return repo->load(repo->user_data, repo->env);
    } else {
        return 0;
    }
}

int
bz_repo_update(struct bz_repo *repo)
{
    if (!repo->updated) {
        repo->updated = true;
        return repo->update(repo->user_data, repo->env);
    } else {
        return 0;
    }
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

int
bz_repo_registry_load_all(void)
{
    size_t  i;
    repos_init();
    for (i = 0; i < cork_array_size(&repos); i++) {
        struct bz_repo  *repo = cork_array_at(&repos, i);
        rii_check(bz_repo_load(repo));
    }
    return 0;
}

int
bz_repo_registry_update_all(void)
{
    size_t  i;
    repos_init();
    for (i = 0; i < cork_array_size(&repos); i++) {
        struct bz_repo  *repo = cork_array_at(&repos, i);
        rii_check(bz_repo_update(repo));
    }
    return 0;
}
