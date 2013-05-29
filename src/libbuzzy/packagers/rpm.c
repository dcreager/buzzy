/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
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

#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/value.h"
#include "buzzy/distro/rpm.h"

#define CLOG_CHANNEL  "rpm"


/*-----------------------------------------------------------------------
 * RPM version values
 */

static const char *
bz_rpm_full_version__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    struct bz_version  *version;
    rpp_check(version = bz_value_get_version(ctx, "version", true));
    cork_buffer_clear(buf);
    bz_version_to_rpm(version, buf);
    return buf->buf;
}

static struct bz_value *
bz_rpm_full_version_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, (cork_free_f) cork_buffer_free, bz_rpm_full_version__get);
}


static const char *
bz_rpm_version__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    const char  *version;
    const char  *dash;
    rpp_check(version = bz_value_get_string(ctx, "rpm.version", true));
    dash = strchr(version, '-');
    if (dash == NULL) {
        bz_bad_config("Invalid RPM version %s", version);
        return NULL;
    }
    cork_buffer_set(buf, version, dash - version);
    return buf->buf;
}

static struct bz_value *
bz_rpm_version_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, (cork_free_f) cork_buffer_free, bz_rpm_version__get);
}


static const char *
bz_rpm_release__get(void *user_data, struct bz_value *ctx)
{
    const char  *version;
    const char  *dash;
    rpp_check(version = bz_value_get_string(ctx, "rpm.version", true));
    dash = strchr(version, '-');
    return dash + 1;
}

static struct bz_value *
bz_rpm_release_value_new(void)
{
    return bz_scalar_value_new(NULL, NULL, bz_rpm_release__get);
}


/*-----------------------------------------------------------------------
 * Builtin rpm variables
 */

bz_define_variables(rpm)
{
    bz_package_variable(
        package_file_base, "rpm.package_file_base",
        bz_interpolated_value_new("${name}-${rpm.version}.${rpm.arch}.rpm"),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        package_file, "rpm.package_file",
        bz_interpolated_value_new
            ("${binary_package_dir}/${rpm.package_file_base}"),
        "The filename for any package that we create",
        ""
    );

    bz_package_variable(
        architecture, "rpm.spec_file",
        bz_interpolated_value_new("${package_build_dir}/${name}.spec"),
        "The location of the RPM spec file we should create",
        ""
    );

    bz_package_variable(
        architecture, "rpm.arch",
        bz_interpolated_value_new("${arch}"),
        "The architecture to build rpm packages for",
        ""
    );

    bz_package_variable(
        full_version, "rpm.version",
        bz_rpm_full_version_value_new(),
        "The RPM equivalent of the package's version",
        ""
    );

    bz_package_variable(
        version, "rpm.version_v",
        bz_rpm_version_value_new(),
        "The Version portion of the package's RPM version",
        ""
    );

    bz_package_variable(
        release, "rpm.version_r",
        bz_rpm_release_value_new(),
        "The Release portion of the package's RPM version",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Creating rpm packages
 */

static int
bz_rpm__package__is_needed(void *user_data, bool *is_needed)
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
                  (env, "rpm.package_file", true));
        clog_info("(%s) Check whether binary package %s exists",
                  package_name, cork_path_get(package_file));
        rii_check(bz_file_exists(cork_path_get(package_file), is_needed));
        *is_needed = !*is_needed;
        return 0;
    }
}

struct bz_rpm_fill_deps {
    struct bz_value  *ctx;
    struct cork_buffer  dep_buf;
    bool  first;
};

static int
bz_rpm_fill_one_dep(void *user_data, struct bz_value *dep_value)
{
    struct bz_rpm_fill_deps  *state = user_data;
    const char  *dep_string;
    struct bz_dependency  *dep;
    rip_check(dep_string = bz_scalar_value_get(dep_value, state->ctx));
    rip_check(dep = bz_dependency_from_string(dep_string));
    if (state->first) {
        state->first = false;
    } else {
        cork_buffer_append(&state->dep_buf, ", ", 2);
    }
    cork_buffer_append_string(&state->dep_buf, dep->package_name);
    if (dep->min_version != NULL) {
        cork_buffer_append(&state->dep_buf, ">=", 2);
        bz_version_to_rpm(dep->min_version, &state->dep_buf);
    }
    bz_dependency_free(dep);
    return 0;
}

static int
bz_rpm_fill_deps(struct bz_env *env, struct cork_buffer *buf,
                 const char *spec_name, const char *var_name)
{
    struct bz_value  *deps_value;
    rie_check(deps_value = bz_env_get_value(env, var_name));
    if (deps_value != NULL) {
        int  rc;
        struct bz_rpm_fill_deps  state = {
            bz_env_as_value(env),
            CORK_BUFFER_INIT(),
            true
        };
        rc = bz_array_value_map_scalars
            (deps_value, &state, bz_rpm_fill_one_dep);
        if (rc == 0 && state.dep_buf.size > 0) {
            cork_buffer_append_printf
                (buf, "%s: %s\n", spec_name, (char *) state.dep_buf.buf);
        }
        cork_buffer_done(&state.dep_buf);
        return rc;
    }
    return 0;
}

struct bz_rpm_spec_files {
    struct cork_dir_walker  parent;
    struct cork_buffer  *buf;
};

static int
bz_rpm_spec_files__file(struct cork_dir_walker *walker, const char *full_path,
                        const char *rel_path, const char *base_name)
{
    struct bz_rpm_spec_files  *state =
        cork_container_of(walker, struct bz_rpm_spec_files, parent);
    struct stat  info;
    rii_check_posix(stat(full_path, &info));
    cork_buffer_append_printf
        (state->buf, "%%attr(0%o,-,-) /%s\n", info.st_mode & 0x7fff, rel_path);
    return 0;
}

static int
bz_rpm_spec_files__enter(struct cork_dir_walker *walker, const char *full_path,
                         const char *rel_path, const char *base_name)
{
    return 0;
}

static int
bz_rpm_spec_files__leave(struct cork_dir_walker *walker, const char *full_path,
                         const char *rel_path, const char *base_name)
{
    return 0;
}

static int
bz_rpm__package(void *user_data)
{
    struct bz_env  *env = user_data;
    struct cork_path  *staging_dir;
    struct cork_path  *binary_package_dir;
    struct cork_path  *package_build_dir;
    struct cork_path  *spec_file;
    struct cork_path  *package_file;
    const char  *package_name;
    const char  *version_v;
    const char  *version_r;
    const char  *license;
    bool  verbose;

    struct cork_exec  *exec;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    struct cork_buffer  param = CORK_BUFFER_INIT();
    bool  staging_exists;

    struct bz_rpm_spec_files  files;

    rii_check(bz_install_dependency_string("rpm-build", env));
    rii_check(bz_package_message(env, "RPM"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Package using RPM", package_name);

    rip_check(staging_dir = bz_env_get_path(env, "staging_dir", true));
    rip_check(binary_package_dir =
              bz_env_get_path(env, "binary_package_dir", true));
    rip_check(package_build_dir =
              bz_env_get_path(env, "package_build_dir", true));
    rip_check(spec_file = bz_env_get_path(env, "rpm.spec_file", true));
    rip_check(package_file = bz_env_get_path(env, "rpm.package_file", true));
    rip_check(version_v = bz_env_get_string(env, "rpm.version_v", true));
    rip_check(version_r = bz_env_get_string(env, "rpm.version_r", true));
    rip_check(license = bz_env_get_string(env, "license", true));
    rie_check(verbose = bz_env_get_bool(env, "verbose", true));

    rii_check(bz_file_exists(cork_path_get(staging_dir), &staging_exists));
    if (CORK_UNLIKELY(!staging_exists)) {
        cork_error_set
            (CORK_BUILTIN_ERROR, CORK_SYSTEM_ERROR,
             "Staging directory %s does not exist", cork_path_get(staging_dir));
        return -1;
    }

    /* Create the temporary directory and the packaging destination */
    rii_check(bz_create_directory(cork_path_get(package_build_dir)));
    rii_check(bz_create_directory(cork_path_get(binary_package_dir)));

    /* Create an RPM spec file for this package */
    cork_buffer_append_printf(&buf, "Summary: %s\n", package_name);
    cork_buffer_append_printf(&buf, "Name: %s\n", package_name);
    cork_buffer_append_printf(&buf, "Version: %s\n", version_v);
    cork_buffer_append_printf(&buf, "Release: %s\n", version_r);
    cork_buffer_append_printf(&buf, "License: %s\n", license);
    cork_buffer_append_printf(&buf, "Group: Buzzy\n");
    cork_buffer_append_printf
        (&buf, "Source: %s\n", cork_path_get(binary_package_dir));
    cork_buffer_append_printf
        (&buf, "BuildRoot: %s\n", cork_path_get(staging_dir));
    rii_check(bz_rpm_fill_deps(env, &buf, "Requires", "dependencies"));
    cork_buffer_append_printf(&buf,
        "\n"
        "%%description\n"
        "No package description\n"
        "\n"
        "%%prep\n"
        "\n"
        "%%clean\n"
    );

    /* Add the file manifest to the spec file */
    cork_buffer_append_printf(&buf,
        "\n"
        "%%files\n"
    );
    files.parent.file = bz_rpm_spec_files__file;
    files.parent.enter_directory = bz_rpm_spec_files__enter;
    files.parent.leave_directory = bz_rpm_spec_files__leave;
    files.buf = &buf;
    ei_check(bz_walk_directory(cork_path_get(staging_dir), &files.parent));

    ei_check(bz_create_file(cork_path_get(spec_file), &buf));
    cork_buffer_done(&buf);

    exec = cork_exec_new("rpmbuild");
    cork_exec_add_param(exec, "rpmbuild");
    cork_exec_add_param(exec, "--define");
    cork_exec_add_param(exec, "_sourcedir .");
    cork_exec_add_param(exec, "--define");
    cork_buffer_printf(&param, "_rpmdir %s", cork_path_get(binary_package_dir));
    cork_exec_add_param(exec, param.buf);
    cork_exec_add_param(exec, "--define");
    cork_exec_add_param(exec, "_builddir .");
    cork_exec_add_param(exec, "--define");
    cork_buffer_printf(&param, "buildroot %s", cork_path_get(staging_dir));
    cork_exec_add_param(exec, param.buf);
    cork_exec_add_param(exec, "--define");
    cork_exec_add_param(exec, "_srcrpmdir .");
    cork_exec_add_param(exec, "--define");
    cork_exec_add_param(exec, "_specdir .");
    cork_exec_add_param(exec, "--define");
    cork_exec_add_param
        (exec, "_build_name_fmt %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm");
    cork_exec_add_param(exec, "--quiet");
    cork_exec_add_param(exec, "-bb");
    cork_exec_add_param(exec, cork_path_get(spec_file));
    cork_exec_set_cwd(exec, cork_path_get(package_build_dir));
    clog_info("(%s) Create %s using RPM",
              package_name, cork_path_get(package_file));
    cork_buffer_done(&param);
    return bz_subprocess_run_exec(verbose, NULL, exec);

error:
    cork_buffer_done(&buf);
    cork_buffer_done(&param);
    return -1;
}


static int
bz_rpm__install__is_needed(void *user_data, bool *is_needed)
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

        clog_info("(%s) Check whether RPM package is installed",
                  package_name);
        rip_check(package_version = bz_env_get_version(env, "version", true));
        ee_check(installed = bz_rpm_native_version_installed(package_name));

        if (installed == NULL) {
            *is_needed = true;
            return 0;
        } else {
            *is_needed = (bz_version_cmp(installed, package_version) < 0);
            bz_version_free(installed);
            return 0;
        }
    }

error:
    bz_version_free(package_version);
    return -1;
}

static int
bz_rpm__install(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct cork_path  *package_file;

    rii_check(bz_install_message(env, "RPM"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(package_file = bz_env_get_path(env, "rpm.package_file", true));
    clog_info("(%s) Install %s using RPM",
              package_name, cork_path_get(package_file));
    return bz_subprocess_run
        (false, NULL,
         "sudo", "rpm", "-U", cork_path_get(package_file),
         NULL);
}


static int
bz_rpm__uninstall__is_needed(void *user_data, bool *is_needed)
{
    struct bz_env  *env = user_data;
    const char  *package_name;
    struct bz_version  *installed;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Check whether RPM package is installed",
              package_name);
    rie_check(installed = bz_rpm_native_version_installed(package_name));
    if (installed == NULL) {
        *is_needed = false;
    } else {
        *is_needed = true;
        bz_version_free(installed);
    }
    return 0;
}

static int
bz_rpm__uninstall(void *user_data)
{
    struct bz_env  *env = user_data;
    const char  *package_name;

    rii_check(bz_uninstall_message(env, "rpm"));

    rip_check(package_name = bz_env_get_string(env, "name", true));
    clog_info("(%s) Uninstall using RPM", package_name);
    return bz_subprocess_run
        (false, NULL,
         "sudo", "rpm", "-e", package_name,
         NULL);
}


struct bz_packager *
bz_rpm_packager_new(struct bz_env *env)
{
    return bz_packager_new
        (env, "rpm", env, NULL,
         bz_rpm__package__is_needed, bz_rpm__package,
         bz_rpm__install__is_needed, bz_rpm__install,
         bz_rpm__uninstall__is_needed, bz_rpm__uninstall);
}