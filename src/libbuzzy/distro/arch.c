/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libcork/core.h>

#include "buzzy/distro/arch.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_arch_is_present(bool *dest)
{
    int  rc;
    struct stat  info;
    rc = stat("/etc/arch-release", &info);
    if (rc == 0) {
        *dest = true;
        return 0;
    } else if (errno == ENOENT) {
        *dest = false;
        return 0;
    } else {
        cork_system_error_set();
        return -1;
    }
}
