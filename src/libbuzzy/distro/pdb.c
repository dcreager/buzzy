/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/distro.h"
#include "buzzy/package.h"

#include "buzzy/distro/arch.h"


static int
bz_detect_arch(void)
{
    bool  is_arch;
    rii_check(bz_arch_is_present(&is_arch));
    if (is_arch) {
        struct bz_pdb  *pdb = bz_arch_native_pdb();
        bz_pdb_register(pdb);
    }
    return 0;
}


int
bz_pdb_discover(void)
{
    rii_check(bz_detect_arch());
    return 0;
}
