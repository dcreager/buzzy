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
#include "buzzy/logging.h"
#include "buzzy/repo.h"

/*-----------------------------------------------------------------------
 * buzzy uninstall
 */

#define SHORT_DESC \
    "Uninstall a package"

#define USAGE_SUFFIX \
    "<dependency>..."

#define HELP_TEXT \
"Uninstalls the given packages.\n" \
GENERAL_HELP_TEXT \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_uninstall =
    cork_leaf_command("uninstall", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
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
                cork_command_show_help(&buzzy_uninstall, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    size_t  i;

    bz_load_repositories();
    satisfy_dependencies(&buzzy_uninstall, argc, argv);

    for (i = 0; i < cork_array_size(&dep_packages); i++) {
        struct bz_package  *package = cork_array_at(&dep_packages, i);
        ri_check_error(bz_package_uninstall(package));
    }

    free_dependencies();
    bz_finalize_actions();
    exit(EXIT_SUCCESS);
}
