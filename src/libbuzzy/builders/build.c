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
#include "buzzy/builders.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/package.h"

#include "buzzy/builders/cmake.h"


struct bz_builder {
    const char  *name;
    bz_builder_new_f  build;
};

static struct bz_builder  builders[] = {
    { "cmake", bz_cmake_new },
    { NULL }
};


struct bz_recipe *
bz_builder_new(struct bz_env *env, struct bz_action *source_action)
{
    const char  *builder_name;
    struct bz_builder  *builder;

    rpp_check(builder_name = bz_env_get_string(env, "builder", true));
    for (builder = builders; builder->name != NULL; builder++) {
        if (strcmp(builder_name, builder->name) == 0) {
            return builder->build(env, source_action);
        }
    }
    bz_bad_config("Unknown builder \"%s\"", builder_name);
    return NULL;
}
