/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#include <getopt.h>
#include <string.h>

#include <libcork/cli.h>
#include <libcork/core.h>

#include "buzzy/commands.h"
#include "buzzy/version.h"

/*-----------------------------------------------------------------------
 * buzzy vercmp
 */

#define VERCMP_SHORT_DESC \
    "Compare two Buzzy version strings"

#define VERCMP_USAGE_SUFFIX \
    "<version 1> <version 2>"

#define VERCMP_HELP_TEXT \
"Compares two Buzzy version strings.  Prints out \"-1\", \"0\", or \"1\",\n" \
"depending on how the two version strings compare to each other.\n" \

static int
buzzy_vercmp__options(int argc, char **argv);

static void
buzzy_vercmp__execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_vercmp =
    cork_leaf_command("vercmp",
                      VERCMP_SHORT_DESC,
                      VERCMP_USAGE_SUFFIX,
                      VERCMP_HELP_TEXT,
                      buzzy_vercmp__options, buzzy_vercmp__execute);

#define VERCMP_SHORT_OPTS  "+"

static struct option  vercmp_opts[] = {
    { NULL, 0, NULL, 0 }
};

static int
buzzy_vercmp__options(int argc, char **argv)
{
    int  ch;
    getopt_reset();
    while ((ch = getopt_long
            (argc, argv, VERCMP_SHORT_OPTS, vercmp_opts, NULL)) != -1) {
        switch (ch) {
            default:
                cork_command_show_help(&buzzy_vercmp, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
buzzy_vercmp__execute(int argc, char **argv)
{
    struct bz_version  *v1;
    struct bz_version  *v2;
    int  cmp;

    if (argc != 2) {
        fprintf(stderr, "Must provide two version strings.\n");
        cork_command_show_help(&buzzy_vercmp, NULL);
        exit(EXIT_FAILURE);
    }

    rp_check_error(v1 = bz_version_new_from_string(argv[0]));
    rp_check_error(v2 = bz_version_new_from_string(argv[1]));
    cmp = bz_version_cmp(v1, v2);
    bz_version_free(v1);
    bz_version_free(v2);

    if (cmp < 0) {
        printf("-1\n");
    } else if (cmp > 0) {
        printf("1\n");
    } else {
        printf("0\n");
    }
    exit(EXIT_SUCCESS);
}
