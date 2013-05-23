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
#include "buzzy/env.h"

/*-----------------------------------------------------------------------
 * buzzy doc
 */

#define SHORT_DESC \
    "Describe a Buzzy configuration variable"

#define USAGE_SUFFIX \
    "<variable name>"

#define HELP_TEXT \
"Prints out documentation about a Buzzy configuration variable.\n" \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_doc =
    cork_leaf_command("doc", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
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
                cork_command_show_help(&buzzy_doc, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    struct bz_env  *env;
    struct bz_var_doc  *doc;
    struct bz_value  *value;

    if (argc != 1) {
        cork_command_show_help(&buzzy_doc, "Must provide a variable name.");
        exit(EXIT_FAILURE);
    }

    bz_load_repositories();
    if (base_repo == NULL) {
        env = bz_global_env();
    } else {
        struct bz_package  *package = bz_repo_default_package(base_repo);
        if (package == NULL) {
            env = bz_repo_env(base_repo);
        } else {
            env = bz_package_env(package);
        }
    }

    rp_check_error(doc = bz_env_get_global_default(argv[0]));
    printf("%s\n", doc->name);

    if (doc->short_desc[0] != '\0') {
        printf("  %s\n", doc->short_desc);
    }
    if (doc->long_desc[0] != '\0') {
        /* TODO: Word wrap this */
        printf("\n  %s\n", doc->long_desc);
    }

    re_check_error(value = bz_env_get_value(env, argv[0]));
    if (value == NULL) {
        printf("\n  No current value\n");
    } else {
        if (bz_value_kind(value) == BZ_VALUE_SCALAR) {
            const char  *content;
            re_check_error(content = bz_scalar_value_get
                           (value, bz_env_as_value(env)));
            printf("\n  Current value: %s\n", content);
        } else {
            printf("\n  Current value: [%s]\n",
                   bz_value_kind_string(bz_value_kind(value)));
        }
    }

    exit(EXIT_SUCCESS);
}
