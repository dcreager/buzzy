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
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/repo.h"
#include "buzzy/distro/git.h"

#define CLOG_CHANNEL  "repo:filesystem"


/*-----------------------------------------------------------------------
 * Filesystem repository
 */

static int
bz_filesystem_repo__load_repo_yaml(struct bz_repo *repo)
{
    bool  exists;
    struct bz_env  *repo_env = bz_repo_env(repo);
    struct cork_path  *repo_yaml_file;

    rip_check(repo_yaml_file = bz_env_get_path
              (repo_env, "repo.repo_yaml", true));
    rii_check(bz_file_exists(cork_path_get(repo_yaml_file), &exists));
    if (exists) {
        const char  *repo_yaml_file_string = cork_path_get(repo_yaml_file);
        struct bz_value  *repo_yaml;
        clog_info("Load repo.yaml file");
        rip_check(repo_yaml = bz_yaml_value_new_from_file
                  (repo_yaml_file_string));
        bz_env_add_set(repo_env, repo_yaml);
    }

    return 0;
}

static int
bz_filesystem_repo__load_links_yaml(struct bz_repo *repo)
{
    bool  exists;
    struct bz_env  *repo_env = bz_repo_env(repo);
    struct cork_path  *links_yaml_file;

    rip_check(links_yaml_file =
              bz_env_get_path(repo_env, "repo.links_yaml", true));
    rii_check(bz_file_exists(cork_path_get(links_yaml_file), &exists));
    if (exists) {
        const char  *links_yaml_file_string = cork_path_get(links_yaml_file);
        clog_info("Load links.yaml file");
        rii_check(bz_repo_parse_yaml_links(repo, links_yaml_file_string));
    }

    return 0;
}

static int
bz_filesystem_repo__add_git_version(struct bz_repo *repo)
{
    bool  exists;
    struct bz_env  *repo_env = bz_repo_env(repo);
    struct cork_path  *git_dir = NULL;

    /* See if the repository is a git checkout. */
    rip_check(git_dir = bz_env_get_path(repo_env, "repo.git_dir", true));
    rii_check(bz_file_exists(cork_path_get(git_dir), &exists));
    if (!exists) {
        return 0;
    }

    /* If so, add a default value for the "version" variable that parses the
     * result of "git describe". */
    bz_env_add_backup(repo_env, "version", bz_git_version_value_new());
    return 0;
}

static int
bz_filesystem_repo__create_default_package(struct bz_repo *repo)
{
    bool  exists;
    struct bz_env  *repo_env = bz_repo_env(repo);
    struct cork_path  *package_file = NULL;
    struct bz_env  *package_env = NULL;
    struct bz_value  *package_yaml;
    struct bz_package  *package = NULL;
    struct bz_pdb  *pdb = NULL;

    /* See if the repository has a package.yaml file. */
    rip_check(package_file = bz_env_get_path
              (repo_env, "repo.package_yaml", true));
    ei_check(bz_file_exists(cork_path_get(package_file), &exists));
    if (!exists) {
        return 0;
    }

    clog_info("Load package.yaml file");

    /* If so, create a default package from it. */
    ep_check(package_env = bz_package_env_new_empty(repo_env, "package"));
    ep_check(package_yaml = bz_yaml_value_new_from_file
             (cork_path_get(package_file)));
    bz_env_add_set(package_env, package_yaml);
    bz_env_add_backup(package_env, "source_dir",
                      bz_interpolated_value_new("${repo.base_dir}"));
    ep_check(package = bz_built_package_new(package_env));
    bz_repo_set_default_package(repo, package);

    /* And a single-package database. */
    ep_check(pdb = bz_single_package_pdb_new(bz_repo_name(repo), package));
    bz_pdb_register(pdb);
    return 0;

error:
    if (package_env != NULL) {
        bz_env_free(package_env);
    }
    if (package != NULL) {
        bz_package_free(package);
    }
    if (pdb != NULL) {
        bz_pdb_free(pdb);
    }
    return -1;
}

int
bz_filesystem_repo_load(struct bz_repo *repo)
{
    rii_check(bz_filesystem_repo__load_repo_yaml(repo));
    rii_check(bz_filesystem_repo__load_links_yaml(repo));
    rii_check(bz_filesystem_repo__add_git_version(repo));
    rii_check(bz_filesystem_repo__create_default_package(repo));
    return 0;
}
