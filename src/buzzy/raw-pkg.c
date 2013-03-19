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
PACKAGE_SPEC_HELP_TEXT \
PACKAGER_HELP_TEXT \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_raw_pkg =
    cork_leaf_command("pkg", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
                      parse_options, execute);

#define SHORT_OPTS  "+f" \
    GENERAL_SHORT_OPTS \
    PACKAGE_SPEC_SHORT_OPTS \
    PACKAGER_SHORT_OPTS \

static struct option  opts[] = {
    { "force", no_argument, NULL, 'f' },
    GENERAL_LONG_OPTS,
    PACKAGE_SPEC_LONG_OPTS,
    PACKAGER_LONG_OPTS,
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

        if (package_spec_parse_opt(ch, &buzzy_raw_pkg)) {
            continue;
        }

        if (packager_parse_opt(ch, &buzzy_raw_pkg)) {
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
    struct bz_package_spec  *spec;
    struct cork_path  *package_path;
    struct cork_path  *staging_path;
    struct bz_action  *action;
    struct bz_action_phase  *phase;

    if (argc == 0) {
        cork_command_show_help(&buzzy_raw_pkg, "Missing staging directory.");
        exit(EXIT_FAILURE);
    } else if (argc > 1) {
        cork_command_show_help(&buzzy_raw_pkg, NULL);
        exit(EXIT_FAILURE);
    }

    ri_check_error(bz_pdb_discover());

    rp_check_error(package_path = cork_path_new("."));
    ri_check_error(cork_path_set_absolute(package_path));
    rp_check_error(staging_path = cork_path_new(argv[0]));
    ri_check_error(cork_path_set_absolute(staging_path));
    rp_check_error(spec = package_spec_get(&buzzy_raw_pkg));
    rp_check_error(action = bz_create_package
                   (spec, package_path, staging_path, NULL,
                    force, verbosity > 0));

    phase = bz_action_phase_new("Create package:");
    bz_action_phase_add(phase, action);
    ri_check_error(bz_action_phase_perform(phase));

    bz_action_phase_free(phase);
    bz_action_free(action);
    bz_package_spec_free(spec);
    exit(EXIT_SUCCESS);
}
