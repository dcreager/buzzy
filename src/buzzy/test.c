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
 * buzzy test
 */

#define SHORT_DESC \
    "test a package"

#define USAGE_SUFFIX \
    "<dependency>..."

#define HELP_TEXT \
"Finds a set of packages that satisfies all of the dependencies that you\n" \
"provide, and then builds and tests them all.  We don't install the packages\n" \
"in question; we only build and test them.\n" \
GENERAL_HELP_TEXT \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_test =
    cork_leaf_command("test", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
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
                cork_command_show_help(&buzzy_test, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    size_t  i;
    struct bz_action_phase  *phase;

    bz_load_repositories();
    satisfy_dependencies(&buzzy_test, argc, argv);

    phase = bz_action_phase_new("Test packages:");
    for (i = 0; i < cork_array_size(&dep_packages); i++) {
        struct bz_package  *package = cork_array_at(&dep_packages, i);
        struct bz_action  *action;
        rp_check_error(action = bz_package_test_action(package));
        bz_action_phase_add(phase, action);
    }

    ri_check_error(bz_action_phase_perform(phase, 0));
    bz_action_phase_free(phase);
    free_dependencies();
    exit(EXIT_SUCCESS);
}
