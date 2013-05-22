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
 * Noop builder
 */

static int
bz_noop__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__build(void *user_data)
{
    struct bz_env  *env = user_data;
    return bz_build_message(env, "noop");
}

static int
bz_noop__test(void *user_data)
{
    struct bz_env  *env = user_data;
    return bz_test_message(env, "noop");
}

static int
bz_noop__stage(void *user_data)
{
    struct bz_env  *env = user_data;
    struct cork_path  *staging_path;

    rii_check(bz_stage_message(env, "noop"));

    /* Create the staging path */
    rip_check(staging_path = bz_env_get_path(env, "staging_path"));
    rii_check(bz_create_directory(cork_path_get(staging_path)));

    return 0;
}

struct bz_builder *
bz_noop_builder_new(struct bz_env *env)
{
    return bz_builder_new
        (env, "noop", env, NULL,
         bz_noop__is_needed, bz_noop__build,
         bz_noop__is_needed, bz_noop__test,
         bz_noop__is_needed, bz_noop__stage);
}
