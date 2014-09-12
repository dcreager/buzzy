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
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/value.h"
#include "buzzy/distro/arch.h"

#define CLOG_CHANNEL  "pacman"


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
bz_pacman_version__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    struct bz_version  *version;
    rpp_check(version = bz_value_get_version(ctx, "version", true));
    cork_buffer_clear(buf);
    bz_version_to_arch(version, buf);
    return buf->buf;
}

static struct bz_value *
bz_pacman_version_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, bz_pacman_version__free, bz_pacman_version__get);
}


/*-----------------------------------------------------------------------
 * Builtin pacman variables
 */

bz_define_variables(pacman)
{
    bz_package_variable(
        package_file_base, "pacman.package_file_base",
        bz_interpolated_value_new(
            "${name}-${pacman.version}-${pacman.pkgrel}-"
            "${pacman.arch}"
            "${pacman.pkgext}"
        ),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        package_file, "pacman.package_file",
        bz_interpolated_value_new
            ("${binary_package_dir}/${pacman.package_file_base}"),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        architecture, "pacman.pkgbuild",
        bz_interpolated_value_new("${package_build_dir}/PKGBUILD"),
        "The location of the PKGBUILD file we should create",
        ""
    );

    bz_package_variable(
        architecture, "pacman.install_base",
        bz_interpolated_value_new("${native_name}.install"),
        "The base name of the install script file we should create",
        ""
    );

    bz_package_variable(
        architecture, "pacman.install",
        bz_interpolated_value_new
            ("${package_build_dir}/${pacman.install_base}"),
        "The location of the install script file we should create",
        ""
    );

    bz_package_variable(
        architecture, "pacman.arch",
        bz_interpolated_value_new("${arch}"),
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

static int
bz_pacman__package__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    bool  force = false;

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rie_check(force = bz_env_get_bool(env, "force", true));

    if (force) {
        *is_needed = true;
        clog_info("(%s) Force creation of binary package",
                  package_name);
        return 0;
    } else {
        struct cork_path  *package_file;
        rip_check(package_file = bz_env_get_path
                  (env, "pacman.package_file", true));
        clog_info("(%s) Check whether binary package %s exists",
                  package_name, cork_path_get(package_file));
        rii_check(bz_file_exists(cork_path_get(package_file), is_needed));
        *is_needed = !*is_needed;
        return 0;
    }
}

struct bz_pacman_fill_deps {
    struct bz_value  *ctx;
    struct cork_buffer  dep_buf;
    bool  first;
};

static int
bz_pacman_fill_one_dep(void *user_data, struct bz_value *dep_value)
{
    struct bz_pacman_fill_deps  *state = user_data;
    const char  *dep_string;
    struct bz_dependency  *dep;
    struct bz_package  *dep_package;
    struct bz_env  *dep_env;
    const char  *dep_name;
    rip_check(dep_string = bz_scalar_value_get(dep_value, state->ctx));
    rip_check(dep = bz_dependency_from_string(dep_string));
    rip_check(dep_package = bz_satisfy_dependency(dep, state->ctx));
    dep_env = bz_package_env(dep_package);
    rip_check(dep_name = bz_env_get_string(dep_env, "native_name", true));
    if (state->first) {
        cork_buffer_append(&state->dep_buf, "'", 1);
        state->first = false;
    } else {
        cork_buffer_append(&state->dep_buf, " '", 2);
    }
    cork_buffer_append_string(&state->dep_buf, dep_name);
    if (dep->min_version != NULL) {
        cork_buffer_append(&state->dep_buf, ">=", 2);
        bz_version_to_arch(dep->min_version, &state->dep_buf);
    }
    cork_buffer_append(&state->dep_buf, "'", 1);
    bz_dependency_free(dep);
    return 0;
}

static int
bz_pacman_fill_deps(struct bz_env *env, struct cork_buffer *buf,
                    const char *pkgbuild_name, const char *var_name)
{
    struct bz_value  *deps_value;
    rie_check(deps_value = bz_env_get_value(env, var_name));
    if (deps_value != NULL) {
        int  rc;
        struct bz_pacman_fill_deps  state = {
            bz_env_as_value(env),
            CORK_BUFFER_INIT(),
            true
        };
        rc = bz_array_value_map_scalars
            (deps_value, &state, bz_pacman_fill_one_dep);
        if (rc == 0 && state.dep_buf.size > 0) {
            cork_buffer_append_printf
                (buf, "%s=(%s)\n", pkgbuild_name, (char *) state.dep_buf.buf);
        }
        cork_buffer_done(&state.dep_buf);
        return rc;
    }
    return 0;
}

static int
bz_pacman_add_install_script(struct bz_env *env, struct cork_buffer *buf,
                             const char *var_name, const char *func_name)
{
    struct cork_path  *install_script;
    rie_check(install_script = bz_env_get_path(env, var_name, false));
    if (install_script != NULL) {
        cork_buffer_append_printf(buf, "%s () {\n", func_name);
        rii_check(bz_load_file(cork_path_get(install_script), buf));
        cork_buffer_append_string(buf, "\n}\n");
    }
    return 0;
}

static int
bz_pacman_add_install_scripts(struct bz_env *env,
                              struct cork_buffer *pkgbuild_buf)
{
    struct cork_buffer  install_buf = CORK_BUFFER_INIT();

    /* Copy each kind of script into install_buf, if present. */
    rii_check(bz_pacman_add_install_script
              (env, &install_buf, "pre_install_script", "pre_install"));
    rii_check(bz_pacman_add_install_script
              (env, &install_buf, "post_install_script", "post_install"));

    /* If any of them added content to the install script, save that to a file
     * and reference it in the PKGBUILD. */
    if (install_buf.size > 0) {
        struct cork_path  *install;
        const char  *install_base;
        rip_check(install = bz_env_get_path(env, "pacman.install", true));
        rip_check(install_base =
                  bz_env_get_string(env, "pacman.install_base", true));
        rii_check(bz_create_file(cork_path_get(install), &install_buf));
        cork_buffer_append_printf(pkgbuild_buf, "install=%s\n", install_base);
    }

    cork_buffer_done(&install_buf);
    return 0;
}

static int
bz_pacman__package(void *user_data)
{
    struct bz_env  *env = user_data;
    struct bz_value  *ctx = bz_env_as_value(env);
    struct cork_path  *staging_dir;
    struct cork_path  *binary_package_dir;
    struct cork_path  *package_build_dir;
    struct cork_path  *pkgbuild;
    struct cork_path  *package_file;
    const char  *package_name;
    const char  *version;
    const char  *pkgrel;
    const char  *pkgext;
    const char  *architecture;
    const char  *license;
    bool  verbose;

    struct cork_env  *exec_env;
    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    bool  staging_exists;

    rii_check(bz_install_dependency_string("pacman", ctx));
    rii_check(bz_package_message(env, "pacman"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Package using pacman", package_name);

    rip_check(staging_dir = bz_env_get_path(env, "staging_dir", true));
    rip_check(binary_package_dir =
              bz_env_get_path(env, "binary_package_dir", true));
    rip_check(package_build_dir =
              bz_env_get_path(env, "package_build_dir", true));
    rip_check(pkgbuild = bz_env_get_path(env, "pacman.pkgbuild", true));
    rip_check(package_file = bz_env_get_path(env, "pacman.package_file", true));
    rip_check(version = bz_env_get_string(env, "pacman.version", true));
    rip_check(pkgrel = bz_env_get_string(env, "pacman.pkgrel", true));
    rip_check(pkgext = bz_env_get_string(env, "pacman.pkgext", true));
    rip_check(architecture = bz_env_get_string(env, "pacman.arch", true));
    rip_check(license = bz_env_get_string(env, "license", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));

    rii_check(bz_file_exists(cork_path_get(staging_dir), &staging_exists));
    if (CORK_UNLIKELY(!staging_exists)) {
        cork_error_set_printf
            (ENOENT, "Staging directory %s does not exist",
             cork_path_get(staging_dir));
        return -1;
    }

    /* Create the temporary directory and the packaging destination */
    rii_check(bz_create_directory(cork_path_get(package_build_dir), 0750));
    rii_check(bz_create_directory(cork_path_get(binary_package_dir), 0750));

    /* Create a PKGBUILD file for this package */
    cork_buffer_append_printf(&buf, "pkgname='%s'\n", package_name);
    cork_buffer_append_printf(&buf, "pkgver='%s'\n", version);
    cork_buffer_append_printf(&buf, "pkgrel='%s'\n", pkgrel);
    cork_buffer_append_printf(&buf, "arch=('%s')\n", architecture);
    cork_buffer_append_printf(&buf, "license=('%s')\n", license);
    rii_check(bz_pacman_fill_deps(env, &buf, "depends", "dependencies"));
    cork_buffer_append_printf(&buf,
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '%s' \"${pkgdir}\"\n"
        "}\n",
        cork_path_get(staging_dir)
    );

    /* Add pre- and post-install scripts, if necessary. */
    rii_check(bz_pacman_add_install_scripts(env, &buf));

    ei_check(bz_create_file(cork_path_get(pkgbuild), &buf));
    cork_buffer_done(&buf);

    exec_env = cork_env_clone_current();
    cork_env_add(exec_env, "PKGDEST", cork_path_get(binary_package_dir));
    cork_env_add(exec_env, "PKGEXT", pkgext);
    exec = cork_exec_new_with_params("makepkg", "-sf", NULL);
    cork_exec_set_cwd(exec, cork_path_get(package_build_dir));
    cork_exec_set_env(exec, exec_env);
    clog_info("(%s) Create %s using pacman",
              package_name, cork_path_get(package_file));
    return bz_subprocess_run_exec(verbose, NULL, exec);

error:
    cork_buffer_done(&buf);
    return -1;
}


static int
bz_pacman__install__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    struct bz_value  *ctx = bz_env_as_value(env);
    const char  *package_name;
    struct bz_version  *package_version;
    bool  force;

    rii_check(bz_install_dependency_string("pacman", ctx));
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rie_check(force = bz_env_get_bool(env, "force", true));

    if (force) {
        *is_needed = true;
        clog_info("(%s) Force installation of package",
                  package_name);
        return 0;
    } else {
        struct bz_version  *installed;

        clog_info("(%s) Check whether pacman package is installed",
                  package_name);
        rip_check(package_version = bz_env_get_version(env, "version", true));
        ee_check(installed = bz_arch_native_version_installed(package_name));

        if (installed == NULL) {
            clog_info("(%s) Package is not installed", package_name);
            *is_needed = true;
            return 0;
        } else {
            *is_needed = (bz_version_cmp(installed, package_version) < 0);
            clog_info("(%s) Installed version %s is %s than %s",
                      package_name,
                      bz_version_to_string(installed),
                      *is_needed? "older": "newer",
                      bz_version_to_string(package_version));
            bz_version_free(installed);
            return 0;
        }
    }

error:
    bz_version_free(package_version);
    return -1;
}

static int
bz_pacman__install(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *package_file;

    rii_check(bz_install_message(env, "pacman"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(package_file = bz_env_get_path(env, "pacman.package_file", true));
    clog_info("(%s) Install %s using pacman",
              package_name, cork_path_get(package_file));
    return bz_subprocess_run
        (false, NULL,
         "sudo", "pacman", "-U", "--noconfirm", cork_path_get(package_file),
         NULL);
}


static int
bz_pacman__uninstall__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    struct bz_value  *ctx = bz_env_as_value(env);
    const char  *package_name;
    struct bz_version  *installed;
    rii_check(bz_install_dependency_string("pacman", ctx));
    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Check whether pacman package is installed",
              package_name);
    rie_check(installed = bz_arch_native_version_installed(package_name));
    if (installed == NULL) {
        clog_info("(%s) Package is not installed", package_name);
        *is_needed = false;
    } else {
        clog_info("(%s) Version %s is installed",
                  package_name, bz_version_to_string(installed));
        *is_needed = true;
        bz_version_free(installed);
    }
    return 0;
}

static int
bz_pacman__uninstall(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;

    rii_check(bz_uninstall_message(env, "pacman"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Uninstall using pacman", package_name);
    return bz_subprocess_run
        (false, NULL,
         "sudo", "pacman", "-R", "--noconfirm", package_name,
         NULL);
}


struct bz_packager *
bz_pacman_packager_new(struct bz_env *env)
{
    return bz_packager_new
        (env, "pacman", env, NULL,
         bz_pacman__package__is_needed, bz_pacman__package,
         bz_pacman__install__is_needed, bz_pacman__install,
         bz_pacman__uninstall__is_needed, bz_pacman__uninstall);
}
