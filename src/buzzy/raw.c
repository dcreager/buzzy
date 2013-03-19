/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/cli.h>

#include "buzzy/commands.h"

static struct cork_command  *subcommands[] = {
    &buzzy_raw_pkg,
    NULL
};

#define SHORT_DESC \
    "Low-level functions for working with packages"

CORK_LOCAL struct cork_command  buzzy_raw =
    cork_command_set("raw", SHORT_DESC, NULL, subcommands);
