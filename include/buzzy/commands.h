/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_COMMANDS_H
#define BUZZY_COMMANDS_H

#include <getopt.h>
#include <stdlib.h>

#include <libcork/core.h>
#include <libcork/cli.h>
#include <libcork/ds.h>

#include "buzzy/distro.h"
#include "buzzy/env.h"

CORK_LOCAL extern struct cork_command  buzzy_root;

CORK_LOCAL extern struct cork_command  buzzy_doc;

CORK_LOCAL extern struct cork_command  buzzy_vercmp;

CORK_LOCAL extern struct cork_command  buzzy_raw;
CORK_LOCAL extern struct cork_command  buzzy_raw_build;
CORK_LOCAL extern struct cork_command  buzzy_raw_pkg;

#define ri_check_error(call) \
    do { \
        CORK_ATTR_UNUSED int  __rc = call; \
        if (CORK_UNLIKELY(cork_error_occurred())) { \
            fprintf(stderr, "%s\n", cork_error_message()); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#define rp_check_error(call) \
    do { \
        CORK_ATTR_UNUSED void  *__result = call; \
        if (CORK_UNLIKELY(cork_error_occurred())) { \
            fprintf(stderr, "%s\n", cork_error_message()); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)


/*-----------------------------------------------------------------------
 * Resetting getopt
 */

#if defined(__linux)

/* For glibc getopt, you reset just by setting optind to 1. */
CORK_ATTR_UNUSED
static void
getopt_reset(void)
{
    optind = 1;
}

#elif defined(__APPLE__) && defined(__MACH__)

/* For BSD getopt, you reset by setting optreset *and* optind. */
CORK_ATTR_UNUSED
static void
getopt_reset(void)
{
    optreset = 1;
    optind = 1;
}

#else
#error "I don't know what getopt implementation you have"

#endif  /* platforms */


/*-----------------------------------------------------------------------
 * Common options: general selection
 */

CORK_ATTR_UNUSED
static int  verbosity = 0;

#define GENERAL_HELP_TEXT \
"\n" \
"General options:\n" \
"  -q, --quiet\n" \
"    Print out less information about the steps being performed.  Providing\n" \
"    this option multiple times decreases the verbosity further.\n" \
"  -v, --verbose\n" \
"    Print out more information about the steps being performed.  Providing\n" \
"    this option multiple times increases the verbosity further.\n" \

#define GENERAL_SHORT_OPTS  "qv"

#define GENERAL_LONG_OPTS \
    { "quiet", no_argument, NULL, 'q' }, \
    { "verbose", no_argument, NULL, 'v' }

CORK_ATTR_UNUSED
static bool
general_parse_opt(int ch, struct cork_command *cmd)
{
    if (ch == 'q') {
        verbosity--;
        return true;
    } else if (ch == 'v') {
        verbosity++;
        return true;
    }

    return false;
}


/*-----------------------------------------------------------------------
 * Common options: Package environments
 */

static struct bz_env  *package_env = NULL;

#define PACKAGE_ENV_HELP_TEXT \
"\n" \
"Package configuration options:\n" \
"  -P <name>=<value>, --package-var <name>=<value>\n" \
"    Override the value of a package-specific configuration option.\n" \

#define PACKAGE_ENV_SHORT_OPTS  "P:"

#define PACKAGE_ENV_LONG_OPTS \
    { "package-var", required_argument, NULL, 'P' } \

CORK_ATTR_UNUSED
static void
package_env_init(void)
{
    if (package_env == NULL) {
        package_env = bz_package_env_new_empty("new package");
    }
}

CORK_ATTR_UNUSED
static void
package_env_done(void)
{
    bz_env_free(package_env);
    package_env = NULL;
}

CORK_ATTR_UNUSED
static bool
package_env_parse_opt(int ch, struct cork_command *cmd)
{
    package_env_init();

    if (ch == 'P') {
        struct cork_buffer  buf = CORK_BUFFER_INIT();
        const char  *equals = strchr(optarg, '=');
        struct bz_value_provider  *value;
        if (equals == NULL) {
            cork_command_show_help(cmd, "Missing variable value.");
            exit(EXIT_FAILURE);
        }
        cork_buffer_set(&buf, optarg, equals - optarg);
        rp_check_error(value = bz_interpolated_value_new(equals + 1));
        bz_env_add_override(package_env, buf.buf, value);
        cork_buffer_done(&buf);
        return true;
    }

    return false;
}


#endif /* BUZZY_COMMANDS_H */
