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

#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/os.h"


/*-----------------------------------------------------------------------
 * Noop packager
 */

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

struct bz_packager *
bz_noop_packager_new(struct bz_env *env)
{
    return bz_packager_new
        (env, "noop", env, NULL, bz_noop__package, bz_noop__install);
}
