/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
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

#define SHORT_DESC \
    "Compare two Buzzy version strings"

#define USAGE_SUFFIX \
    "<version 1> <version 2>"

#define HELP_TEXT \
"Compares two Buzzy version strings.  Prints out \"-1\", \"0\", or \"1\",\n" \
"depending on how the two version strings compare to each other.\n" \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_vercmp =
    cork_leaf_command("vercmp", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
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
                cork_command_show_help(&buzzy_vercmp, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    struct bz_version  *v1;
    struct bz_version  *v2;
    int  cmp;

    if (argc != 2) {
        fprintf(stderr, "Must provide two version strings.\n");
        cork_command_show_help(&buzzy_vercmp, NULL);
        exit(EXIT_FAILURE);
    }

    rp_check_error(v1 = bz_version_from_string(argv[0]));
    rp_check_error(v2 = bz_version_from_string(argv[1]));
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
