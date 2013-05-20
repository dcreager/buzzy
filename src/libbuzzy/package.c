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

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/package.h"
#include "buzzy/version.h"


#if !defined(BZ_DEBUG_PACKAGES)
#define BZ_DEBUG_PACKAGES  0
#endif

#if BZ_DEBUG_PACKAGES
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


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
        version, "version",
        NULL,
        "The version of the package",
        ""
    );

    bz_package_variable(
        package_work_dir, "package_work_dir",
        bz_interpolated_value_new("${work_dir}/build/${name}/${version}"),
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
        install_prefix, "install_prefix",
        bz_string_value_new("/usr"),
        "The installation prefix for the package",
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
}


/*-----------------------------------------------------------------------
 * Packages
 */

struct bz_package {
    struct bz_env  *env;
    const char  *name;
    struct bz_version  *version;
    void  *user_data;
    cork_free_f  free_user_data;
    bz_package_step_f  build;
    bz_package_step_f  test;
    bz_package_step_f  install;
    bool  built;
    bool  tested;
    bool  installed;
};


struct bz_package *
bz_package_new(const char *name, struct bz_version *version, struct bz_env *env,
               void *user_data, cork_free_f free_user_data,
               bz_package_step_f build,
               bz_package_step_f test,
               bz_package_step_f install)
{
    struct bz_package  *package = cork_new(struct bz_package);
    package->env = env;
    package->name = cork_strdup(name);
    package->version = version;
    package->user_data = user_data;
    package->free_user_data = free_user_data;
    package->build = build;
    package->test = test;
    package->install = install;
    package->built = false;
    package->tested = false;
    package->installed = false;
    return package;
}

void
bz_package_free(struct bz_package *package)
{
    cork_strfree(package->name);
    bz_version_free(package->version);
    cork_free_user_data(package);
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

int
bz_package_build(struct bz_package *package)
{
    if (package->built) {
        return 0;
    } else {
        package->built = true;
        return package->build(package->user_data);
    }
}

int
bz_package_test(struct bz_package *package)
{
    if (package->tested) {
        return 0;
    } else {
        package->tested = true;
        return package->test(package->user_data);
    }
}

int
bz_package_install(struct bz_package *package)
{
    if (package->installed) {
        return 0;
    } else {
        package->installed = true;
        return package->install(package->user_data);
    }
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
bz_pdb_satisfy_dependency(struct bz_pdb *pdb, struct bz_dependency *dep)
{
    return pdb->satisfy(pdb->user_data, dep);
}


/*-----------------------------------------------------------------------
 * Single-package databases
 */

struct bz_single_package_pdb {
    struct bz_package  *package;
    const char  *package_name;
    struct bz_version  *package_version;
};

static void
bz_single_package_pdb__free(void *user_data)
{
    struct bz_single_package_pdb  *pdb = user_data;
    bz_package_free(pdb->package);
    free(pdb);
}

static struct bz_package *
bz_single_package_pdb__satisfy(void *user_data, struct bz_dependency *dep)
{
    struct bz_single_package_pdb  *pdb = user_data;
    if (strcmp(pdb->package_name, dep->package_name) == 0) {
        if (dep->min_version == NULL ||
            bz_version_cmp(pdb->package_version, dep->min_version) >= 0) {
            return pdb->package;
        }
    }
    return NULL;
}

struct bz_pdb *
bz_single_package_pdb_new(const char *pdb_name, struct bz_package *package)
{
    struct bz_single_package_pdb  *pdb = cork_new(struct bz_single_package_pdb);
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
    struct cork_hash_table  packages;
};

static enum cork_hash_table_map_result
bz_cached_pdb__free_package(struct cork_hash_table_entry *entry, void *ud)
{
    const char  *dep_string = entry->key;
    struct bz_package  *package = entry->value;
    cork_strfree(dep_string);
    if (package != NULL) {
        bz_package_free(package);
    }
    return CORK_HASH_TABLE_MAP_DELETE;
}

static void
bz_cached_pdb__free(void *user_data)
{
    struct bz_cached_pdb  *pdb = user_data;
    cork_hash_table_map(&pdb->packages, bz_cached_pdb__free_package, NULL);
    cork_hash_table_done(&pdb->packages);
    cork_free_user_data(pdb);
    free(pdb);
}

static struct bz_package *
bz_cached_pdb__satisfy(void *user_data, struct bz_dependency *dep)
{
    struct bz_cached_pdb  *pdb = user_data;
    const char  *dep_string = bz_dependency_to_string(dep);
    bool  is_new;
    struct cork_hash_table_entry  *entry;

    entry = cork_hash_table_get_or_create
        (&pdb->packages, (void *) dep_string, &is_new);

    if (is_new) {
        struct bz_package  *package = pdb->satisfy(pdb->user_data, dep);
        entry->key = (void *) cork_strdup(dep_string);
        entry->value = package;
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
    cork_string_hash_table_init(&pdb->packages, 0);
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
bz_satisfy_dependency(struct bz_dependency *dep)
{
    struct cork_dllist_item  *curr;
    for (curr = cork_dllist_start(&pdbs); !cork_dllist_is_end(&pdbs, curr);
         curr = curr->next) {
        struct bz_pdb  *pdb = cork_container_of(curr, struct bz_pdb, item);
        struct bz_package  *package = bz_pdb_satisfy_dependency(pdb, dep);
        if (CORK_UNLIKELY(cork_error_occurred())) {
            return NULL;
        } else if (package != NULL) {
            return package;
        }
    }

    bz_cannot_satisfy
        ("Cannot satisfy dependency %s", bz_dependency_to_string(dep));
    return NULL;
}

int
bz_install_dependency(struct bz_dependency *dep)
{
    struct bz_package  *package;
    rip_check(package = bz_satisfy_dependency(dep));
    return bz_package_install(package);
}

struct bz_package *
bz_satisfy_dependency_string(const char *dep_string)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    rpp_check(dep = bz_dependency_from_string(dep_string));
    package = bz_satisfy_dependency(dep);
    bz_dependency_free(dep);
    return package;
}

int
bz_install_dependency_string(const char *dep_string)
{
    struct bz_dependency  *dep;
    int  rc;
    rip_check(dep = bz_dependency_from_string(dep_string));
    rc = bz_install_dependency(dep);
    bz_dependency_free(dep);
    return rc;
}
