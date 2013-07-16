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

#include "buzzy/env.h"
#include "buzzy/os.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Noop packager
 */

static int
bz_noop__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__package(void *user_data)
{
    struct bz_env  *env = user_data;
    return bz_package_message(env, "noop");
}

static int
bz_noop__install(void *user_data)
{
    struct bz_env  *env = user_data;
    return bz_install_message(env, "noop");
}

static int
bz_noop__uninstall(void *user_data)
{
    struct bz_env  *env = user_data;
    return bz_uninstall_message(env, "noop");
}

struct bz_packager *
bz_noop_packager_new(struct bz_env *env)
{
    return bz_packager_new
        (env, "noop", env, NULL,
         bz_noop__is_needed, bz_noop__package,
         bz_noop__is_needed, bz_noop__install,
         bz_noop__is_needed, bz_noop__uninstall);
}
