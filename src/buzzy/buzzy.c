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

int
main(int argc, char **argv)
{
    return cork_command_main(&buzzy_root, argc, argv);
}
