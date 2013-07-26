/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdarg.h>

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/native.h"
#include "buzzy/package.h"
#include "buzzy/version.h"

#define CLOG_CHANNEL  "native"


/*-----------------------------------------------------------------------
 * Preinstalled packages
 */

struct bz_preinstalled {
    struct bz_env  *env;
    const char  *short_distro_name;
    const char  *package_name;
    struct bz_version  *version;
};

static void
bz_preinstalled__free(void *user_data)
{
    struct bz_preinstalled  *preinstalled = user_data;
    cork_strfree(preinstalled->short_distro_name);
    cork_strfree(preinstalled->package_name);
    bz_env_free(preinstalled->env);
    free(preinstalled);
}

static int
bz_preinstalled__package__is_needed(void *user_data, bool *is_needed)
{
    /* Preinstalled packages never need to be packaged. */
    *is_needed = false;
    return 0;
}

static int
bz_preinstalled__package(void *user_data)
{
    return 0;
}


static int
bz_preinstalled__install__is_needed(void *user_data, bool *is_needed)
{
    /* Preinstalled packages never need to be installed, but we still want to
     * print out a nice message. */
    *is_needed = true;
    return 0;
}

static int
bz_preinstalled__install(void *user_data)
{
    struct bz_preinstalled  *preinstalled = user_data;
    bz_log_action
        ("Preinstalled native %s package %s %s",
         preinstalled->short_distro_name,
         preinstalled->package_name,
         bz_version_to_string(preinstalled->version));
    return 0;
}


static int
bz_preinstalled__uninstall__is_needed(void *user_data, bool *is_needed)
{
    /* Preinstalled packages never need to be uninstalled. */
    *is_needed = false;
    return 0;
}

static int
bz_preinstalled__uninstall(void *user_data)
{
    return 0;
}

static struct bz_packager *
bz_preinstalled_packager_new(struct bz_env *env,
                             const char *short_distro_name,
                             const char *package_name,
                             struct bz_version *version)
{
    struct bz_preinstalled  *preinstalled = cork_new(struct bz_preinstalled);
    preinstalled->env = env;
    preinstalled->short_distro_name = cork_strdup(short_distro_name);
    preinstalled->package_name = cork_strdup(package_name);
    preinstalled->version = version;
    return bz_packager_new
        (env, short_distro_name,
         preinstalled, bz_preinstalled__free,
         bz_preinstalled__package__is_needed,
         bz_preinstalled__package,
         bz_preinstalled__install__is_needed,
         bz_preinstalled__install,
         bz_preinstalled__uninstall__is_needed,
         bz_preinstalled__uninstall);
}

struct bz_package *
bz_preinstalled_package_new(const char *short_distro_name,
                            const char *package_name,
                            struct bz_version *version)
{
    struct bz_env  *env;
    struct bz_builder  *builder;
    struct bz_packager  *packager;

    env = bz_package_env_new_empty(NULL, package_name);
    bz_env_add_override(env, "name", bz_string_value_new(package_name));
    bz_env_add_override
        (env, "version", bz_string_value_new(bz_version_to_string(version)));
    builder = bz_noop_builder_new(env);
    packager = bz_preinstalled_packager_new
        (env, short_distro_name, package_name, version);
    return bz_package_new(package_name, version, env, builder, packager);
}


/*-----------------------------------------------------------------------
 * Native packages
 */

struct bz_native_packager {
    struct bz_env  *env;
    const char  *short_distro_name;
    const char  *package_name;
    const char  *native_package_name;
    struct bz_version  *version;
    bz_native_detect_f  version_installed;
    bz_native_install_f  install;
    bz_native_uninstall_f  uninstall;
};

static void
bz_native_packager__free(void *user_data)
{
    struct bz_native_packager  *native = user_data;
    cork_strfree(native->short_distro_name);
    cork_strfree(native->package_name);
    cork_strfree(native->native_package_name);
    bz_env_free(native->env);
    bz_version_free(native->version);
    free(native);
}

static int
bz_native_packager__package__is_needed(void *user_data, bool *is_needed)
{
    /* Native packages never need to be packaged. */
    *is_needed = false;
    return 0;
}

static int
bz_native_packager__package(void *user_data)
{
    return 0;
}


static int
bz_native_packager__install__is_needed(void *user_data, bool *is_needed)
{
    struct bz_native_packager  *native = user_data;
    struct bz_version  *installed;
    clog_info("(%s) Check whether %s package %s is needed",
              native->package_name,
              native->short_distro_name, native->native_package_name);
    rie_check(installed = native->version_installed
              (native->native_package_name));
    if (installed == NULL) {
        *is_needed = true;
    } else {
        *is_needed = (bz_version_cmp(installed, native->version) < 0);
        bz_version_free(installed);
    }
    return 0;
}

static int
bz_native_packager__install(void *user_data)
{
    struct bz_native_packager  *native = user_data;
    bz_log_action
        ("Install native %s package %s %s",
         native->short_distro_name,
         native->native_package_name,
         bz_version_to_string(native->version));
    return native->install(native->native_package_name, native->version);
}


static int
bz_native_packager__uninstall__is_needed(void *user_data, bool *is_needed)
{
    struct bz_native_packager  *native = user_data;
    struct bz_version  *installed;
    clog_info("(%s) Check whether %s package %s is installed",
              native->package_name,
              native->short_distro_name, native->native_package_name);
    rie_check(installed = native->version_installed
              (native->native_package_name));
    /* Uninstall any version that happens to be installed. */
    if (installed == NULL) {
        *is_needed = false;
    } else {
        *is_needed = true;
        bz_version_free(installed);
    }
    return 0;
}

static int
bz_native_packager__uninstall(void *user_data)
{
    struct bz_native_packager  *native = user_data;
    bz_log_action
        ("Uninstall native %s package %s %s",
         native->short_distro_name,
         native->native_package_name,
         bz_version_to_string(native->version));
    return native->uninstall(native->native_package_name);
}

static struct bz_packager *
bz_native_packager_new(struct bz_env *env,
                       const char *short_distro_name,
                       const char *package_name,
                       const char *native_package_name,
                       struct bz_version *version,
                       bz_native_detect_f version_installed,
                       bz_native_install_f install,
                       bz_native_uninstall_f uninstall)
{
    struct bz_native_packager  *native;

    native = cork_new(struct bz_native_packager);
    native->env = env;
    native->short_distro_name = cork_strdup(short_distro_name);
    native->package_name = cork_strdup(package_name);
    native->native_package_name = cork_strdup(native_package_name);
    native->version = version;
    native->version_installed = version_installed;
    native->install = install;
    native->uninstall = uninstall;

    return bz_packager_new
        (env, short_distro_name,
         native, bz_native_packager__free,
         bz_native_packager__package__is_needed,
         bz_native_packager__package,
         bz_native_packager__install__is_needed,
         bz_native_packager__install,
         bz_native_packager__uninstall__is_needed,
         bz_native_packager__uninstall);
}

struct bz_package *
bz_native_package_new(const char *short_distro_name,
                      const char *package_name, const char *native_package_name,
                      struct bz_version *version,
                      bz_native_detect_f version_installed,
                      bz_native_install_f install,
                      bz_native_uninstall_f uninstall)
{
    struct bz_env  *env;
    struct bz_builder  *builder;
    struct bz_packager  *packager;

    env = bz_package_env_new_empty(NULL, package_name);
    bz_env_add_override(env, "name", bz_string_value_new(package_name));
    bz_env_add_override
        (env, "version", bz_string_value_new(bz_version_to_string(version)));
    builder = bz_noop_builder_new(env);
    packager = bz_native_packager_new
        (env, short_distro_name, package_name, native_package_name,
         version, version_installed, install, uninstall);
    return bz_package_new(package_name, version, env, builder, packager);
}


/*-----------------------------------------------------------------------
 * Native package databases
 */

struct bz_native_pdb {
    const char  *short_distro_name;
    const char  *slug;
    bz_native_detect_f  version_available;
    bz_native_detect_f  version_installed;
    bz_native_install_f  install;
    bz_native_uninstall_f  uninstall;
    cork_array(const char *)  patterns;
    struct cork_buffer  buf;
};

static void
bz_native_pdb__free(void *user_data)
{
    size_t  i;
    struct bz_native_pdb  *pdb = user_data;

    for (i = 0; i < cork_array_size(&pdb->patterns); i++) {
        const char  *pattern = cork_array_at(&pdb->patterns, i);
        cork_strfree(pattern);
    }
    cork_array_done(&pdb->patterns);

    cork_strfree(pdb->short_distro_name);
    cork_strfree(pdb->slug);
    cork_buffer_done(&pdb->buf);
    free(pdb);
}

static struct bz_package *
bz_native_pdb_try_package(struct bz_native_pdb *pdb, const char *name,
                          struct bz_dependency *dep)
{
    struct bz_version  *available;
    clog_info("(%s) Check whether %s package %s exists",
              dep->package_name, pdb->short_distro_name, name);
    available = pdb->version_available(name);
    if (available == NULL) {
        return NULL;
    }

    if (dep->min_version != NULL) {
        if (bz_version_cmp(available, dep->min_version) < 0) {
            clog_info("(%s) Native version %s is older than %s",
                      dep->package_name,
                      bz_version_to_string(available),
                      bz_version_to_string(dep->min_version));
            return NULL;
        }
    }

    return bz_native_package_new
        (pdb->short_distro_name,
         dep->package_name, name, available,
         pdb->version_installed, pdb->install, pdb->uninstall);
}

static struct bz_package *
bz_native_pdb__satisfy(void *user_data, struct bz_dependency *dep,
                       struct bz_value *ctx)
{
    size_t  i;
    struct bz_native_pdb  *pdb = user_data;
    struct bz_package  *result;
    struct bz_version  *preinstalled_version;
    const char  *name;

    /* First see if the package is preinstalled on the current platform. */
    cork_buffer_printf
        (&pdb->buf, "preinstalled.%s.%s", pdb->slug, dep->package_name);
    rpe_check(preinstalled_version =
              bz_value_get_version(ctx, pdb->buf.buf, false));
    if (preinstalled_version != NULL) {
        return bz_preinstalled_package_new
            (pdb->short_distro_name, dep->package_name, preinstalled_version);
    }

    /* Then see if someone has provided an explicit native package name. */
    cork_buffer_printf(&pdb->buf, "native.%s", dep->package_name);
    rpe_check(name = bz_value_get_string(ctx, pdb->buf.buf, false));
    if (name != NULL) {
        return bz_native_pdb_try_package(pdb, name, dep);
    }

    cork_buffer_printf(&pdb->buf, "native.%s.%s", pdb->slug, dep->package_name);
    rpe_check(name = bz_value_get_string(ctx, pdb->buf.buf, false));
    if (name != NULL) {
        return bz_native_pdb_try_package(pdb, name, dep);
    }

    /* Otherwise try each of the standard patterns for this architecture. */
    for (i = 0; i < cork_array_size(&pdb->patterns); i++) {
        const char  *pattern = cork_array_at(&pdb->patterns, i);
        cork_buffer_printf(&pdb->buf, pattern, dep->package_name);
        rpe_check(result = bz_native_pdb_try_package(pdb, pdb->buf.buf, dep));
        if (result != NULL) {
            return result;
        }
    }
    return NULL;
}

struct bz_pdb *
bz_native_pdb_new(const char *short_distro_name, const char *slug,
                  bz_native_detect_f version_available,
                  bz_native_detect_f version_installed,
                  bz_native_install_f install,
                  bz_native_uninstall_f uninstall,
                  /* const char *pattern */ ...)
{
    va_list  args;
    const char  *pattern;
    struct bz_native_pdb  *pdb = cork_new(struct bz_native_pdb);
    pdb->version_available = version_available;
    pdb->version_installed = version_installed;
    pdb->install = install;
    pdb->uninstall = uninstall;
    pdb->short_distro_name = cork_strdup(short_distro_name);
    pdb->slug = cork_strdup(slug);
    cork_buffer_init(&pdb->buf);
    cork_array_init(&pdb->patterns);
    va_start(args, uninstall);
    while ((pattern = va_arg(args, const char *)) != NULL) {
        cork_array_append(&pdb->patterns, cork_strdup(pattern));
    }
    cork_buffer_printf(&pdb->buf, "Native %s packages", short_distro_name);
    return bz_cached_pdb_new
        (pdb->buf.buf,
         pdb, bz_native_pdb__free,
         bz_native_pdb__satisfy);
}
