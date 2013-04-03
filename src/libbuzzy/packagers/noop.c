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
 * Noop packager
 */

static int
bz_noop__package__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_package_message(dest, env, "noop");
}

static int
bz_noop__package__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__package__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
bz_noop__package(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_noop__package__message,
         bz_noop__package__is_needed,
         bz_noop__package__perform);
}


static int
bz_noop__install__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_install_message(dest, env, "noop");
}

static int
bz_noop__install__is_needed(void *user_data, bool *is_needed)
{
    *is_needed = true;
    return 0;
}

static int
bz_noop__install__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
bz_noop__install(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_noop__install__message,
         bz_noop__install__is_needed,
         bz_noop__install__perform);
}


struct bz_packager *
bz_noop_packager_new(struct bz_env *env)
{
    struct bz_packager  *packager;
    rpp_check(packager = bz_packager_new
              (env, "noop",
               bz_noop__package(env),
               bz_noop__install(env)));
    ei_check(bz_packager_add_prereq_package(packager, "noop"));
    return packager;

error:
    bz_packager_free(packager);
    return NULL;
}
