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
 * Homebrew pkg-config support
 */

/* We set the PKG_CONFIG_PATH environment variable to look in each formula's
 * /usr/local/opt directory for pkgconfig files.  This works for both regular
 * and keg-only formulas, since in both cases, Homebrew will create pkgconfig
 * files that reference headers and libraries directly in the installation
 * cellar. */

struct bz_homebrew_pkgconfig {
    struct cork_hash_table  *packages;
    struct cork_buffer  buf;
};

static void
bz_homebrew_pkgconfig_path__free(void *user_data)
{
    struct bz_homebrew_pkgconfig  *self = user_data;
    cork_hash_table_free(self->packages);
    cork_buffer_done(&self->buf);
    free(self);
}

static int
bz_pkgconfig_process_package_list(struct bz_homebrew_pkgconfig *self,
                                  struct bz_package_list *list);

static int
bz_pkgconfig_process_package(struct bz_homebrew_pkgconfig *self,
                             struct bz_package *package)
{
    bool  is_new;
    const char  *package_name = bz_package_name(package);
    cork_hash_table_put
        (self->packages, (void *) package_name, NULL, &is_new, NULL, NULL);
    if (is_new) {
        struct bz_package_list  *deps;
        rip_check(deps = bz_package_build_deps(package));
        rii_check(bz_pkgconfig_process_package_list(self, deps));
        rip_check(deps = bz_package_deps(package));
        rii_check(bz_pkgconfig_process_package_list(self, deps));
    }
    return 0;
}

static int
bz_pkgconfig_process_package_list(struct bz_homebrew_pkgconfig *self,
                                  struct bz_package_list *list)
{
    size_t  i;
    size_t  count;
    count = bz_package_list_count(list);
    for (i = 0; i < count; i++) {
        struct bz_package  *package = bz_package_list_get(list, i);
        rii_check(bz_pkgconfig_process_package(self, package));
    }
    return 0;
}

static enum cork_hash_table_map_result
bz_pkgconfig_add_one_path(void *user_data, struct cork_hash_table_entry *entry)
{
    struct bz_homebrew_pkgconfig  *self = user_data;
    const char  *package_name = entry->key;
    if (self->buf.size != 0) {
        cork_buffer_append(&self->buf, ":", 1);
    }
    cork_buffer_append_printf
        (&self->buf, "/usr/local/opt/%s/lib/pkgconfig", package_name);
    return CORK_HASH_TABLE_MAP_DELETE;
}

/* Clears the hash table as a result */
static void
bz_pkgconfig_construct_path(struct bz_homebrew_pkgconfig *self)
{
    cork_hash_table_map(self->packages, self, bz_pkgconfig_add_one_path);
}


struct bz_pkgconfig_process_deps {
    struct bz_value  *ctx;
    struct bz_homebrew_pkgconfig  *self;
};

static int
bz_pkgconfig_process_dep(void *user_data, struct bz_value *dep_value)
{
    struct bz_pkgconfig_process_deps  *state = user_data;
    const char  *dep_string;
    struct bz_package  *package;
    rip_check(dep_string = bz_scalar_value_get(dep_value, state->ctx));
    rip_check(package = bz_satisfy_dependency_string(dep_string, state->ctx));
    return bz_pkgconfig_process_package(state->self, package);
}

static int
bz_pkgconfig_process_deps(struct bz_homebrew_pkgconfig *self,
                          struct bz_value *ctx, const char *var_name)
{
    struct bz_value  *deps_value;
    rie_check(deps_value = bz_value_get_nested(ctx, var_name));
    if (deps_value == NULL) {
        return 0;
    } else {
        struct bz_pkgconfig_process_deps  state = { ctx, self };
        return bz_array_value_map_scalars
            (deps_value, &state, bz_pkgconfig_process_dep);
    }
}

static const char *
bz_homebrew_pkgconfig_path__get(void *user_data, struct bz_value *ctx)
{
    struct bz_homebrew_pkgconfig  *self = user_data;
    cork_hash_table_clear(self->packages);
    cork_buffer_clear(&self->buf);
    rpi_check(bz_pkgconfig_process_deps(self, ctx, "build_dependencies"));
    rpi_check(bz_pkgconfig_process_deps(self, ctx, "dependencies"));
    bz_pkgconfig_construct_path(self);
    return self->buf.buf;
}

struct bz_value *
bz_homebrew_pkgconfig_path_value_new(void)
{
    struct bz_homebrew_pkgconfig  *self =
        cork_new(struct bz_homebrew_pkgconfig);
    self->packages = cork_string_hash_table_new(0, 0);
    cork_buffer_init(&self->buf);
    return bz_scalar_value_new
        (self, bz_homebrew_pkgconfig_path__free,
         bz_homebrew_pkgconfig_path__get);
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
