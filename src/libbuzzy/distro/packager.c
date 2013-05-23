/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/distro.h"

#include "buzzy/distro/arch.h"


static const char *
bz_packager__detect(void *user_data, struct bz_value *ctx)
{
    bool  is_arch;

    rpi_check(bz_arch_is_present(&is_arch));
    if (is_arch) {
        return "pacman";
    }

    bz_bad_config("Don't know what packager to use on this platform");
    return NULL;
}

struct bz_value *
bz_packager_detector_new(void)
{
    return bz_scalar_value_new(NULL, NULL, bz_packager__detect);
}
