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

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/package.h"

#define CLOG_CHANNEL  "autotools"


/*-----------------------------------------------------------------------
 * Builtin Autotools variables
 */

bz_define_variables(autotools)
{
    bz_package_variable(
        configure_in, "autotools.configure.configure_in",
        bz_interpolated_value_new("${source_dir}/configure.ac"),
        "The location of the package's configure.{ac,in} file",
        ""
    );

    bz_package_variable(
        configure, "autotools.configure.configure",
        bz_interpolated_value_new("${source_dir}/configure"),
        "The location of the package's configure script",
        ""
    );

    bz_package_variable(
        configure, "autotools.configure.args",
        NULL,
        "Additional arguments to pass in to the configure script",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Autotools builder
 */

static int
bz_autotools__is_needed(void *user_data, bool *is_needed)
{
    /* We always let Autotools built, test, and stage the package. */
    *is_needed = true;
    return 0;
}

struct bz_configure_add_arg {
    struct cork_exec  *exec;
    struct bz_env  *env;
};

static int
bz_configure_add_arg(void *user_data, struct bz_value *value)
{
    struct bz_configure_add_arg  *state = user_data;
    const char  *scalar;
    rip_check(scalar = bz_scalar_value_get(value, bz_env_as_value(state->env)));
    cork_exec_add_param(state->exec, scalar);
    return 0;
}

static int
bz_autotools__build(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *build_dir;
    struct cork_path  *source_dir;
    struct cork_path  *configure;
    struct bz_value  *configure_args;
    const char  *pkgconfig_path;
    bool  exists;
    bool  verbose;
    struct cork_exec  *exec;
    struct cork_env  *exec_env;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    rii_check(bz_install_dependency_string("autoconf", env));
    rii_check(bz_install_dependency_string("automake", env));
    rii_check(bz_build_message(env, "autotools"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(build_dir = bz_env_get_path(env, "build_dir", true));
    rip_check(source_dir = bz_env_get_path(env, "source_dir", true));
    rip_check(configure =
              bz_env_get_path(env, "autotools.configure.configure", true));
    rie_check(pkgconfig_path = bz_env_get_string(env, "pkgconfig.path", false));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));

    /* Create the build path */
    rii_check(bz_create_directory(cork_path_get(build_dir)));

    clog_info("(%s) Configure using autotools", package_name);

    /* $ autoreconf -i */
    rii_check(bz_file_exists(cork_path_get(configure), &exists));
    if (!exists) {
        exec = cork_exec_new("autoreconf");
        cork_exec_add_param(exec, "autoreconf");
        cork_exec_add_param(exec, "-i");
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        ei_check(bz_subprocess_run_exec(verbose, NULL, exec));
    }

#define add_dir(buzzy_name, param_name) \
    do { \
        struct cork_path  *value; \
        ep_check(value = bz_env_get_path(env, buzzy_name, true)); \
        cork_buffer_printf(&buf, "--" param_name "=%s", cork_path_get(value)); \
        cork_exec_add_param(exec, buf.buf); \
    } while (0)

    /* $ ./configure ... */
    exec = cork_exec_new(cork_path_get(configure));
    exec_env = cork_env_clone_current();
    cork_exec_set_env(exec, exec_env);
    cork_exec_add_param(exec, cork_path_get(configure));
    add_dir("prefix", "prefix");
    add_dir("exec_prefix", "exec-prefix");
    add_dir("bin_dir", "bindir");
    add_dir("sbin_dir", "sbindir");
    add_dir("lib_dir", "libdir");
    add_dir("libexec_dir", "libexecdir");
    add_dir("share_dir", "datadir");
    add_dir("man_dir", "mandir");

    /* Add custom configure arguments, if given. */
    configure_args = bz_env_get_value(env, "autotools.configure.args");
    if (configure_args != NULL) {
        struct bz_configure_add_arg  state = { exec, env };

        switch (bz_value_kind(configure_args)) {
            case BZ_VALUE_SCALAR:
                ei_check(bz_configure_add_arg(&state, configure_args));
                break;

            case BZ_VALUE_ARRAY:
                ei_check(bz_array_value_map
                         (configure_args, &state, bz_configure_add_arg));
                break;

            case BZ_VALUE_MAP:
                bz_bad_config("autotools.configure.args cannot be a map");
                goto error;

            default:
                cork_unreachable();
        }
    }

    cork_exec_set_cwd(exec, cork_path_get(build_dir));
    if (pkgconfig_path != NULL) {
        cork_env_add(exec_env, "PKG_CONFIG_PATH", pkgconfig_path);
    }
    ei_check(bz_subprocess_run_exec(verbose, NULL, exec));

    /* $ make */
    clog_info("(%s) Build using autotools", package_name);
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
bz_autotools__test(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *build_dir;
    bool  verbose;
    struct cork_exec  *exec;

    rii_check(bz_test_message(env, "autotools"));

    /* $ make check */
    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Test using autotools", package_name);
    rip_check(build_dir = bz_env_get_path(env, "build_dir", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));
    exec = cork_exec_new("make");
    cork_exec_add_param(exec, "make");
    cork_exec_add_param(exec, "check");
    cork_exec_set_cwd(exec, cork_path_get(build_dir));
    return bz_subprocess_run_exec(verbose, NULL, exec);
}

static int
bz_autotools__stage(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *build_dir;
    struct cork_path  *staging_dir;
    bool  verbose;
    struct cork_env  *exec_env;
    struct cork_exec  *exec;

    rii_check(bz_stage_message(env, "autotools"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Stage using autotools", package_name);
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
bz_autotools_builder_new(struct bz_env *env)
{
    return bz_builder_new
        (env, "autotools", env, NULL,
         bz_autotools__is_needed, bz_autotools__build,
         bz_autotools__is_needed, bz_autotools__test,
         bz_autotools__is_needed, bz_autotools__stage);
}
