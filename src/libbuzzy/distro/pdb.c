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
#include "buzzy/distro/homebrew.h"
#include "buzzy/distro/rpm.h"


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


static int
bz_detect_homebrew(void)
{
    bool  is_homebrew;
    rii_check(bz_homebrew_is_present(&is_homebrew));
    if (is_homebrew) {
        struct bz_pdb  *pdb = bz_homebrew_native_pdb();
        bz_pdb_register(pdb);
    }
    return 0;
}


static int
bz_detect_redhat(void)
{
    bool  is_redhat;
    rii_check(bz_redhat_is_present(&is_redhat));
    if (is_redhat) {
        struct bz_pdb  *pdb = bz_yum_native_pdb();
        bz_pdb_register(pdb);
    }
    return 0;
}


int
bz_pdb_discover(void)
{
    rii_check(bz_detect_arch());
    rii_check(bz_detect_homebrew());
    rii_check(bz_detect_redhat());
    return 0;
}
