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

#include "buzzy/built.h"
#include "buzzy/commands.h"
#include "buzzy/version.h"

/*-----------------------------------------------------------------------
 * buzzy raw build
 */

#define SHORT_DESC \
    "Build a package from a source directory"

#define USAGE_SUFFIX \
    "[<options>]"

#define HELP_TEXT \
"Builds a package whose source code is in the current directory.\n" \
"\n" \
"Options:\n" \
"  -f, --force\n" \
"    Rebuild the package even if a previous build is up-to-date.\n" \
GENERAL_HELP_TEXT \
PACKAGE_ENV_HELP_TEXT \

static int
parse_options(int argc, char **argv);

static void
execute(int argc, char **argv);

CORK_LOCAL struct cork_command  buzzy_raw_build =
    cork_leaf_command("build", SHORT_DESC, USAGE_SUFFIX, HELP_TEXT,
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
        if (general_parse_opt(ch, &buzzy_raw_build)) {
            continue;
        }

        if (package_env_parse_opt(ch, &buzzy_raw_build)) {
            continue;
        }

        switch (ch) {
            case 'f':
                force = true;
                break;

            default:
                cork_command_show_help(&buzzy_raw_build, NULL);
                exit(EXIT_FAILURE);
        }

    }
    return optind;
}

static void
execute(int argc, char **argv)
{
    struct cork_path  *source_path;
    struct bz_builder  *builder;
    struct bz_action  *action;
    struct bz_action_phase  *phase;
    struct bz_value_provider  *value;

    if (argc > 0) {
        cork_command_show_help(&buzzy_raw_build, NULL);
        exit(EXIT_FAILURE);
    }

    ri_check_error(bz_load_variable_definitions());
    ri_check_error(bz_pdb_discover());
    package_env_init();

    rp_check_error(source_path = cork_path_new("."));
    ri_check_error(cork_path_set_absolute(source_path));
    rp_check_error(value = bz_path_value_new(source_path));
    bz_env_add_override(package_env, "source_path", value);

    rp_check_error(value = bz_string_value_new(force? "1": "0"));
    bz_env_add_override(package_env, "force", value);

    rp_check_error(value = bz_string_value_new((verbosity > -1)? "1": "0"));
    bz_env_add_override(package_env, "verbose", value);

    rp_check_error(builder = bz_package_builder_new(package_env));
    rp_check_error(action = bz_builder_build_action(builder));

    phase = bz_action_phase_new("Build package:");
    bz_action_phase_add(phase, action);
    ri_check_error(bz_action_phase_perform(phase, 0));

    bz_action_phase_free(phase);
    bz_builder_free(builder);
    package_env_done();
    exit(EXIT_SUCCESS);
}
