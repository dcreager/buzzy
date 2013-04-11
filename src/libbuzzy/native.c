/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdarg.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/error.h"
#include "buzzy/native.h"
#include "buzzy/package.h"
#include "buzzy/version.h"


#if !defined(BZ_DEBUG_NATIVE_PACKAGES)
#define BZ_DEBUG_NATIVE_PACKAGES  0
#endif

#if BZ_DEBUG_NATIVE_PACKAGES
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * Native packages
 */

struct bz_native_package {
    const char  *short_distro_name;
    const char  *package_name;
    const char  *native_package_name;
    struct bz_version  *version;
    bz_native_detect_f  version_installed;
    bz_native_install_f  install;
};

static void
bz_native_package__free(void *user_data)
{
    struct bz_native_package  *native = user_data;
    cork_strfree(native->short_distro_name);
    cork_strfree(native->package_name);
    cork_strfree(native->native_package_name);
    /* version will be freed by the package wrapper */
    free(native);
}


static int
bz_native_package__build__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_native_package  *native = user_data;
    cork_buffer_append_printf
        (dest, "Build native %s package %s %s",
         native->short_distro_name,
         native->native_package_name,
         bz_version_to_string(native->version));
    return 0;
}

static int
bz_native_package__build__is_needed(void *user_data, bool *is_needed)
{
    /* Native packages never need to be built. */
    *is_needed = false;
    return 0;
}

static int
bz_native_package__build__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
bz_native_package__build(void *user_data)
{
    struct bz_native_package  *native = user_data;
    return bz_action_new
        (native, NULL,
         bz_native_package__build__message,
         bz_native_package__build__is_needed,
         bz_native_package__build__perform);
}


static int
bz_native_package__test__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_native_package  *native = user_data;
    cork_buffer_append_printf
        (dest, "test native %s package %s %s",
         native->short_distro_name,
         native->native_package_name,
         bz_version_to_string(native->version));
    return 0;
}

static int
bz_native_package__test__is_needed(void *user_data, bool *is_needed)
{
    /* Native packages never need to be tested. */
    *is_needed = false;
    return 0;
}

static int
bz_native_package__test__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
bz_native_package__test(void *user_data)
{
    struct bz_native_package  *native = user_data;
    return bz_action_new
        (native, NULL,
         bz_native_package__test__message,
         bz_native_package__test__is_needed,
         bz_native_package__test__perform);
}


static int
bz_native_package__install__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_native_package  *native = user_data;
    cork_buffer_append_printf
        (dest, "Install native %s package %s %s",
         native->short_distro_name,
         native->native_package_name,
         bz_version_to_string(native->version));
    return 0;
}

static int
bz_native_package__install__is_needed(void *user_data, bool *is_needed)
{
    struct bz_native_package  *native = user_data;
    struct bz_version  *installed;

    installed = native->version_installed(native->native_package_name);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return -1;
    }

    if (installed == NULL) {
        *is_needed = true;
        return 0;
    } else {
        *is_needed = (bz_version_cmp(installed, native->version) < 0);
        bz_version_free(installed);
        return 0;
    }
}

static int
bz_native_package__install__perform(void *user_data)
{
    struct bz_native_package  *native = user_data;
    return native->install(native->native_package_name, native->version);
}

static struct bz_action *
bz_native_package__install(void *user_data)
{
    struct bz_native_package  *native = user_data;
    return bz_action_new
        (native, NULL,
         bz_native_package__install__message,
         bz_native_package__install__is_needed,
         bz_native_package__install__perform);
}


struct bz_package *
bz_native_package_new(const char *short_distro_name,
                      const char *package_name, const char *native_package_name,
                      struct bz_version *version,
                      bz_native_detect_f version_installed,
                      bz_native_install_f install)
{
    struct bz_native_package  *native;
    struct bz_env  *env;

    native = cork_new(struct bz_native_package);
    native->short_distro_name = cork_strdup(short_distro_name);
    native->package_name = cork_strdup(package_name);
    native->native_package_name = cork_strdup(native_package_name);
    native->version = version;
    native->version_installed = version_installed;
    native->install = install;

    env = bz_package_env_new_empty(NULL, package_name);
    bz_env_add_override(env, "name", bz_string_value_new(package_name));
    bz_env_add_override
        (env, "version", bz_string_value_new(bz_version_to_string(version)));

    return bz_package_new
        (package_name, version, env,
         native, bz_native_package__free,
         bz_native_package__build,
         bz_native_package__test,
         bz_native_package__install);
}


/*-----------------------------------------------------------------------
 * Native package databases
 */

struct bz_native_pdb {
    const char  *short_distro_name;
    bz_native_detect_f  version_available;
    bz_native_detect_f  version_installed;
    bz_native_install_f  install;
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
    cork_buffer_done(&pdb->buf);
    free(pdb);
}

static struct bz_package *
bz_native_pdb_try_pattern(struct bz_native_pdb *pdb, const char *pattern,
                          struct bz_dependency *dep)
{
    struct bz_version  *available;
    cork_buffer_printf(&pdb->buf, pattern, dep->package_name);
    available = pdb->version_available(pdb->buf.buf);
    if (available == NULL) {
        return NULL;
    } else {
        return bz_native_package_new
            (pdb->short_distro_name,
             dep->package_name, pdb->buf.buf, available,
             pdb->version_installed, pdb->install);
    }
}

static struct bz_package *
bz_native_pdb__satisfy(void *user_data, struct bz_dependency *dep)
{
    size_t  i;
    struct bz_native_pdb  *pdb = user_data;
    for (i = 0; i < cork_array_size(&pdb->patterns); i++) {
        const char  *pattern = cork_array_at(&pdb->patterns, i);
        struct bz_package  *package =
            bz_native_pdb_try_pattern(pdb, pattern, dep);
        if (package != NULL) {
            return package;
        }
    }
    return NULL;
}

struct bz_pdb *
bz_native_pdb_new(const char *short_distro_name,
                  bz_native_detect_f version_available,
                  bz_native_detect_f version_installed,
                  bz_native_install_f install,
                  /* const char *pattern */ ...)
{
    va_list  args;
    const char  *pattern;
    struct bz_native_pdb  *pdb = cork_new(struct bz_native_pdb);
    pdb->version_available = version_available;
    pdb->version_installed = version_installed;
    pdb->install = install;
    pdb->short_distro_name = cork_strdup(short_distro_name);
    cork_buffer_init(&pdb->buf);
    cork_array_init(&pdb->patterns);
    va_start(args, install);
    while ((pattern = va_arg(args, const char *)) != NULL) {
        cork_array_append(&pdb->patterns, cork_strdup(pattern));
    }
    cork_buffer_printf(&pdb->buf, "Native %s packages", short_distro_name);
    return bz_cached_pdb_new
        (pdb->buf.buf,
         pdb, bz_native_pdb__free,
         bz_native_pdb__satisfy);
}
