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

CORK_LOCAL extern struct cork_command  buzzy_root;
CORK_LOCAL extern struct cork_command  buzzy_vercmp;

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


#endif /* BUZZY_COMMANDS_H */
