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
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/os.h"
#include "buzzy/repo.h"
#include "buzzy/distro/git.h"


/*-----------------------------------------------------------------------
 * git repository
 */

struct bz_git_repo {
    struct cork_buffer  slug;
    const char  *url;
    const char  *commit;
    struct bz_repo  *repo;
};


static int
bz_git__load(void *user_data, struct bz_env *env)
{
    struct bz_git_repo  *repo = user_data;
    struct cork_path  *repo_base_dir;
    rip_check(repo_base_dir = bz_env_get_path(env, "repo.base_dir"));
    rii_check(bz_git_clone(repo->url, repo->commit, repo_base_dir));
    return bz_filesystem_repo_load(repo->repo);
}


static int
bz_git__update(void *user_data, struct bz_env *env)
{
    struct bz_git_repo  *repo = user_data;
    struct cork_path  *repo_git_dir;
    rip_check(repo_git_dir = bz_env_get_path(env, "repo.git_dir"));
    rii_check(bz_git_update(repo->url, repo->commit, repo_git_dir));
    return 0;
}


static void
bz_git_make_slug(struct cork_buffer *dest, const char *url, const char *commit)
{
    struct cork_path  *path;
    cork_hash  hash;

    /* First grab the basename of the URL. */
    path = cork_path_new(url);
    cork_path_set_basename(path);
    cork_buffer_append_string(dest, cork_path_get(path));
    cork_path_free(path);

    /* Then remove any trailing ".git" extension. */
    if (dest->size >= 4) {
        const char  *extension = &cork_buffer_char(dest, dest->size - 4);
        if (strcmp(extension, ".git") == 0) {
            cork_buffer_truncate(dest, dest->size - 4);
        }
    }

    /* Then calculate the hash of the full URL and commit, and append that to
     * ensure we have a unique slug. */
    hash = 0x48f2a642;   /* hash of "git" */
    hash = cork_stable_hash_buffer(hash, url, strlen(url));
    hash = cork_stable_hash_buffer(hash, commit, strlen(commit));
    cork_buffer_append_printf(dest, "-%08" PRIx32, hash);
}

static void
bz_git__free(void *user_data)
{
    struct bz_git_repo  *repo = user_data;
    cork_buffer_done(&repo->slug);
    cork_strfree(repo->url);
    cork_strfree(repo->commit);
    free(repo);
}

struct bz_repo *
bz_git_repo_new(const char *url, const char *commit)
{
    struct bz_git_repo  *repo;
    struct bz_env  *repo_env;

    repo = cork_new(struct bz_git_repo);
    cork_buffer_init(&repo->slug);
    repo->url = cork_strdup(url);
    repo->commit = cork_strdup(commit);
    bz_git_make_slug(&repo->slug, url, commit);

    repo_env = bz_repo_env_new_empty();
    bz_env_add_override
        (repo_env, "repo.git.url", bz_string_value_new(url));
    bz_env_add_override
        (repo_env, "repo.git.commit", bz_string_value_new(commit));
    bz_env_add_override
        (repo_env, "repo.name",
         bz_interpolated_value_new("${repo.git.url} (${repo.git.commit})"));
    bz_env_add_override
        (repo_env, "repo.slug", bz_string_value_new(repo->slug.buf));
    bz_env_add_override
        (repo_env, "repo.base_dir",
         bz_interpolated_value_new("${repo_dir}/${repo.slug}"));

    repo->repo = bz_repo_new
        (repo_env, repo, bz_git__free,
         bz_git__load,
         bz_git__update);
    return repo->repo;
}
