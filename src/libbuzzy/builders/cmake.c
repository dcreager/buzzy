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
        bz_interpolated_value_new("${source_dir}/CMakeLists.txt"),
        "The location of the top-level CMake build script",
        ""
    );
}


/*-----------------------------------------------------------------------
 * CMake builder
 */

static int
bz_cmake__is_needed(void *user_data, bool *is_needed)
{
    /* We always let CMake built, test, and stage the package. */
    *is_needed = true;
    return 0;
}

static int
bz_cmake__build(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *build_dir;
    const char  *source_dir;
    const char  *install_prefix;
    const char  *build_type;
    bool  verbose = false;
    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    rii_check(bz_install_dependency_string("cmake"));
    rii_check(bz_build_message(env, "cmake"));

    rip_check(build_dir = bz_env_get_string(env, "build_dir", true));
    rip_check(source_dir = bz_env_get_string(env, "source_dir", true));
    rip_check(install_prefix = bz_env_get_string(env, "install_prefix", true));
    rip_check(build_type = bz_env_get_string(env, "cmake.build_type", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));

    /* Create the build path */
    rii_check(bz_create_directory(build_dir));

    /* $ cmake ${source_dir} */
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, source_dir);
    cork_buffer_printf
        (&buf, "-DCMAKE_INSTALL_PREFIX=%s", install_prefix);
    cork_exec_add_param(exec, buf.buf);
    cork_buffer_printf
        (&buf, "-DCMAKE_BUILD_TYPE=%s", build_type);
    cork_exec_add_param(exec, buf.buf);
    cork_exec_set_cwd(exec, build_dir);
    ei_check(bz_subprocess_run_exec(verbose, NULL, exec));

    /* $ cmake --build ${build_dir} */
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, "--build");
    cork_exec_add_param(exec, build_dir);
    ei_check(bz_subprocess_run_exec(verbose, NULL, exec));

    cork_buffer_done(&buf);
    return 0;

error:
    cork_buffer_done(&buf);
    return -1;
}

static int
bz_cmake__test(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *build_dir;
    bool  verbose = false;
    struct cork_exec  *exec;

    rii_check(bz_install_dependency_string("cmake"));
    rii_check(bz_test_message(env, "cmake"));

    /* $ cmake --build ${build_path} --target test */
    rip_check(build_dir = bz_env_get_string(env, "build_dir", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, "--build");
    cork_exec_add_param(exec, build_dir);
    cork_exec_add_param(exec, "--target");
    cork_exec_add_param(exec, "test");
    return bz_subprocess_run_exec(verbose, NULL, exec);
}

static int
bz_cmake__stage(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *build_dir;
    const char  *staging_dir;
    bool  verbose = false;
    struct cork_env  *exec_env;
    struct cork_exec  *exec;

    rii_check(bz_install_dependency_string("cmake"));
    rii_check(bz_stage_message(env, "cmake"));

    rip_check(build_dir = bz_env_get_string(env, "build_dir", true));
    rip_check(staging_dir = bz_env_get_string(env, "staging_dir", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));

    /* Create the staging path */
    rii_check(bz_create_directory(staging_dir));

    /* $ cmake --build ${build_path} --target install */
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, "--build");
    cork_exec_add_param(exec, build_dir);
    cork_exec_add_param(exec, "--target");
    cork_exec_add_param(exec, "install");
    exec_env = cork_env_clone_current();
    cork_env_add(exec_env, "DESTDIR", staging_dir);
    cork_exec_set_env(exec, exec_env);
    return bz_subprocess_run_exec(verbose, NULL, exec);
}


struct bz_builder *
bz_cmake_builder_new(struct bz_env *env)
{
    return bz_builder_new
        (env, "cmake", env, NULL,
         bz_cmake__is_needed, bz_cmake__build,
         bz_cmake__is_needed, bz_cmake__test,
         bz_cmake__is_needed, bz_cmake__stage);
}
