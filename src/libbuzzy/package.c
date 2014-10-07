/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <string.h>

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/package.h"
#include "buzzy/value.h"
#include "buzzy/version.h"

#define CLOG_CHANNEL  "package"


/*-----------------------------------------------------------------------
 * Builtin package-specific variables
 */

bz_define_variables(package)
{
    bz_package_variable(
        name, "name",
        NULL,
        "The name of the package",
        ""
    );

    bz_package_variable(
        native_name, "native_name",
        bz_interpolated_value_new("${name}"),
        "The native name of the package",
        ""
    );

    bz_package_variable(
        version, "version",
        NULL,
        "The version of the package",
        ""
    );

    bz_package_variable(
        package_slug, "package_slug",
        bz_interpolated_value_new("${name}-buzzy"),
        "A unique name for a combination of a package and its source",
        ""
    );

    bz_package_variable(
        package_work_dir, "package_work_dir",
        bz_interpolated_value_new("${work_dir}/build/${package_slug}"),
        "Location for artefacts created while building or installing a package",
        ""
    );

    bz_package_variable(
        license, "license",
        bz_string_value_new("unknown"),
        "The license that the package is released under",
        ""
    );

    bz_package_variable(
        dependencies, "dependencies",
        NULL,
        "Other packages that this package needs at runtime",
        ""
    );

    bz_package_variable(
        build_dependencies, "build_dependencies",
        NULL,
        "Other packages needed to build this package",
        ""
    );

    bz_package_variable(
        force, "force",
        bz_string_value_new("false"),
        "Whether to always rebuild and reinstall a package",
        ""
    );

    bz_package_variable(
        verbose, "verbose",
        bz_string_value_new("false"),
        "Whether to print out more information while building a package",
        ""
    );

    /* Everything below is only needed for built packages */

    bz_package_variable(
        builder, "builder",
        bz_builder_detector_new(),
        "What build system is used to build the package",
        ""
    );

    bz_package_variable(
        packager, "packager",
        bz_packager_detector_new(),
        "What packager is used to create a binary package file",
        ""
    );

    bz_package_variable(
        relocatable, "relocatable",
        bz_string_value_new("false"),
        "The installation prefix for platform-agnostic files",
        ""
    );

    bz_package_variable(
        prefix, "prefix",
        bz_string_value_new("/usr"),
        "The installation prefix for platform-agnostic files",
        ""
    );

    bz_package_variable(
        prefix, "exec_prefix",
        bz_interpolated_value_new("${prefix}"),
        "The installation prefix for platform-specific files",
        ""
    );

    bz_package_variable(
        bin_dir, "bin_dir",
        bz_interpolated_value_new("${exec_prefix}/bin"),
        "The installation location for binaries",
        ""
    );

    bz_package_variable(
        sbin_dir, "sbin_dir",
        bz_interpolated_value_new("${exec_prefix}/sbin"),
        "The installation location for superuser binaries",
        ""
    );

    bz_package_variable(
        lib_dir, "lib_dir_name",
        bz_interpolated_value_new("lib"),
        "The base name of the installation location for libraries",
        ""
    );

    bz_package_variable(
        lib_dir, "lib_dir",
        bz_interpolated_value_new("${exec_prefix}/${lib_dir_name}"),
        "The installation location for libraries",
        ""
    );

    bz_package_variable(
        libexec_dir, "libexec_dir",
        bz_interpolated_value_new("${exec_prefix}/lib"),
        "The installation location for internal binaries",
        ""
    );

    bz_package_variable(
        share_dir, "share_dir",
        bz_interpolated_value_new("${prefix}/share"),
        "The installation location for data files",
        ""
    );

    bz_package_variable(
        doc_dir, "doc_dir",
        bz_interpolated_value_new("${share_dir}/doc"),
        "The installation location for documentation",
        ""
    );

    bz_package_variable(
        man_dir, "man_dir",
        bz_interpolated_value_new("${share_dir}/man"),
        "The installation location for manuals",
        ""
    );

    bz_package_variable(
        build_dir, "build_dir",
        bz_interpolated_value_new("${package_work_dir}/build"),
        "Where the package's build artefacts should be placed",
        ""
    );

    bz_package_variable(
        package_build_dir, "package_build_dir",
        bz_interpolated_value_new("${package_work_dir}/pkg"),
        "Temporary directory while building a binary package",
        ""
    );

    bz_package_variable(
        source_dir, "source_dir",
        bz_interpolated_value_new("${package_work_dir}/source"),
        "Where the package's extracted source archive should be placed",
        ""
    );

    bz_package_variable(
        staging_dir, "staging_dir",
        bz_interpolated_value_new("${package_work_dir}/stage"),
        "Where a package's staged installation should be placed",
        ""
    );

    bz_package_variable(
        pre_install_script, "pre_install_script",
        NULL,
        "A script to run before the package is installed, "
        "relative to build_dir",
        ""
    );

    bz_package_variable(
        post_install_script, "post_install_script",
        NULL,
        "A script to run after the package is installed, "
        "relative to build_dir",
        ""
    );

    bz_package_variable(
        pre_remove_script, "pre_remove_script",
        NULL,
        "A script to run before the package is removed, "
        "relative to build_dir",
        ""
    );

    bz_package_variable(
        post_remove_script, "post_remove_script",
        NULL,
        "A script to run after the package is removed, "
        "relative to build_dir",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Package lists
 */

struct bz_package_list {
    cork_array(struct bz_package *)  packages;
    bool  filled;
};

static void
bz_package_list_init(struct bz_package_list *list)
{
    cork_array_init(&list->packages);
    list->filled = false;
}

static void
bz_package_list_done(struct bz_package_list *list)
{
    cork_array_done(&list->packages);
}

struct bz_package_list_fill {
    struct bz_package_list  *list;
    struct bz_value  *ctx;
};

static int
bz_package_list_fill_one(void *user_data, struct bz_value *dep_value)
{
    struct bz_package_list_fill  *state = user_data;
    const char  *dep_string;
    struct bz_package  *dep;
    rip_check(dep_string = bz_scalar_value_get(dep_value, state->ctx));
    rip_check(dep = bz_satisfy_dependency_string(dep_string, state->ctx));
    cork_array_append(&state->list->packages, dep);
    return 0;
}

static int
bz_package_list_fill(struct bz_package_list *list, struct bz_package *package,
                     const char *var_name)
{
    if (!list->filled) {
        struct bz_env  *env = bz_package_env(package);
        struct bz_value  *ctx = bz_env_as_value(env);
        struct bz_value  *value;
        list->filled = true;
        rie_check(value = bz_map_value_get(ctx, var_name));
        if (value != NULL) {
            struct bz_package_list_fill  state = { list, ctx };
            return bz_array_value_map_scalars
                (value, &state, bz_package_list_fill_one);
        }
    }

    return 0;
}

size_t
bz_package_list_count(struct bz_package_list *list)
{
    assert(list->filled);
    return cork_array_size(&list->packages);
}

struct bz_package *
bz_package_list_get(struct bz_package_list *list, size_t index)
{
    assert(list->filled);
    return cork_array_at(&list->packages, index);
}

int
bz_package_list_install(struct bz_package_list *list)
{
    size_t  i;
    assert(list->filled);
    for (i = 0; i < cork_array_size(&list->packages); i++) {
        struct bz_package  *dep = cork_array_at(&list->packages, i);
        rii_check(bz_package_install(dep));
    }
    return 0;
}


/*-----------------------------------------------------------------------
 * Packages
 */

struct bz_package {
    struct bz_env  *env;
    const char  *name;
    struct bz_version  *version;
    struct bz_package_list  deps;
    struct bz_package_list  build_deps;
    struct bz_builder  *builder;
    struct bz_packager  *packager;
};


struct bz_package *
bz_package_new(const char *name, struct bz_version *version, struct bz_env *env,
               struct bz_builder *builder, struct bz_packager *packager)
{
    struct bz_package  *package = cork_new(struct bz_package);
    package->env = env;
    package->name = cork_strdup(name);
    package->version = bz_version_copy(version);
    bz_package_list_init(&package->deps);
    bz_package_list_init(&package->build_deps);
    package->builder = builder;
    bz_builder_set_package(builder, package);
    package->packager = packager;
    bz_packager_set_package(packager, package);
    return package;
}

void
bz_package_free(struct bz_package *package)
{
    cork_strfree(package->name);
    bz_version_free(package->version);
    bz_package_list_done(&package->deps);
    bz_package_list_done(&package->build_deps);
    bz_builder_free(package->builder);
    bz_packager_free(package->packager);
    free(package);
}

struct bz_env *
bz_package_env(struct bz_package *package)
{
    return package->env;
}

const char *
bz_package_name(struct bz_package *package)
{
    return package->name;
}

struct bz_version *
bz_package_version(struct bz_package *package)
{
    return package->version;
}


static int
bz_package_load_deps(struct bz_package *package)
{
    rii_check(bz_package_list_fill
              (&package->deps, package, "dependencies"));
    rii_check(bz_package_list_fill
              (&package->build_deps, package, "build_dependencies"));
    return 0;
}

struct bz_package_list *
bz_package_build_deps(struct bz_package *package)
{
    rpi_check(bz_package_load_deps(package));
    return &package->build_deps;
}

struct bz_package_list *
bz_package_deps(struct bz_package *package)
{
    rpi_check(bz_package_load_deps(package));
    return &package->deps;
}

int
bz_package_install_build_deps(struct bz_package *package)
{
    struct bz_package_list  *list;
    rip_check(list = bz_package_build_deps(package));
    return bz_package_list_install(list);
}

int
bz_package_install_deps(struct bz_package *package)
{
    struct bz_package_list  *list;
    rip_check(list = bz_package_deps(package));
    return bz_package_list_install(list);
}


int
bz_package_build(struct bz_package *package)
{
    return bz_builder_build(package->builder);
}

int
bz_package_test(struct bz_package *package)
{
    return bz_builder_test(package->builder);
}

int
bz_package_stage(struct bz_package *package)
{
    return bz_builder_stage(package->builder);
}

int
bz_package_package(struct bz_package *package)
{
    return bz_packager_package(package->packager);
}

int
bz_package_install(struct bz_package *package)
{
    return bz_packager_install(package->packager);
}

int
bz_package_uninstall(struct bz_package *package)
{
    return bz_packager_uninstall(package->packager);
}


/*-----------------------------------------------------------------------
 * Built packages
 */

struct bz_package *
bz_built_package_new(struct bz_env *env)
{
    const char  *name;
    struct bz_version  *version;
    struct bz_builder  *builder;
    struct bz_packager  *packager;
    rpp_check(name = bz_env_get_string(env, "name", true));
    rpp_check(version = bz_env_get_version(env, "version", true));
    rpp_check(builder = bz_package_builder_new(env));
    rpp_check(packager = bz_package_packager_new(env));
    return bz_package_new (name, version, env, builder, packager);
}


/*-----------------------------------------------------------------------
 * Package databases
 */

struct bz_pdb {
    const char  *name;
    void  *user_data;
    cork_free_f  free_user_data;
    bz_pdb_satisfy_f  satisfy;
    struct cork_dllist_item  item;
};


struct bz_pdb *
bz_pdb_new(const char *name,
           void *user_data, cork_free_f free_user_data,
           bz_pdb_satisfy_f satisfy)
{
    struct bz_pdb  *pdb = cork_new(struct bz_pdb);
    pdb->name = cork_strdup(name);
    pdb->user_data = user_data;
    pdb->free_user_data = free_user_data;
    pdb->satisfy = satisfy;
    return pdb;
}

void
bz_pdb_free(struct bz_pdb *pdb)
{
    cork_strfree(pdb->name);
    cork_free_user_data(pdb);
    free(pdb);
}

struct bz_package *
bz_pdb_satisfy_dependency(struct bz_pdb *pdb, struct bz_dependency *dep,
                          struct bz_value *ctx)
{
    if (ctx == NULL) {
        struct bz_env  *env = bz_global_env();
        ctx = bz_env_as_value(env);
    }
    return pdb->satisfy(pdb->user_data, dep, ctx);
}


/*-----------------------------------------------------------------------
 * Single-package databases
 */

struct bz_single_package_pdb {
    struct bz_package  *package;
    const char  *package_name;
    struct bz_version  *package_version;
    bool  free_package;
};

static void
bz_single_package_pdb__free(void *user_data)
{
    struct bz_single_package_pdb  *pdb = user_data;
    if (pdb->free_package) {
        bz_package_free(pdb->package);
    }
    free(pdb);
}

static struct bz_package *
bz_single_package_pdb__satisfy(void *user_data, struct bz_dependency *dep,
                               struct bz_value *ctx)
{
    struct bz_single_package_pdb  *pdb = user_data;
    if (strcmp(pdb->package_name, dep->package_name) == 0) {
        if (dep->min_version == NULL ||
            bz_version_cmp(pdb->package_version, dep->min_version) >= 0) {
            /* Once we return the package instance, the cached_pdb wrapper is
             * going to try to free the package, so we shouldn't. */
            pdb->free_package = false;
            return pdb->package;
        }
    }
    return NULL;
}

struct bz_pdb *
bz_single_package_pdb_new(const char *pdb_name, struct bz_package *package)
{
    struct bz_single_package_pdb  *pdb = cork_new(struct bz_single_package_pdb);
    pdb->free_package = true;
    pdb->package = package;
    pdb->package_name = bz_package_name(package);
    pdb->package_version = bz_package_version(package);
    return bz_cached_pdb_new
        (pdb_name, pdb, bz_single_package_pdb__free,
         bz_single_package_pdb__satisfy);
}


/*-----------------------------------------------------------------------
 * Cached package databases
 */

struct bz_cached_pdb {
    void  *user_data;
    cork_free_f  free_user_data;
    bz_pdb_satisfy_f  satisfy;
    struct cork_hash_table  *packages;
    struct cork_hash_table  *unique_packages;
};

static void
bz_cached_pdb__free(void *user_data)
{
    struct bz_cached_pdb  *pdb = user_data;
    cork_hash_table_free(pdb->packages);
    cork_hash_table_free(pdb->unique_packages);
    cork_free_user_data(pdb);
    free(pdb);
}

static struct bz_package *
bz_cached_pdb__satisfy(void *user_data, struct bz_dependency *dep,
                       struct bz_value *ctx)
{
    struct bz_cached_pdb  *pdb = user_data;
    const char  *dep_string = bz_dependency_to_string(dep);
    bool  is_new;
    struct cork_hash_table_entry  *entry;

    /* TODO: We might want to have the cache be aware of the value context when
     * caching packages. */
    entry = cork_hash_table_get_or_create
        (pdb->packages, (void *) dep_string, &is_new);

    if (is_new) {
        struct bz_package  *package = pdb->satisfy(pdb->user_data, dep, ctx);
        entry->key = (void *) cork_strdup(dep_string);
        entry->value = package;
        if (package != NULL) {
            cork_hash_table_put
                (pdb->unique_packages, package, NULL, NULL, NULL, NULL);
        }
        return package;
    } else {
        return entry->value;
    }
}

struct bz_pdb *
bz_cached_pdb_new(const char *pdb_name,
                  void *user_data, cork_free_f free_user_data,
                  bz_pdb_satisfy_f satisfy)
{
    struct bz_cached_pdb  *pdb = cork_new(struct bz_cached_pdb);
    pdb->user_data = user_data;
    pdb->free_user_data = free_user_data;
    pdb->satisfy = satisfy;
    pdb->packages = cork_string_hash_table_new(0, 0);
    cork_hash_table_set_free_key(pdb->packages, (cork_free_f) cork_strfree);
    pdb->unique_packages = cork_pointer_hash_table_new(0, 0);
    cork_hash_table_set_free_key
        (pdb->unique_packages, (cork_free_f) bz_package_free);
    return bz_pdb_new
        (pdb_name, pdb, bz_cached_pdb__free, bz_cached_pdb__satisfy);
}


/*-----------------------------------------------------------------------
 * Package database registry
 */

static struct cork_dllist  pdbs;

static void
free_pdb(struct cork_dllist_item *item, void *user_data)
{
    struct bz_pdb  *pdb = cork_container_of(item, struct bz_pdb, item);
    bz_pdb_free(pdb);
}

static void
free_pdbs(void)
{
    cork_dllist_map(&pdbs, free_pdb, NULL);
}

CORK_INITIALIZER(init_pdbs)
{
    cork_dllist_init(&pdbs);
    cork_cleanup_at_exit(0, free_pdbs);
}

void
bz_pdb_register(struct bz_pdb *pdb)
{
    cork_dllist_add(&pdbs, &pdb->item);
}

void
bz_pdb_registry_clear(void)
{
    cork_dllist_map(&pdbs, free_pdb, NULL);
    cork_dllist_init(&pdbs);
}

struct bz_package *
bz_satisfy_dependency(struct bz_dependency *dep, struct bz_value *ctx)
{
    struct cork_dllist_item  *curr;
    const char  *dep_string = bz_dependency_to_string(dep);
    clog_info("(%s) Satisfy dependency %s", dep->package_name, dep_string);
    for (curr = cork_dllist_start(&pdbs); !cork_dllist_is_end(&pdbs, curr);
         curr = curr->next) {
        struct bz_pdb  *pdb = cork_container_of(curr, struct bz_pdb, item);
        struct bz_package  *package;
        clog_info("(%s) Check %s for %s",
                  dep->package_name, pdb->name, dep_string);
        rpe_check(package = bz_pdb_satisfy_dependency(pdb, dep, ctx));
        if (package != NULL) {
            clog_info("(%s) Found package that satisfies dependency",
                      dep->package_name);
            return package;
        }
    }

    bz_cannot_satisfy
        ("Cannot satisfy dependency %s", bz_dependency_to_string(dep));
    return NULL;
}

int
bz_install_dependency(struct bz_dependency *dep, struct bz_value *ctx)
{
    struct bz_package  *package;
    rip_check(package = bz_satisfy_dependency(dep, ctx));
    return bz_package_install(package);
}

struct bz_package *
bz_satisfy_dependency_string(const char *dep_string, struct bz_value *ctx)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    rpp_check(dep = bz_dependency_from_string(dep_string));
    package = bz_satisfy_dependency(dep, ctx);
    bz_dependency_free(dep);
    return package;
}

int
bz_install_dependency_string(const char *dep_string, struct bz_value *ctx)
{
    struct bz_dependency  *dep;
    int  rc;
    rip_check(dep = bz_dependency_from_string(dep_string));
    rc = bz_install_dependency(dep, ctx);
    bz_dependency_free(dep);
    return rc;
}
