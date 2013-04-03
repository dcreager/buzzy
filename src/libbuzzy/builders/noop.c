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

#include "buzzy/action.h"
#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/os.h"


/*-----------------------------------------------------------------------
 * Noop builder
 */

static int
bz_noop__build__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_build_message(dest, env, "noop");
}

static int
bz_noop__build__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__build__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
bz_noop__build(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_noop__build__message,
         bz_noop__build__is_needed,
         bz_noop__build__perform);
}


static int
bz_noop__test__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_test_message(dest, env, "noop");
}

static int
bz_noop__test__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__test__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
bz_noop__test(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_noop__test__message,
         bz_noop__test__is_needed,
         bz_noop__test__perform);
}


static int
bz_noop__stage__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_stage_message(dest, env, "noop");
}

static int
bz_noop__stage__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__stage__perform(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *staging_path;

    /* Create the staging path */
    rip_check(staging_path = bz_env_get_string(env, "staging_path", true));
    rii_check(bz_create_directory(staging_path));

    return 0;
}

static struct bz_action *
bz_noop__stage(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_noop__stage__message,
         bz_noop__stage__is_needed,
         bz_noop__stage__perform);
}


struct bz_builder *
bz_noop_builder_new(struct bz_env *env)
{
    struct bz_builder  *builder;
    rpp_check(builder = bz_builder_new
              (env, "noop",
               bz_noop__build(env),
               bz_noop__test(env),
               bz_noop__stage(env)));
    ei_check(bz_builder_add_prereq_package(builder, "noop"));
    return builder;

error:
    bz_builder_free(builder);
    return NULL;
}
