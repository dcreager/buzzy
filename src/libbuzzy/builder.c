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
#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Standard messages
 */

int
bz_build_message(struct cork_buffer *dest, struct bz_env *env,
                 const char *builder_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    cork_buffer_append_printf
        (dest, "Build %s %s (%s)", package_name, version, builder_name);
    return 0;
}

int
bz_test_message(struct cork_buffer *dest, struct bz_env *env,
                const char *builder_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    cork_buffer_append_printf
        (dest, "Test %s %s (%s)", package_name, version, builder_name);
    return 0;
}

int
bz_stage_message(struct cork_buffer *dest, struct bz_env *env,
                 const char *builder_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    cork_buffer_append_printf
        (dest, "Stage %s %s (%s)", package_name, version, builder_name);
    return 0;
}


/*-----------------------------------------------------------------------
 * Builders
 */

struct bz_builder {
    struct bz_env  *env;
    const char  *builder_name;
    struct bz_action  *build;
    struct bz_action  *test;
    struct bz_action  *stage;
};


struct bz_builder *
bz_builder_new(struct bz_env *env, const char *builder_name,
               struct bz_action *build,
               struct bz_action *test,
               struct bz_action *stage)
{
    struct bz_builder  *builder = cork_new(struct bz_builder);
    builder->env = env;
    builder->builder_name = cork_strdup(builder_name);
    builder->build = build;
    builder->test = test;
    builder->stage = stage;
    bz_action_add_pre(builder->test, builder->build);
    bz_action_add_pre(builder->stage, builder->build);
    return builder;
}

void
bz_builder_free(struct bz_builder *builder)
{
    cork_strfree(builder->builder_name);
    bz_action_free(builder->build);
    bz_action_free(builder->test);
    bz_action_free(builder->stage);
    free(builder);
}

void
bz_builder_add_prereq(struct bz_builder *builder, struct bz_action *action)
{
    bz_action_add_pre(builder->build, action);
    bz_action_add_pre(builder->test, action);
    bz_action_add_pre(builder->stage, action);
}

int
bz_builder_add_prereq_package(struct bz_builder *builder, const char *dep_string)
{
    struct bz_action  *prereq;
    rip_check(prereq = bz_install_dependency_string(dep_string));
    bz_builder_add_prereq(builder, prereq);
    return 0;
}


struct bz_action *
bz_builder_build_action(struct bz_builder *builder)
{
    return builder->build;
}

struct bz_action *
bz_builder_test_action(struct bz_builder *builder)
{
    return builder->test;
}

struct bz_action *
bz_builder_stage_action(struct bz_builder *builder)
{
    return builder->stage;
}


/*-----------------------------------------------------------------------
 * Available builders
 */

struct bz_builder_reg {
    const char  *name;
    struct bz_builder *(*new_builder)(struct bz_env *env);
};

static struct bz_builder_reg  builders[] = {
    { "cmake", bz_cmake_builder_new },
    { "noop", bz_noop_builder_new },
    { NULL }
};

struct bz_builder *
bz_package_builder_new(struct bz_env *env)
{
    const char  *builder_name;
    struct bz_builder_reg  *builder;

    rpp_check(builder_name = bz_env_get_string(env, "builder", true));
    for (builder = builders; builder->name != NULL; builder++) {
        if (strcmp(builder_name, builder->name) == 0) {
            return builder->new_builder(env);
        }
    }
    bz_bad_config("Unknown builder \"%s\"", builder_name);
    return NULL;
}
