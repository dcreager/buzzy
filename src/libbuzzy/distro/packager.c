/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <string.h>

#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/distro.h"
#include "buzzy/package.h"

#include "buzzy/distro/arch.h"


struct bz_packager {
    const char  *name;
    bz_create_package_f  create;
};

static struct bz_packager  packagers[] = {
    { "pacman", bz_pacman_create_package },
    { NULL }
};


struct bz_action *
bz_create_package(struct bz_env *env, struct bz_action *stage_action)
{
    const char  *packager_name;
    struct bz_packager  *packager;

    rpp_check(packager_name = bz_env_get_string(env, "packager", true));
    for (packager = packagers; packager->name != NULL; packager++) {
        if (strcmp(packager_name, packager->name) == 0) {
            return packager->create(env, stage_action);
        }
    }
    bz_bad_config("Unknown packager \"%s\"", packager_name);
    return NULL;
}

static const char *
bz_packager__detect(void *user_data, struct bz_env *env)
{
    bool  is_arch;

    rpi_check(bz_arch_is_present(&is_arch));
    if (is_arch) {
        return "pacman";
    }

    bz_bad_config("Don't know what packager to use on this platform");
    return NULL;
}

struct bz_value_provider *
bz_packager_detector_new(void)
{
    return bz_value_provider_new(NULL, NULL, bz_packager__detect);
}
