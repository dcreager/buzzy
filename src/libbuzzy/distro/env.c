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
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/distro.h"
#include "buzzy/distro/posix.h"

#include "buzzy/distro/arch.h"
#include "buzzy/distro/homebrew.h"
#include "buzzy/distro/rpm.h"

#define CLOG_CHANNEL  "distro:env"


static int
bz_detect_arch(void)
{
    /* There aren't any variables to override on Arch Linux. */
    return 0;
}


static int
bz_detect_homebrew(void)
{
    bool  is_homebrew;
    rii_check(bz_homebrew_is_present(&is_homebrew));
    if (is_homebrew) {
        struct bz_env  *env = bz_global_env();
        bz_env_add_override
            (env, "prefix",
             bz_interpolated_value_new("${homebrew.prefix}"));
    }
    return 0;
}


static int
bz_detect_redhat(void)
{
    bool  is_redhat;
    rii_check(bz_redhat_is_present(&is_redhat));
    if (is_redhat) {
        struct bz_env  *env = bz_global_env();
        const char  *arch = bz_posix_current_architecture();
        if (strcmp(arch, "x86_64") == 0) {
            clog_info("Installing libraries into /usr/lib64");
            bz_env_add_backup
                (env, "lib_dir_name", bz_string_value_new("lib64"));
        }
    }
    return 0;
}


int
bz_distro_add_env_overrides(void)
{
    rii_check(bz_detect_arch());
    rii_check(bz_detect_homebrew());
    rii_check(bz_detect_redhat());
    return 0;
}
