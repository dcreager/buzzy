/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <getopt.h>
#include <unistd.h>

#include <libcork/cli.h>
#include <libcork/core.h>
#include <libcork/helpers/errors.h>
#include <libcork/helpers/posix.h>

#include "buzzy/action.h"
#include "buzzy/commands.h"
#include "buzzy/env.h"
#include "buzzy/repo.h"

/*-----------------------------------------------------------------------
 * buzzy info
 */

#define SHORT_DESC \
    "Print information about the current Buzzy repository"

#define USAGE_SUFFIX \
    ""

#define HELP_TEXT \
"Prints out information about the current Buzzy repository, and any\n" \
"packages that it defines.\n" \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_info =
    cork_leaf_command("info", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
                      parse_options, execute);

#define SHORT_OPTS  "+"

static struct option  opts[] = {
    { NULL, 0, NULL, 0 }
};

static int
parse_options(int argc, char **argv)
{
    int  ch;
    getopt_reset();
    while ((ch = getopt_long(argc, argv, SHORT_OPTS, opts, NULL)) != -1) {
        switch (ch) {
            default:
                cork_command_show_help(&buzzy_info, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    size_t  i;
    size_t  repo_count;
    struct bz_env  *env;
    struct cork_path  *repo_path;

    if (argc != 0) {
        cork_command_show_help(&buzzy_info, NULL);
        exit(EXIT_FAILURE);
    }

    bz_load_repositories();

    repo_count = bz_repo_registry_count();
    if (repo_count == 0) {
        printf("No repositories found!\n");
        exit(EXIT_SUCCESS);
    }

    printf("Repositories:\n");
    for (i = 0; i < repo_count; i++) {
        struct bz_repo  *repo = bz_repo_registry_get(i);
        rp_check_error(env = bz_repo_env(repo));
        rp_check_error(repo_path = bz_env_get_path
                       (env, "repo_base_path", true));
        printf("  %s\n", cork_path_get(repo_path));
        cork_path_free(repo_path);
    }

    exit(EXIT_SUCCESS);
}
