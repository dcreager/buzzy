/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/os.h"

#define CLOG_CHANNEL  "cmake"


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
    const char  *package_name;
    struct cork_path  *build_dir;
    struct cork_path  *source_dir;
    struct cork_path  *install_prefix;
    const char  *build_type;
    bool  verbose;
    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    rii_check(bz_install_dependency_string("cmake", env));
    rii_check(bz_build_message(env, "cmake"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(build_dir = bz_env_get_path(env, "build_dir", true));
    rip_check(source_dir = bz_env_get_path(env, "source_dir", true));
    rip_check(install_prefix = bz_env_get_path(env, "install_prefix", true));
    rip_check(build_type = bz_env_get_string(env, "cmake.build_type", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));

    /* Create the build path */
    rii_check(bz_create_directory(cork_path_get(build_dir)));

    /* $ cmake ${source_dir} */
    clog_info("(%s) Configure using cmake", package_name);
    exec = cork_exec_new("cmake");
    cork_exec_add_param(exec, "cmake");
    cork_exec_add_param(exec, cork_path_get(source_dir));
    cork_buffer_printf
        (&buf, "-DCMAKE_INSTALL_PREFIX=%s", cork_path_get(install_prefix));
    cork_exec_add_param(exec, buf.buf);
    cork_buffer_printf
        (&buf, "-DCMAKE_BUILD_TYPE=%s", build_type);
    cork_exec_add_param(exec, buf.buf);
    cork_exec_set_cwd(exec, cork_path_get(build_dir));
    ei_check(bz_subprocess_run_exec(verbose, NULL, exec));

    /* $ make */
    clog_info("(%s) Build using cmake", package_name);
    exec = cork_exec_new("make");
    cork_exec_add_param(exec, "make");
    cork_exec_set_cwd(exec, cork_path_get(build_dir));
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
    const char  *package_name;
    struct cork_path  *build_dir;
    bool  verbose;
    struct cork_exec  *exec;

    rii_check(bz_install_dependency_string("cmake", env));
    rii_check(bz_test_message(env, "cmake"));

    /* $ make test */
    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Test using cmake", package_name);
    rip_check(build_dir = bz_env_get_path(env, "build_dir", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));
    exec = cork_exec_new("make");
    cork_exec_add_param(exec, "make");
    cork_exec_add_param(exec, "test");
    cork_exec_set_cwd(exec, cork_path_get(build_dir));
    return bz_subprocess_run_exec(verbose, NULL, exec);
}

static int
bz_cmake__stage(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *build_dir;
    struct cork_path  *staging_dir;
    bool  verbose;
    struct cork_env  *exec_env;
    struct cork_exec  *exec;

    rii_check(bz_install_dependency_string("cmake", env));
    rii_check(bz_stage_message(env, "cmake"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Stage using cmake", package_name);
    rip_check(build_dir = bz_env_get_path(env, "build_dir", true));
    rip_check(staging_dir = bz_env_get_path(env, "staging_dir", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));

    /* Create the staging path */
    rii_check(bz_create_directory(cork_path_get(staging_dir)));

    /* $ make install */
    exec = cork_exec_new("make");
    cork_exec_add_param(exec, "make");
    cork_exec_add_param(exec, "install");
    cork_exec_set_cwd(exec, cork_path_get(build_dir));
    exec_env = cork_env_clone_current();
    cork_env_add(exec_env, "DESTDIR", cork_path_get(staging_dir));
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
