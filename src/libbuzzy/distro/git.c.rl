/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <string.h>

#include <libcork/core.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/version.h"
#include "buzzy/distro/git.h"


#if !defined(BZ_DEBUG_GIT)
#define BZ_DEBUG_GIT  0
#endif

#if BZ_DEBUG_GIT
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * git version strings
 */

struct bz_version *
bz_version_from_git_describe(const char *git_version)
{
    int  cs;
    const char  *p = git_version;
    const char  *pe = strchr(git_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    const char  *start;
    const char  *git_start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    DEBUG("---\nParse git version \"%s\"\n", git_version);
    version = bz_version_new();

    %%{
        machine git_version;

        action start_part {
            start = fpc;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }

        action start_trailer {
            start = fpc;
            DEBUG("  Looking for a trailer\n");
        }

        action add_to_part {
            cork_buffer_append(&buf, start, fpc - start);
        }

        action add_release_part {
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }

        action add_prerelease_part {
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }

        action add_postrelease_part {
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }

        skip_leading = (alpha | '-')*;

        first_release = (digit >start_part) digit* %add_to_part
                      %add_release_part;

        release = '.' (digit >start_part) digit* %add_to_part
                %add_release_part;

        tag_numeric_trailer = '-' (digit >start_trailer) digit* %add_to_part;
        tag_alpha_trailer = (digit >start_trailer) alnum* %add_to_part;
        tag_trailer = tag_numeric_trailer | tag_alpha_trailer;

        dash_digit = '-' (digit >start_part) digit* %add_to_part
                   %add_postrelease_part;

        dash_pre = '-pre' tag_trailer %add_prerelease_part;

        dash_post = '-post' tag_trailer %add_postrelease_part;

        hex = 'a'..'f' | digit;
        dash_g = '-g' (hex >{ git_start = fpc; }) hex*
               %{ cork_buffer_set(&buf, "git", 3); }
               %{ cork_buffer_append(&buf, git_start, fpc - git_start); }
               %add_postrelease_part;

        dash_tag = ((alpha >start_part) alpha*) -- ('pre' | 'post');
        dash_tagged = '-' dash_tag %add_to_part tag_trailer?
                    %add_postrelease_part;

        dash = dash_digit | dash_pre | dash_post | dash_g | dash_tagged;

        ddash_digit = '--' (digit >start_part) digit* %add_to_part
                    %add_prerelease_part;

        ddash_pre = '--pre' tag_trailer %add_prerelease_part;

        ddash_tag = ((alpha >start_part) alpha*) -- ('pre');
        ddash_tagged = '--' ddash_tag %add_to_part tag_trailer?
                     %add_prerelease_part;

        ddash = ddash_digit | ddash_pre | ddash_tagged;

        part = release | dash | ddash;

        main := skip_leading first_release part**;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) git_version_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Invalid git version \"%s\"", git_version);
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    cork_buffer_done(&buf);
    return version;
}


/*-----------------------------------------------------------------------
 * git version values
 */

struct bz_git_version {
    struct bz_version  *version;
};

static void
bz_git_version__free(void *user_data)
{
    struct bz_git_version  *git = user_data;
    if (git->version != NULL) {
        bz_version_free(git->version);
    }
    free(git);
}

static const char *
bz_git_version__provide(void *user_data, struct bz_env *env)
{
    struct bz_git_version  *git = user_data;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct cork_buffer  dirty = CORK_BUFFER_INIT();

    if (git->version == NULL) {
        bool  successful;
        struct cork_path  *source_dir;
        struct cork_exec  *exec;

        /* Grab the base version string from "git describe" */
        ep_check(source_dir = bz_env_get_path(env, "source_dir", true));
        exec = cork_exec_new_with_params("git", "describe", NULL);
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        cork_path_free(source_dir);
        ei_check(bz_subprocess_get_output_exec(&out, NULL, &successful, exec));
        if (!successful) {
            goto error;
        }

        /* Chomp the trailing newline */
        ((char *) out.buf)[--out.size] = '\0';

        /* If the working tree is dirty, append "+dirty" to the version. */
        ep_check(source_dir = bz_env_get_path(env, "source_dir", true));
        exec = cork_exec_new_with_params("git", "status", "--porcelain", NULL);
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        cork_path_free(source_dir);
        ei_check(bz_subprocess_get_output_exec
                 (&dirty, NULL, &successful, exec));
        if (!successful) {
            goto error;
        }
        if (dirty.size > 0) {
            cork_buffer_append_string(&out, "-dirty");
        }

        ep_check(git->version = bz_version_from_git_describe(out.buf));
    }

    cork_buffer_done(&out);
    cork_buffer_done(&dirty);
    return bz_version_to_string(git->version);

error:
    cork_buffer_done(&out);
    cork_buffer_done(&dirty);
    return NULL;
}

struct bz_value_provider *
bz_git_version_value_new(void)
{
    struct bz_git_version  *git = cork_new(struct bz_git_version);
    git->version = NULL;
    return bz_value_provider_new
        (git, bz_git_version__free, bz_git_version__provide);
}


/*-----------------------------------------------------------------------
 * git repository actions
 */

struct bz_git {
    const char  *url;
    const char  *commit;
    struct cork_path  *dest_dir;
    struct cork_buffer  remote_commit;
};

struct bz_git *
bz_git_new(const char *url, const char *commit, struct cork_path *dest_dir)
{
    struct bz_git  *git = cork_new(struct bz_git);
    git->url = cork_strdup(url);
    git->commit = cork_strdup(commit);
    git->dest_dir = dest_dir;
    cork_buffer_init(&git->remote_commit);
    cork_buffer_printf(&git->remote_commit, "origin/%s", commit);
    return git;
}

static void
bz_git__free(void *user_data)
{
    struct bz_git  *git = user_data;
    cork_strfree(git->url);
    cork_strfree(git->commit);
    cork_path_free(git->dest_dir);
    cork_buffer_done(&git->remote_commit);
    free(git);
}


static int
bz_git__clone__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_git  *git = user_data;
    cork_buffer_append_printf(dest, "Clone %s (%s)", git->url, git->commit);
    return 0;
}

static int
bz_git__clone__is_needed(void *user_data, bool *is_needed)
{
    struct bz_git  *git = user_data;
    bool  exists;
    rii_check(bz_file_exists(cork_path_get(git->dest_dir), &exists));
    *is_needed = !exists;
    return 0;
}

static int
bz_git__clone__perform(void *user_data)
{
    struct bz_git  *git = user_data;
    struct cork_path  *parent;

    /* Create the parent directory of our clone. */
    parent = cork_path_dirname(git->dest_dir);
    ei_check(bz_create_directory(cork_path_get(parent)));
    cork_path_free(parent);

    return bz_subprocess_run
        (false, NULL,
         "git", "clone", "--recursive",
         "--branch", git->commit,
         git->url,
         cork_path_get(git->dest_dir),
         NULL);

error:
    cork_path_free(parent);
    return -1;
}

struct bz_action *
bz_git_clone_new(const char *url, const char *commit,
                 struct cork_path *dest_dir)
{
    struct bz_git  *git = bz_git_new(url, commit, dest_dir);
    return bz_action_new
        (git, bz_git__free,
         bz_git__clone__message,
         bz_git__clone__is_needed,
         bz_git__clone__perform);
}


static int
bz_git__update__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_git  *git = user_data;
    cork_buffer_append_printf(dest, "Update %s (%s)", git->url, git->commit);
    return 0;
}

static int
bz_git__update__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_git__update__perform(void *user_data)
{
    struct bz_git  *git = user_data;
    bool  exists;

    /* If the destination directory doesn't exist yet, perform a clone instead
     * of an update. */
    rii_check(bz_file_exists(cork_path_get(git->dest_dir), &exists));
    if (!exists) {
        return bz_git__clone__perform(git);
    }

    /* Otherwise perform a fetch + reset.  Assume that we've already checked out
     * the right branch. */

    rii_check(bz_subprocess_run
              (false, NULL,
               "git", "--git-dir", cork_path_get(git->dest_dir),
               "fetch", "origin",
               NULL));
    rii_check(bz_subprocess_run
              (false, NULL,
               "git", "--git-dir", cork_path_get(git->dest_dir),
               "reset", "--hard", git->remote_commit.buf,
               NULL));
    return 0;
}

struct bz_action *
bz_git_update_new(const char *url, const char *commit,
                 struct cork_path *dest_dir)
{
    struct bz_git  *git = bz_git_new(url, commit, dest_dir);
    return bz_action_new
        (git, bz_git__free,
         bz_git__update__message,
         bz_git__update__is_needed,
         bz_git__update__perform);
}
