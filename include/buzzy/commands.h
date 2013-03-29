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

CORK_LOCAL extern struct cork_command  buzzy_root;

CORK_LOCAL extern struct cork_command  buzzy_doc;

CORK_LOCAL extern struct cork_command  buzzy_vercmp;

CORK_LOCAL extern struct cork_command  buzzy_raw;
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
 * Common options: Packager selection
 */

CORK_ATTR_UNUSED
static const char  *packager_name = NULL;

#define PACKAGER_HELP_TEXT \
"\n" \
"Packager options:\n" \
"  --packager=<packager>\n" \
"    Choose which packager to use to create and install binary packages.\n" \
"    Normally, you will not have to provide this option, and we will use the\n" \
"    native packager for the current platform.  You can override our\n" \
"    selection with one of the following values:\n" \
"        pacman\n" \

#define PACKAGER_SHORT_OPTS  ""

#define OPT_PACKAGER  1000

#define PACKAGER_LONG_OPTS \
    { "packager", required_argument, NULL, OPT_PACKAGER }

CORK_ATTR_UNUSED
static bool
packager_parse_opt(int ch, struct cork_command *cmd)
{
    if (ch == OPT_PACKAGER) {
        if (CORK_UNLIKELY(bz_packager_choose(optarg) != 0)) {
            cork_command_show_help(cmd, cork_error_message());
            exit(EXIT_FAILURE);
        }

        return true;
    }

    return false;
}


/*-----------------------------------------------------------------------
 * Common options: Package specs
 */

static const char  *pspec_name = NULL;
static const char  *pspec_version = NULL;
static const char  *pspec_license = NULL;

#define PACKAGE_SPEC_HELP_TEXT \
"\n" \
"Package specification options:\n" \
"  --package-name=<name>\n" \
"    The name of the new package.\n" \
"  --package-version=<version>\n" \
"    The version number of the new package.\n" \
"  --package-license=<license>\n" \
"    The name of the license that the package is released under.\n" \

#define PACKAGE_SPEC_SHORT_OPTS  ""

#define OPT_PSPEC_NAME     2000
#define OPT_PSPEC_VERSION  2001
#define OPT_PSPEC_LICENSE  2002

#define PACKAGE_SPEC_LONG_OPTS \
    { "package-name", required_argument, NULL, OPT_PSPEC_NAME }, \
    { "package-version", required_argument, NULL, OPT_PSPEC_VERSION }, \
    { "package-license", required_argument, NULL, OPT_PSPEC_LICENSE }

CORK_ATTR_UNUSED
static bool
package_spec_parse_opt(int ch, struct cork_command *cmd)
{
    if (ch == OPT_PSPEC_NAME) {
        pspec_name = optarg;
        return true;
    } else if (ch == OPT_PSPEC_VERSION) {
        pspec_version = optarg;
        return true;
    } else if (ch == OPT_PSPEC_LICENSE) {
        pspec_license = optarg;
        return true;
    }

    return false;
}

CORK_ATTR_UNUSED
static struct bz_package_spec *
package_spec_get(struct cork_command *cmd)
{
    struct bz_version  *version;
    struct bz_package_spec  *spec;

    if (pspec_name == NULL) {
        cork_command_show_help(cmd, "Missing package name.");
        exit(EXIT_FAILURE);
    }

    if (pspec_version == NULL) {
        cork_command_show_help(cmd, "Missing package version.");
        exit(EXIT_FAILURE);
    }

    rp_check_error(version = bz_version_from_string(pspec_version));
    rp_check_error(spec = bz_package_spec_new(pspec_name, version));

    if (pspec_license != NULL) {
        bz_package_spec_set_license(spec, pspec_license);
    }

    return spec;
}


#endif /* BUZZY_COMMANDS_H */
