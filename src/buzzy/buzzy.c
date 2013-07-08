/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/cli.h>

#include "buzzy/commands.h"

int
main(int argc, char **argv)
{
    clog_set_default_format("[%L] %m");
    ri_check_error(clog_setup_logging());
    return cork_command_main(&buzzy_root, argc, argv);
}
