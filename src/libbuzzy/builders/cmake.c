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
 * Builtin CMake variables
 */

bz_define_variables(cmake)
{
    bz_package_variable(
        install_prefix, "cmake.build_type",
        bz_string_value_new("RelWithDebInfo"),
        "The CMake build type for this package",
        ""
    );

    bz_package_variable(
        install_prefix, "cmake.cmakelists",
        bz_interpolated_value_new("${source_path}/CMakeLists.txt"),
        "The location of the top-level CMake build script",
        ""
    );
}


/*-----------------------------------------------------------------------
 * CMake builder
 */

static int
bz_cmake__build__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_build_message(dest, env, "cmake");
}

static int
bz_cmake__build__is_needed(void *user_data, bool *is_needed)
{
    /* Let CMake determine what needs to be rebuilt.  We'll reuse the build
     * directory from the last time we tried to build this particular version of
     * the package, so we might be able to reuse some work. */
    *is_needed = true;
    return 0;
}

static int
bz_cmake__build__perform(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *build_path;
    const char  *source_path;
    const char  *install_prefix;
    const char  *build_type;
    bool  verbose = false;
    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    rip_check(build_path = bz_env_get_string(env, "build_path", true));
    rip_check(source_path = bz_env_get_string(env, "source_path", true));
    rip_check(install_prefix = bz_env_get_string(env, "install_prefix", true));
    rip_check(build_type = bz_env_get_string(env, "cmake.build_type", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));

    /* Create the build path */
    rii_check(bz_create_directory(build_path));

    /* $ cmake ${source_path} */
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, source_path);
    cork_buffer_printf
        (&buf, "-DCMAKE_INSTALL_PREFIX=%s", install_prefix);
    cork_exec_add_param(exec, buf.buf);
    cork_buffer_printf
        (&buf, "-DCMAKE_BUILD_TYPE=%s", build_type);
    cork_exec_add_param(exec, buf.buf);
    cork_exec_set_cwd(exec, build_path);
    ei_check(bz_subprocess_run_exec(verbose, NULL, exec));

    /* $ cmake --build ${build_path} */
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, "--build");
    cork_exec_add_param(exec, build_path);
    ei_check(bz_subprocess_run_exec(verbose, NULL, exec));

    cork_buffer_done(&buf);
    return 0;

error:
    cork_buffer_done(&buf);
    return -1;
}

static struct bz_action *
bz_cmake__build(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_cmake__build__message,
         bz_cmake__build__is_needed,
         bz_cmake__build__perform);
}


static int
bz_cmake__test__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_test_message(dest, env, "cmake");
}

static int
bz_cmake__test__is_needed(void *user_data, bool *is_needed)
{
    /* We only perform tests when explicitly asked to, and when asked, we always
     * perform them. */
    *is_needed = true;
    return 0;
}

static int
bz_cmake__test__perform(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *build_path;
    bool  verbose = false;
    struct cork_exec  *exec;

    /* $ cmake --build ${build_path} --target test */
    rip_check(build_path = bz_env_get_string(env, "build_path", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, "--build");
    cork_exec_add_param(exec, build_path);
    cork_exec_add_param(exec, "--target");
    cork_exec_add_param(exec, "test");
    return bz_subprocess_run_exec(verbose, NULL, exec);
}

static struct bz_action *
bz_cmake__test(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_cmake__test__message,
         bz_cmake__test__is_needed,
         bz_cmake__test__perform);
}


static int
bz_cmake__stage__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    return bz_stage_message(dest, env, "cmake");
}

static int
bz_cmake__stage__is_needed(void *user_data, bool *is_needed)
{
    /* Always perform a fresh staged installation. */
    *is_needed = true;
    return 0;
}

static int
bz_cmake__stage__perform(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *build_path;
    const char  *staging_path;
    bool  verbose = false;
    struct cork_env  *exec_env;
    struct cork_exec  *exec;

    rip_check(build_path = bz_env_get_string(env, "build_path", true));
    rip_check(staging_path = bz_env_get_string(env, "staging_path", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));

    /* Create the staging path */
    rii_check(bz_create_directory(staging_path));

    /* $ cmake --build ${build_path} --target install */
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, "--build");
    cork_exec_add_param(exec, build_path);
    cork_exec_add_param(exec, "--target");
    cork_exec_add_param(exec, "install");
    exec_env = cork_env_clone_current();
    cork_env_add(exec_env, "DESTDIR", staging_path);
    cork_exec_set_env(exec, exec_env);
    return bz_subprocess_run_exec(verbose, NULL, exec);
}

static struct bz_action *
bz_cmake__stage(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_cmake__stage__message,
         bz_cmake__stage__is_needed,
         bz_cmake__stage__perform);
}


struct bz_builder *
bz_cmake_builder_new(struct bz_env *env)
{
    struct bz_builder  *builder;
    rpp_check(builder = bz_builder_new
              (env, "cmake",
               bz_cmake__build(env),
               bz_cmake__test(env),
               bz_cmake__stage(env)));
    ei_check(bz_builder_add_prereq_package(builder, "cmake"));
    return builder;

error:
    bz_builder_free(builder);
    return NULL;
}
