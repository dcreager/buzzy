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
#include "buzzy/env.h"
#include "buzzy/os.h"
#include "buzzy/recipe.h"


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
}


/*-----------------------------------------------------------------------
 * CMake builder
 */

static int
bz_cmake__build(void *user_data, struct bz_env *env)
{
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

static int
bz_cmake__test(void *user_data, struct bz_env *env)
{
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

static int
bz_cmake__stage(void *user_data, struct bz_env *env)
{
    const char  *build_path;
    const char  *staging_path;
    bool  verbose = false;
    struct cork_env  *exec_env;
    struct cork_exec  *exec;

    rip_check(build_path = bz_env_get_string(env, "build_path", true));
    rip_check(staging_path = bz_env_get_string(env, "staging_path", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));

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

struct bz_recipe *
bz_cmake_new(struct bz_env *env, struct bz_action *source_action)
{
    struct bz_recipe  *recipe;
    rpp_check(recipe = bz_recipe_new
              (env, "cmake", source_action, NULL, NULL,
               bz_cmake__build, bz_cmake__test, bz_cmake__stage));
    ei_check(bz_recipe_add_prereq_package(recipe, "cmake"));
    return recipe;

error:
    bz_recipe_free(recipe);
    return NULL;
}
