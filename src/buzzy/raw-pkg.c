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
#include <libcork/os.h>

#include "buzzy/commands.h"
#include "buzzy/logging.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * buzzy raw_pkg
 */

#define SHORT_DESC \
    "Create a binary package from a staging directory"

#define USAGE_SUFFIX \
    "[<options>] <staging directory>"

#define HELP_TEXT \
"Creates a new binary package from an existing staging directory.  The\n" \
"staging directory should contain the full installation prefix for the\n" \
"package that you want to create.\n" \
"\n" \
"Options:\n" \
"  -f, --force\n" \
"    Rebuild the binary package even if it already exists.\n" \
GENERAL_HELP_TEXT \
PACKAGE_ENV_HELP_TEXT \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_raw_pkg =
    cork_leaf_command("pkg", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
                      parse_options, execute);

#define SHORT_OPTS  "+f" \
    GENERAL_SHORT_OPTS \
    PACKAGE_ENV_SHORT_OPTS \

static struct option  opts[] = {
    { "force", no_argument, NULL, 'f' },
    GENERAL_LONG_OPTS,
    PACKAGE_ENV_LONG_OPTS,
    { NULL, 0, NULL, 0 }
};

static bool  force = false;

static int
parse_options(int argc, char **argv)
{
    int  ch;
    getopt_reset();
    while ((ch = getopt_long(argc, argv, SHORT_OPTS, opts, NULL)) != -1) {
        if (general_parse_opt(ch, &buzzy_raw_pkg)) {
            continue;
        }

        if (package_env_parse_opt(ch, &buzzy_raw_pkg)) {
            continue;
        }

        switch (ch) {
            case 'f':
                force = true;
                break;

            default:
                cork_command_show_help(&buzzy_raw_pkg, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    struct cork_path  *binary_package_dir;
    struct cork_path  *staging_dir;
    struct bz_packager  *packager;
    struct bz_value  *value;

    if (argc == 0) {
        cork_command_show_help(&buzzy_raw_pkg, "Missing staging directory.");
        exit(EXIT_FAILURE);
    } else if (argc > 1) {
        cork_command_show_help(&buzzy_raw_pkg, NULL);
        exit(EXIT_FAILURE);
    }

    ri_check_error(bz_load_variable_definitions());
    ri_check_error(bz_pdb_discover());
    package_env_init();

    rp_check_error(binary_package_dir = cork_path_new("."));
    ri_check_error(cork_path_set_absolute(binary_package_dir));
    rp_check_error(value = bz_path_value_new(binary_package_dir));
    bz_env_add_override(package_env, "binary_package_dir", value);

    rp_check_error(staging_dir = cork_path_new(argv[0]));
    ri_check_error(cork_path_set_absolute(staging_dir));
    rp_check_error(value = bz_path_value_new(staging_dir));
    bz_env_add_override(package_env, "staging_dir", value);

    rp_check_error(value = bz_string_value_new(force? "1": "0"));
    bz_env_add_override(package_env, "force", value);

    rp_check_error(value = bz_string_value_new((verbosity > 0)? "1": "0"));
    bz_env_add_override(package_env, "verbose", value);

    rp_check_error(packager = bz_package_packager_new(package_env));
    ri_check_error(bz_packager_package(packager));

    bz_packager_free(packager);
    package_env_done();
    bz_finalize_actions();
    exit(EXIT_SUCCESS);
}
