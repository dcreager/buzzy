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
#include "buzzy/built.h"
#include "buzzy/callbacks.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/repo.h"
#include "buzzy/distro/git.h"


/*-----------------------------------------------------------------------
 * Repository variables
 */

bz_define_variables(repo)
{
    bz_repo_variable(
        repo_base_path, "repo_base_path",
        NULL,
        "The base path of the files defining the repository",
        ""
    );

    bz_repo_variable(
        repo_path, "repo_path",
        bz_interpolated_value_new("${repo_base_path}/repo.yaml"),
        "The location of the YAML file defining the repository",
        ""
    );

    bz_repo_variable(
        repo_links_path, "repo_links_path",
        bz_interpolated_value_new("${repo_base_path}/links.yaml"),
        "The location of the YAML file defining linked repositories",
        ""
    );

    bz_repo_variable(
        yaml_package_file_path, "yaml_package_file_path",
        bz_interpolated_value_new("${repo_base_path}/package.yaml"),
        "The location of the YAML file defining the repository's package",
        ""
    );

    bz_repo_variable(
        git_path, "git_path",
        bz_interpolated_value_new("${repo_base_path}/../.git"),
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
            bz_repo_update_f update,
            struct bz_package *default_package)
{
    struct bz_repo  *repo = cork_new(struct bz_repo);
    repo->env = env;
    repo->user_data = user_data;
    repo->user_data_free = user_data_free;
    repo->load = load;
    repo->update = update;
    repo->load_action = NULL;
    repo->update_action = NULL;
    repo->default_package = default_package;
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


/*-----------------------------------------------------------------------
 * Repository registry
 */

static bool  first_initialization = true;
static bool  repos_initialized = false;
static cork_array(struct bz_repo *)  repos;
static struct bz_action  *registry_load;

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
        bz_action_free(registry_load);
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
        registry_load = bz_noop_action_new();
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
    bz_action_add_pre(registry_load, bz_repo_load(repo));
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

struct bz_action *
bz_repo_registry_load_all(void)
{
    repos_init();
    return registry_load;
}


/*-----------------------------------------------------------------------
 * Filesystem repository
 */

struct bz_filesystem_repo {
    const char  *path;
    struct bz_env  *env;
    struct bz_action  *update;
    struct bz_package  *default_package;
};

static void
bz_filesystem__free(void *user_data)
{
    struct bz_filesystem_repo  *repo = user_data;
    cork_strfree(repo->path);
    free(repo);
}

static int
bz_filesystem__single_package_pdb(struct bz_filesystem_repo *repo)
{
    struct bz_pdb  *pdb = NULL;

    /* If there's a default package for this repository, create a single-package
     * PDB for it. */
    if (repo->default_package != NULL) {
        const char  *package_name = bz_package_name(repo->default_package);
        rip_check(pdb = bz_single_package_pdb_new
                  (package_name, repo->default_package));
        bz_pdb_register(pdb);
    }

    return 0;
}

static int
bz_filesystem__load__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_filesystem_repo  *repo = user_data;
    cork_buffer_append_printf(dest, "Load %s", repo->path);
    return 0;
}

static int
bz_filesystem__load__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_filesystem__load__perform(void *user_data)
{
    struct bz_filesystem_repo  *repo = user_data;
    rii_check(bz_filesystem__single_package_pdb(repo));
    return 0;
}

static struct bz_action *
bz_filesystem__load(void *user_data, struct bz_env *env)
{
    struct bz_filesystem_repo  *repo = user_data;
    return bz_action_new
        (repo, NULL,
         bz_filesystem__load__message,
         bz_filesystem__load__is_needed,
         bz_filesystem__load__perform);
}

static struct bz_action *
bz_filesystem__update(void *user_data, struct bz_env *env)
{
    struct bz_filesystem_repo  *repo = user_data;
    return repo->update;
}

static int
bz_filesystem_repo_add_git_version(struct bz_filesystem_repo *repo)
{
    bool  exists;
    struct cork_path  *git_path = NULL;

    /* See if the repository is a git checkout. */
    rip_check(git_path = bz_env_get_path(repo->env, "git_path", true));
    ei_check(bz_file_exists(cork_path_get(git_path), &exists));
    cork_path_free(git_path);
    if (!exists) {
        return 0;
    }

    /* If so, add a default value for the "version" variable that parses the
     * result of "git describe". */
    bz_env_add_backup(repo->env, "version", bz_git_version_value_new());
    return 0;

error:
    cork_path_free(git_path);
    return -1;
}

static int
bz_filesystem_repo_create_default_package(struct bz_filesystem_repo *repo)
{
    bool  exists;
    struct cork_path  *package_path = NULL;
    struct bz_env  *package_env = NULL;
    struct bz_value_set  *package_yaml;
    struct bz_package  *package = NULL;

    /* See if the repository has a package.yaml file. */
    rip_check(package_path = bz_env_get_path
              (repo->env, "yaml_package_file_path", true));
    ei_check(bz_file_exists(cork_path_get(package_path), &exists));
    if (!exists) {
        return 0;
    }

    /* If so, create a default package from it. */
    ep_check(package_env = bz_package_env_new_empty(repo->env, "package"));
    ep_check(package_yaml = bz_yaml_value_set_new_from_file
             ("package", cork_path_get(package_path)));
    bz_env_add_set(package_env, package_yaml);
    bz_env_add_backup(package_env, "source_path",
                      bz_interpolated_value_new("${repo_base_path}/.."));
    ep_check(repo->default_package = bz_built_package_new(package_env));
    return 0;

error:
    if (package_path != NULL) {
        cork_path_free(package_path);
    }
    if (package_env != NULL) {
        bz_env_free(package_env);
    }
    if (package != NULL) {
        bz_package_free(package);
    }
    return -1;
}

struct bz_repo *
bz_filesystem_repo_new(const char *path, struct bz_action *update)
{
    bool  exists;
    struct bz_filesystem_repo  *repo;
    struct cork_path  *repo_path = NULL;

    rpi_check(bz_file_exists(path, &exists));
    if (!exists) {
        bz_bad_config("Repository directory %s doesn't exist", path);
        return NULL;
    }

    repo = cork_new(struct bz_filesystem_repo);
    repo->default_package = NULL;
    repo->path = cork_strdup(path);
    repo->env = bz_repo_env_new_empty();
    repo->update = (update == NULL)? bz_noop_action_new(): update;
    bz_env_add_override(repo->env, "repo_base_path", bz_string_value_new(path));

    ep_check(repo_path = bz_env_get_path(repo->env, "repo_path", true));
    ei_check(bz_file_exists(cork_path_get(repo_path), &exists));
    if (exists) {
        struct bz_value_set  *repo_yaml;
        ep_check(repo_yaml = bz_yaml_value_set_new_from_file
                 (cork_path_get(repo_path), cork_path_get(repo_path)));
        bz_env_add_set(repo->env, repo_yaml);
    }

    ei_check(bz_filesystem_repo_add_git_version(repo));
    ei_check(bz_filesystem_repo_create_default_package(repo));

    cork_path_free(repo_path);
    return bz_repo_new
        (repo->env, repo, bz_filesystem__free,
         bz_filesystem__load,
         bz_filesystem__update,
         repo->default_package);

error:
    if (repo_path != NULL) {
        cork_path_free(repo_path);
    }
    bz_filesystem__free(repo);
    return NULL;
}


/*-----------------------------------------------------------------------
 * Finding a filesystem repository
 */

struct bz_repo *
bz_filesystem_repo_find(const char *path_string)
{
    struct cork_path  *path = cork_path_new(path_string);

    do {
        bool  exists;
        cork_path_append(path, ".buzzy");

        ei_check(bz_file_exists(cork_path_get(path), &exists));
        if (exists) {
            struct bz_repo  *repo;
            ep_check(repo = bz_filesystem_repo_new(cork_path_get(path), NULL));
            cork_path_free(path);
            bz_repo_register(repo);
            return repo;
        }

        /* Remove ".buzzy" from the path. */
        /* Otherwise check the parent directory next. */
        cork_path_set_dirname(path);

        /* If we just checked the root directory or current working directory,
         * then there's nothing else to check. */
        if (strcmp(cork_path_get(path), "") == 0 ||
            strcmp(cork_path_get(path), "/") == 0) {
            goto error;
        }

        /* Otherwise check the parent directory next. */
        cork_path_set_dirname(path);
    } while (true);

error:
    cork_path_free(path);
    return NULL;
}
