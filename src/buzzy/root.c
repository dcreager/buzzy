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
    &buzzy_doc,
    &buzzy_info,
    &buzzy_raw,
    &buzzy_vercmp,
    NULL
};

CORK_LOCAL struct cork_command  buzzy_root =
    cork_command_set("buzzy", NULL, NULL, subcommands);
