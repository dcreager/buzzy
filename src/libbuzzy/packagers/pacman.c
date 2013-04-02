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
#include "buzzy/distro/arch.h"


/*-----------------------------------------------------------------------
 * Architecture values
 */

static const char *
bz_architecture_value__provide(void *user_data, struct bz_env *env)
{
    return bz_arch_current_architecture();
}

static struct bz_value_provider *
bz_architecture_value_new(void)
{
    return bz_value_provider_new(NULL, NULL, bz_architecture_value__provide);
}


/*-----------------------------------------------------------------------
 * pacman version values
 */

static void
bz_pacman_version__free(void *user_data)
{
    struct cork_buffer  *buf = user_data;
    cork_buffer_free(buf);
}

static const char *
bz_pacman_version__provide(void *user_data, struct bz_env *env)
{
    struct cork_buffer  *buf = user_data;
    struct bz_version  *version;
    rpp_check(version = bz_env_get_version(env, "version", true));
    cork_buffer_clear(buf);
    bz_version_to_arch(version, buf);
    bz_version_free(version);
    return buf->buf;
}

static struct bz_value_provider *
bz_pacman_version_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_value_provider_new
        (buf, bz_pacman_version__free, bz_pacman_version__provide);
}


/*-----------------------------------------------------------------------
 * Builtin pacman variables
 */

bz_define_variables(pacman)
{
    bz_package_variable(
        package_filename, "pacman.package_filename",
        bz_interpolated_value_new(
            "${name}-${pacman.version}-${pacman.pkgrel}-"
            "${pacman.arch}"
            "${pacman.pkgext}"
        ),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        package_path, "pacman.package_path",
        bz_interpolated_value_new("${package_path}/${pacman.package_filename}"),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        architecture, "pacman.pkgbuild",
        bz_interpolated_value_new("${pkg_path}/PKGBUILD"),
        "The location of the PKGBUILD file we should create",
        ""
    );

    bz_package_variable(
        architecture, "pacman.arch",
        bz_architecture_value_new(),
        "The architecture to build pacman packages for",
        ""
    );

    bz_package_variable(
        pkgrel, "pacman.pkgrel",
        bz_string_value_new("1"),
        "The package release number to use for any packages we create",
        ""
    );

    bz_package_variable(
        pkgext, "pacman.pkgext",
        bz_string_value_new(".pkg.tar.xz"),
        "The extension for any packages we create",
        ""
    );

    bz_package_variable(
        version, "pacman.version",
        bz_pacman_version_value_new(),
        "The pacman equivalent of the package's version",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Creating pacman packages
 */

static void
bz_pacman__package__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    bz_package_message(dest, env, "pacman");
}

static int
bz_pacman__package__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    bool  force = false;

    rii_check(bz_env_get_bool(env, "force", &force, false));

    if (force) {
        *is_needed = true;
        return 0;
    } else {
        const char  *package_path;
        rip_check(package_path = bz_env_get_string
                  (env, "pacman.package_path", true));
        rii_check(bz_file_exists_from_string(package_path, is_needed));
        *is_needed = !*is_needed;
        return 0;
    }
}

static int
bz_pacman__package__perform(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *staging_path;
    const char  *package_path;
    const char  *pkg_path;
    const char  *pkgbuild_path;
    const char  *package_name;
    const char  *version;
    const char  *pkgrel;
    const char  *pkgext;
    const char  *architecture;
    const char  *license;
    bool  verbose = false;

    struct cork_env  *exec_env;
    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    bool  staging_exists;

    rip_check(staging_path = bz_env_get_string(env, "staging_path", true));
    rip_check(package_path = bz_env_get_string(env, "package_path", true));
    rip_check(pkg_path = bz_env_get_string(env, "pkg_path", true));
    rip_check(pkgbuild_path = bz_env_get_string(env, "pacman.pkgbuild", true));
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "pacman.version", true));
    rip_check(pkgrel = bz_env_get_string(env, "pacman.pkgrel", true));
    rip_check(pkgext = bz_env_get_string(env, "pacman.pkgext", true));
    rip_check(architecture = bz_env_get_string(env, "pacman.arch", true));
    rip_check(license = bz_env_get_string(env, "license", true));
    rii_check(bz_env_get_bool(env, "verbose", &verbose, false));

    rii_check(bz_file_exists_from_string(staging_path, &staging_exists));
    if (CORK_UNLIKELY(!staging_exists)) {
        cork_error_set
            (CORK_BUILTIN_ERROR, CORK_SYSTEM_ERROR,
             "Staging directory %s does not exist", staging_path);
        return -1;
    }

    /* Create a temporary directory */
    rii_check(bz_create_directory_from_string(pkg_path));

    /* Create a PKGBUILD file for this package */
    cork_buffer_append_printf(&buf, "pkgname='%s'\n", package_name);
    cork_buffer_append_printf(&buf, "pkgver='%s'\n", version);
    cork_buffer_append_printf(&buf, "pkgrel='%s'\n", pkgrel);
    cork_buffer_append_printf(&buf, "arch=('%s')\n", architecture);
    cork_buffer_append_printf(&buf, "license=('%s')\n", license);
    /* TODO: dependencies */
    cork_buffer_append_printf(&buf,
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '%s' \"${pkgdir}\"\n"
        "}\n",
        staging_path
    );

    ei_check(bz_create_file_from_string(pkgbuild_path, &buf));
    cork_buffer_done(&buf);

    exec_env = cork_env_clone_current();
    cork_env_add(exec_env, "PKGDEST", package_path);
    cork_env_add(exec_env, "PKGEXT", pkgext);
    exec = cork_exec_new_with_params("makepkg", "-sf", NULL);
    cork_exec_set_cwd(exec, pkg_path);
    cork_exec_set_env(exec, exec_env);
    return bz_subprocess_run_exec(verbose, NULL, exec);

error:
    cork_buffer_done(&buf);
    return -1;
}

static struct bz_action *
bz_pacman__package(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_pacman__package__message,
         bz_pacman__package__is_needed,
         bz_pacman__package__perform);
}


static void
bz_pacman__install__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_env  *env = user_data;
    bz_install_message(dest, env, "pacman");
}

static int
bz_pacman__install__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    bool  force = false;

    rii_check(bz_env_get_bool(env, "force", &force, false));

    if (force) {
        *is_needed = true;
        return 0;
    } else {
        const char  *package_name;
        struct bz_version  *package_version;
        struct bz_version  *installed;

        rip_check(package_name = bz_env_get_string(env, "name", true));
        rip_check(package_version = bz_env_get_version(env, "version", true));

        installed = bz_arch_native_version_installed(package_name);
        if (CORK_UNLIKELY(cork_error_occurred())) {
            bz_version_free(package_version);
            return -1;
        }

        if (installed == NULL) {
            bz_version_free(package_version);
            *is_needed = true;
            return 0;
        } else {
            *is_needed = (bz_version_cmp(installed, package_version) < 0);
            bz_version_free(package_version);
            bz_version_free(installed);
            return 0;
        }
    }
}

static int
bz_pacman__install__perform(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_path;
    rip_check(package_path = bz_env_get_string
              (env, "pacman.package_path", true));
    return bz_subprocess_run
        (false, NULL,
         "sudo", "pacman", "-U", "--noconfirm", package_path,
         NULL);
}

static struct bz_action *
bz_pacman__install(struct bz_env *env)
{
    return bz_action_new
        (env, NULL,
         bz_pacman__install__message,
         bz_pacman__install__is_needed,
         bz_pacman__install__perform);
}


struct bz_packager *
bz_pacman_packager_new(struct bz_env *env)
{
    struct bz_packager  *packager;
    rpp_check(packager = bz_packager_new
              (env, "pacman",
               bz_pacman__package(env),
               bz_pacman__install(env)));
    ei_check(bz_packager_add_prereq_package(packager, "pacman"));
    return packager;

error:
    bz_packager_free(packager);
    return NULL;
}
