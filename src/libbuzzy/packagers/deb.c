/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013-2014, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>
#include <libcork/helpers/posix.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/value.h"
#include "buzzy/distro/debian.h"

#define CLOG_CHANNEL  "deb"


/*-----------------------------------------------------------------------
 * Debian version values
 */

static const char *
bz_deb_version__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    struct bz_version  *version;
    rpp_check(version = bz_value_get_version(ctx, "version", true));
    cork_buffer_clear(buf);
    bz_version_to_deb(version, buf);
    return buf->buf;
}

static struct bz_value *
bz_deb_version_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, (cork_free_f) cork_buffer_free, bz_deb_version__get);
}


/*-----------------------------------------------------------------------
 * Current architecture
 */

static struct cork_buffer  architecture = CORK_BUFFER_INIT();

static void
done_architecture(void)
{
    cork_buffer_done(&architecture);
}

CORK_INITIALIZER(init_architecture)
{
    cork_cleanup_at_exit(0, done_architecture);
}

const char *
bz_deb_current_architecture(void)
{
    char  *buf;
    cork_buffer_clear(&architecture);
    rpi_check(bz_subprocess_get_output
              (&architecture, NULL, NULL,
               "dpkg-architecture", "-qDEB_HOST_ARCH", NULL));
    /* Chomp the trailing newline */
    buf = architecture.buf;
    buf[--architecture.size] = '\0';
    return architecture.buf;
}


static const char *
bz_deb_architecture_value__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    if (buf->size == 0) {
        const char  *arch;
        rpp_check(arch = bz_deb_current_architecture());
        cork_buffer_set_string(buf, arch);
    }
    return buf->buf;
}

struct bz_value *
bz_deb_architecture_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, (cork_free_f) cork_buffer_free, bz_deb_architecture_value__get);
}


/*-----------------------------------------------------------------------
 * Builtin deb variables
 */

bz_define_variables(deb)
{
    bz_package_variable(
        package_file_base, "deb.package_file_base",
        bz_interpolated_value_new("${name}_${deb.version}_${deb.arch}.deb"),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        package_file, "deb.package_file",
        bz_interpolated_value_new
            ("${binary_package_dir}/${deb.package_file_base}"),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        architecture, "deb.debian_dir",
        bz_interpolated_value_new("${staging_dir}/DEBIAN"),
        "The location of the DEBIAN control directory",
        ""
    );

    bz_package_variable(
        architecture, "deb.control_file",
        bz_interpolated_value_new("${deb.debian_dir}/control"),
        "The location of the Debian control file we should create",
        ""
    );

    bz_package_variable(
        architecture, "deb.preinst_script",
        bz_interpolated_value_new("${deb.debian_dir}/preinst"),
        "The location of the Debian preinst script we should create",
        ""
    );

    bz_package_variable(
        architecture, "deb.postinst_script",
        bz_interpolated_value_new("${deb.debian_dir}/postinst"),
        "The location of the Debian postinst script we should create",
        ""
    );

    bz_package_variable(
        architecture, "deb.prerm_script",
        bz_interpolated_value_new("${deb.debian_dir}/prerm"),
        "The location of the Debian prerm script we should create",
        ""
    );

    bz_package_variable(
        architecture, "deb.postrm_script",
        bz_interpolated_value_new("${deb.debian_dir}/postrm"),
        "The location of the Debian postrm script we should create",
        ""
    );

    bz_package_variable(
        architecture, "deb.arch",
        bz_deb_architecture_value_new(),
        "The architecture to build deb packages for",
        ""
    );

    bz_package_variable(
        version, "deb.version",
        bz_deb_version_value_new(),
        "The Debian equivalent of the package's version",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Creating deb packages
 */

static int
bz_deb__package__is_needed(void *user_data, bool *is_needed)
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
                  (env, "deb.package_file", true));
        clog_info("(%s) Check whether binary package %s exists",
                  package_name, cork_path_get(package_file));
        rii_check(bz_file_exists(cork_path_get(package_file), is_needed));
        *is_needed = !*is_needed;
        return 0;
    }
}

struct bz_deb_fill_deps {
    struct bz_value  *ctx;
    struct cork_buffer  dep_buf;
    bool  first;
};

static int
bz_deb_fill_one_dep(void *user_data, struct bz_value *dep_value)
{
    struct bz_deb_fill_deps  *state = user_data;
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
        state->first = false;
    } else {
        cork_buffer_append(&state->dep_buf, ", ", 2);
    }
    cork_buffer_append_string(&state->dep_buf, dep_name);
    if (dep->min_version != NULL) {
        cork_buffer_append(&state->dep_buf, " (>= ", 5);
        bz_version_to_deb(dep->min_version, &state->dep_buf);
        cork_buffer_append(&state->dep_buf, ")", 1);
    }
    bz_dependency_free(dep);
    return 0;
}

static int
bz_deb_fill_deps(struct bz_env *env, struct cork_buffer *buf,
                 const char *control_name, const char *var_name)
{
    struct bz_value  *deps_value;
    rie_check(deps_value = bz_env_get_value(env, var_name));
    if (deps_value != NULL) {
        int  rc;
        struct bz_deb_fill_deps  state = {
            bz_env_as_value(env),
            CORK_BUFFER_INIT(),
            true
        };
        rc = bz_array_value_map_scalars
            (deps_value, &state, bz_deb_fill_one_dep);
        if (rc == 0 && state.dep_buf.size > 0) {
            cork_buffer_append_printf
                (buf, "%s: %s\n", control_name, (char *) state.dep_buf.buf);
        }
        cork_buffer_done(&state.dep_buf);
        return rc;
    }
    return 0;
}

static int
bz_deb_add_install_script(struct bz_env *env, struct cork_buffer *buf,
                          const char *var_name)
{
    struct cork_path  *install_script;
    rie_check(install_script = bz_env_get_path(env, var_name, false));
    if (install_script != NULL) {
        clog_debug("Load %s script from %s",
                   var_name, cork_path_get(install_script));
        rii_check(bz_load_file(cork_path_get(install_script), buf));
        cork_buffer_append_string(buf, "\n");
    }
    return 0;
}

static int
bz_deb_add_script_to_package(struct bz_env *env, struct cork_buffer *script,
                             const char *var_name)
{
    if (script->size > 0) {
        struct cork_path  *deb_script;
        rip_check(deb_script = bz_env_get_path(env, var_name, true));
        clog_debug("Create %s script %s", var_name, cork_path_get(deb_script));
        return bz_create_file(cork_path_get(deb_script), script, 0755);
    }
    return 0;
}

static int
bz_deb__package(void *user_data)
{
    struct bz_env  *env = user_data;
    struct cork_path  *staging_dir;
    struct cork_path  *debian_dir;
    struct cork_path  *binary_package_dir;
    struct cork_path  *package_build_dir;
    struct cork_path  *control_file;
    struct cork_path  *package_file;
    const char  *package_name;
    const char  *version;
    const char  *license;
    const char  *deb_arch;
    bool  verbose;

    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    struct cork_buffer  preinst = CORK_BUFFER_INIT();
    struct cork_buffer  postinst = CORK_BUFFER_INIT();
    struct cork_buffer  prerm = CORK_BUFFER_INIT();
    struct cork_buffer  postrm = CORK_BUFFER_INIT();
    struct cork_buffer  param = CORK_BUFFER_INIT();
    bool  staging_exists;

    rii_check(bz_package_message(env, "Debian"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Package using Debian", package_name);

    rip_check(staging_dir = bz_env_get_path(env, "staging_dir", true));
    rip_check(debian_dir = bz_env_get_path(env, "deb.debian_dir", true));
    rip_check(binary_package_dir =
              bz_env_get_path(env, "binary_package_dir", true));
    rip_check(package_build_dir =
              bz_env_get_path(env, "package_build_dir", true));
    rip_check(control_file = bz_env_get_path(env, "deb.control_file", true));
    rip_check(package_file = bz_env_get_path(env, "deb.package_file", true));
    rip_check(version = bz_env_get_string(env, "deb.version", true));
    rip_check(license = bz_env_get_string(env, "license", true));
    rip_check(deb_arch = bz_env_get_string(env, "deb.arch", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));

    rii_check(bz_file_exists(cork_path_get(staging_dir), &staging_exists));
    if (CORK_UNLIKELY(!staging_exists)) {
        cork_error_set_printf
            (ENOENT, "Staging directory %s does not exist",
             cork_path_get(staging_dir));
        goto error;
    }

    /* For now, always call ldconfig in a post-install script, regardless of
     * whether the package includes any shared libraries. */
    cork_buffer_append_literal(&postinst, "/sbin/ldconfig\n");
    cork_buffer_append_literal(&postrm, "/sbin/ldconfig\n");

    /* Create the temporary directory and the packaging destination */
    rii_check(bz_create_directory(cork_path_get(debian_dir), 0755));
    rii_check(bz_create_directory(cork_path_get(package_build_dir), 0750));
    rii_check(bz_create_directory(cork_path_get(binary_package_dir), 0750));

    /* Create an Debian control file for this package */
    cork_buffer_append_printf(&buf, "Package: %s\n", package_name);
    cork_buffer_append_printf(&buf, "Description: %s\n", package_name);
    cork_buffer_append_printf
        (&buf, "Maintainer: Unknown <unknown@unknown.org>\n");
    cork_buffer_append_printf(&buf, "Version: %s\n", version);
    cork_buffer_append_printf(&buf, "Section: Miscellaneous\n");
    cork_buffer_append_printf(&buf, "Priority: optional\n");
    cork_buffer_append_printf(&buf, "Architecture: %s\n", deb_arch);
    rii_check(bz_deb_fill_deps(env, &buf, "Depends", "dependencies"));

    /* Add pre- and post-install scripts, if necessary. */
    rii_check(bz_deb_add_install_script(env, &preinst, "pre_install_script"));
    rii_check(bz_deb_add_install_script(env, &postinst, "post_install_script"));
    rii_check(bz_deb_add_install_script(env, &prerm, "pre_remove_script"));
    rii_check(bz_deb_add_install_script(env, &postrm, "post_remove_script"));

    /* Create the control file */
    ei_check(bz_create_file(cork_path_get(control_file), &buf, 0640));

    /* And any installation scripts that have content. */
    bz_deb_add_script_to_package(env, &preinst, "deb.preinst_script");
    bz_deb_add_script_to_package(env, &postinst, "deb.postinst_script");
    bz_deb_add_script_to_package(env, &prerm, "deb.prerm_script");
    bz_deb_add_script_to_package(env, &postrm, "deb.postrm_script");

    cork_buffer_done(&buf);
    cork_buffer_done(&preinst);
    cork_buffer_done(&postinst);
    cork_buffer_done(&prerm);
    cork_buffer_done(&postrm);

    exec = cork_exec_new("dpkg-deb");
    cork_exec_add_param(exec, "dpkg-deb");
    cork_exec_add_param(exec, "-b");
    cork_exec_add_param(exec, cork_path_get(staging_dir));
    cork_exec_add_param(exec, cork_path_get(package_file));
    clog_info("(%s) Create %s using Debian",
              package_name, cork_path_get(package_file));
    cork_buffer_done(&param);
    return bz_subprocess_run_exec(verbose, NULL, exec);

error:
    cork_buffer_done(&buf);
    cork_buffer_done(&preinst);
    cork_buffer_done(&postinst);
    cork_buffer_done(&prerm);
    cork_buffer_done(&postrm);
    cork_buffer_done(&param);
    return -1;
}


static int
bz_deb__install__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct bz_version  *package_version;
    bool  force;

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rie_check(force = bz_env_get_bool(env, "force", true));

    if (force) {
        *is_needed = true;
        clog_info("(%s) Force installation of package",
                  package_name);
        return 0;
    } else {
        struct bz_version  *installed;

        clog_info("(%s) Check whether Debian package is installed",
                  package_name);
        rip_check(package_version = bz_env_get_version(env, "version", true));
        ee_check(installed = bz_deb_native_version_installed(package_name));

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
bz_deb__install(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *package_file;

    rii_check(bz_install_message(env, "Debian"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(package_file = bz_env_get_path(env, "deb.package_file", true));
    clog_info("(%s) Install %s using Debian",
              package_name, cork_path_get(package_file));
    return bz_subprocess_run
        (false, NULL,
         "sudo", "dpkg", "-i", cork_path_get(package_file),
         NULL);
}


static int
bz_deb__uninstall__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct bz_version  *installed;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Check whether Debian package is installed",
              package_name);
    rie_check(installed = bz_deb_native_version_installed(package_name));
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
bz_deb__uninstall(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;

    rii_check(bz_uninstall_message(env, "deb"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Uninstall using Debian", package_name);
    return bz_subprocess_run
        (false, NULL,
         "sudo", "dpkg", "-r", package_name,
         NULL);
}


struct bz_packager *
bz_deb_packager_new(struct bz_env *env)
{
    return bz_packager_new
        (env, "deb", env, NULL,
         bz_deb__package__is_needed, bz_deb__package,
         bz_deb__install__is_needed, bz_deb__install,
         bz_deb__uninstall__is_needed, bz_deb__uninstall);
}
