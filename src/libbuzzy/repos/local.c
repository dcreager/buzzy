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
#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/os.h"
#include "buzzy/repo.h"

#define CLOG_CHANNEL  "repo:local"


/*-----------------------------------------------------------------------
 * Local filesystem repository
 */

struct bz_local_filesystem_repo {
    const char  *path;
    struct bz_repo  *repo;
};

static void
bz_local_filesystem__free(void *user_data)
{
    struct bz_local_filesystem_repo  *repo = user_data;
    cork_strfree(repo->path);
    free(repo);
}

static int
bz_local_filesystem__load(void *user_data, struct bz_env *env)
{
    struct bz_local_filesystem_repo  *repo = user_data;
    return bz_filesystem_repo_load(repo->repo);
}

static int
bz_local_filesystem__update(void *user_data, struct bz_env *env)
{
    return 0;
}

struct bz_repo *
bz_local_filesystem_repo_new(const char *path)
{
    bool  exists;
    struct bz_local_filesystem_repo  *repo;
    struct bz_env  *repo_env;

    rpi_check(bz_file_exists(path, &exists));
    if (!exists) {
        bz_bad_config("Repository directory %s doesn't exist", path);
        return NULL;
    }

    repo = cork_new(struct bz_local_filesystem_repo);
    repo->path = cork_strdup(path);
    repo_env = bz_repo_env_new_empty();
    bz_env_set_base_path(repo_env, path);
    bz_env_add_override(repo_env, "repo.name", bz_string_value_new(path));
    bz_env_add_override(repo_env, "repo.base_dir", bz_string_value_new(path));

    repo->repo = bz_repo_new
        (repo_env, repo, bz_local_filesystem__free,
         bz_local_filesystem__load,
         bz_local_filesystem__update);
    return repo->repo;
}


/*-----------------------------------------------------------------------
 * Finding a filesystem repository
 */

struct bz_repo *
bz_local_filesystem_repo_find(const char *path_string)
{
    struct cork_path  *path = cork_path_new(path_string);

    clog_info("Look for local repository near current directory");

    do {
        bool  exists;
        cork_path_append(path, ".buzzy");
        ei_check(bz_file_exists(cork_path_get(path), &exists));

        /* Remove ".buzzy" from the path. */
        cork_path_set_dirname(path);
        if (exists) {
            struct bz_repo  *repo;
            clog_info("Found local repository at %s", cork_path_get(path));
            ep_check(repo = bz_local_filesystem_repo_new(cork_path_get(path)));
            cork_path_free(path);
            bz_repo_register(repo);
            return repo;
        }

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
