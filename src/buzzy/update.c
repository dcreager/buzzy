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

#include "buzzy/commands.h"
#include "buzzy/env.h"
#include "buzzy/repo.h"

/*-----------------------------------------------------------------------
 * buzzy update
 */

#define SHORT_DESC \
    "Update the current repository and its dependencies"

#define USAGE_SUFFIX \
    ""

#define HELP_TEXT \
"If the current repository, or any of the repositories that it depends on,\n" \
"come from a remote source, we will contact the remote source to make sure\n" \
"that our local copies are up-to-date.\n" \
GENERAL_HELP_TEXT \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_update =
    cork_leaf_command("update", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
                      parse_options, execute);

#define SHORT_OPTS  "+" \
    GENERAL_SHORT_OPTS \

static struct option  opts[] = {
    GENERAL_LONG_OPTS,
    { NULL, 0, NULL, 0 }
};

static int
parse_options(int argc, char **argv)
{
    int  ch;
    getopt_reset();
    while ((ch = getopt_long(argc, argv, SHORT_OPTS, opts, NULL)) != -1) {
        if (general_parse_opt(ch, &buzzy_raw_pkg)) {
            continue;
        }

        switch (ch) {
            default:
                cork_command_show_help(&buzzy_update, NULL);
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

    bz_load_repositories();
    repo_count = bz_repo_registry_count();
    if (repo_count == 0) {
        printf("No repositories found!\n");
        exit(EXIT_SUCCESS);
    }

    for (i = 0; i < repo_count; i++) {
        struct bz_repo  *repo = bz_repo_registry_get(i);
        ri_check_error(bz_repo_update(repo));
    }

    exit(EXIT_SUCCESS);
}
