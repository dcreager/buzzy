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
#include "buzzy/value.h"
#include "buzzy/distro/homebrew.h"

#define CLOG_CHANNEL  "homebrew"


/*-----------------------------------------------------------------------
 * Homebrew path values
 */

static void
bz_homebrew_cellar__free(void *user_data)
{
    struct cork_buffer  *buf = user_data;
    cork_buffer_free(buf);
}

static const char *
bz_homebrew_cellar__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    if (buf->size == 0) {
        char  *value;
        rpi_check(bz_subprocess_get_output
                  (buf, NULL, NULL, "brew", "--cellar", NULL));
        /* Chomp the trailing newline */
        value = buf->buf;
        value[--buf->size] = '\0';
    }
    return buf->buf;
}

static struct bz_value *
bz_homebrew_cellar_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, bz_homebrew_cellar__free, bz_homebrew_cellar__get);
}


static void
bz_homebrew_prefix__free(void *user_data)
{
    struct cork_buffer  *buf = user_data;
    cork_buffer_free(buf);
}

static const char *
bz_homebrew_prefix__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    if (buf->size == 0) {
        char  *value;
        rpi_check(bz_subprocess_get_output
                  (buf, NULL, NULL, "brew", "--prefix", NULL));
        /* Chomp the trailing newline */
        value = buf->buf;
        value[--buf->size] = '\0';
    }
    return buf->buf;
}

static struct bz_value *
bz_homebrew_prefix_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, bz_homebrew_prefix__free, bz_homebrew_prefix__get);
}


/*-----------------------------------------------------------------------
 * Builtin homebrew variables
 */

bz_define_variables(homebrew)
{
    bz_package_variable(
        cellar, "homebrew.cellar",
        bz_homebrew_cellar_value_new(),
        "The location of the Homebrew cellar",
        ""
    );

    bz_package_variable(
        cellar, "homebrew.prefix",
        bz_homebrew_prefix_value_new(),
        "The location of the Homebrew installation prefix",
        ""
    );

    bz_package_variable(
        cellar, "homebrew.pkg_cellar",
        bz_interpolated_value_new("${homebrew.cellar}/${name}"),
        "The location of this package in the Homebrew cellar",
        ""
    );

    bz_package_variable(
        cellar, "homebrew.pkg_ver_cellar",
        bz_interpolated_value_new("${homebrew.pkg_cellar}/${version}"),
        "The location of this package in the Homebrew cellar",
        ""
    );

    bz_package_variable(
        cellar, "homebrew.staged_cellar",
        bz_interpolated_value_new("${staging_dir}${prefix}"),
        "The staged contents of the package's cellar",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Creating homebrew packages
 */

static int
bz_homebrew__package_install__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    bool  force = false;

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rie_check(force = bz_env_get_bool(env, "force", true));

    if (force) {
        *is_needed = true;
        clog_info("(%s) Force installation of package", package_name);
        return 0;
    } else {
        struct cork_path  *pkg_ver_cellar;
        rip_check(pkg_ver_cellar =
                  bz_env_get_path(env, "homebrew.pkg_ver_cellar", true));
        clog_info("(%s) Check whether package %s exists",
                  package_name, cork_path_get(pkg_ver_cellar));
        rii_check(bz_file_exists(cork_path_get(pkg_ver_cellar), is_needed));
        *is_needed = !*is_needed;
        return 0;
    }
}

static int
bz_homebrew__package(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *pkg_cellar;
    struct cork_path  *pkg_ver_cellar;
    struct cork_path  *staged_cellar;

    rii_check(bz_package_message(env, "Homebrew"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Package using Homebrew", package_name);

    rip_check(pkg_cellar =
              bz_env_get_path(env, "homebrew.pkg_cellar", true));
    rip_check(pkg_ver_cellar =
              bz_env_get_path(env, "homebrew.pkg_ver_cellar", true));
    rip_check(staged_cellar =
              bz_env_get_path(env, "homebrew.staged_cellar", true));

    rii_check(bz_create_directory(cork_path_get(pkg_cellar)));
    return bz_subprocess_run
          (false, NULL,
           "cp", "-R",
           cork_path_get(staged_cellar), cork_path_get(pkg_ver_cellar),
           NULL);
}

static int
bz_homebrew__install(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;

    rii_check(bz_install_message(env, "Homebrew"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Install using Homebrew", package_name);

    return bz_subprocess_run
        (false, NULL,
         "brew", "link", package_name,
         NULL);
}


static int
bz_homebrew__uninstall__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    bool  force = false;

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rie_check(force = bz_env_get_bool(env, "force", true));

    if (force) {
        *is_needed = true;
        clog_info("(%s) Force installation of package", package_name);
        return 0;
    } else {
        struct cork_path  *pkg_cellar;
        rip_check(pkg_cellar =
                  bz_env_get_path(env, "homebrew.pkg_cellar", true));
        clog_info("(%s) Check whether package %s exists",
                  package_name, cork_path_get(pkg_cellar));
        return bz_file_exists(cork_path_get(pkg_cellar), is_needed);
    }
}

static int
bz_homebrew__uninstall(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;

    rii_check(bz_uninstall_message(env, "Homebrew"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Uninstall using Homebrew", package_name);
    return bz_subprocess_run
        (false, NULL,
         "brew", "remove", package_name,
         NULL);
}


struct bz_packager *
bz_homebrew_packager_new(struct bz_env *env)
{
    return bz_packager_new
        (env, "homebrew", env, NULL,
         bz_homebrew__package_install__is_needed, bz_homebrew__package,
         bz_homebrew__package_install__is_needed, bz_homebrew__install,
         bz_homebrew__uninstall__is_needed, bz_homebrew__uninstall);
}
