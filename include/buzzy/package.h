/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_PACKAGE_H
#define BUZZY_PACKAGE_H

#include <libcork/core.h>

#include "buzzy/env.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Packages
 */

struct bz_package;

typedef int
(*bz_package_is_needed_f)(void *user_data, bool *is_needed);

typedef int
(*bz_package_step_f)(void *user_data);


struct bz_package *
bz_package_new(const char *name, struct bz_version *version, struct bz_env *env,
               void *user_data, cork_free_f free_user_data,
               bz_package_step_f build,
               bz_package_step_f test,
               bz_package_step_f install,
               bz_package_step_f uninstall);

void
bz_package_free(struct bz_package *package);

struct bz_env *
bz_package_env(struct bz_package *package);

const char *
bz_package_name(struct bz_package *package);

struct bz_version *
bz_package_version(struct bz_package *package);

int
bz_package_build(struct bz_package *package);

int
bz_package_test(struct bz_package *package);

int
bz_package_install(struct bz_package *package);

int
bz_package_uninstall(struct bz_package *package);


/*-----------------------------------------------------------------------
 * Package databases
 */

struct bz_pdb;

/* The pdb is responsible for freeing any packages it creates. */
typedef struct bz_package *
(*bz_pdb_satisfy_f)(void *user_data, struct bz_dependency *dep);


struct bz_pdb *
bz_pdb_new(const char *pdb_name,
           void *user_data, cork_free_f free_user_data,
           bz_pdb_satisfy_f satisfy);

void
bz_pdb_free(struct bz_pdb *pdb);

struct bz_package *
bz_pdb_satisfy_dependency(struct bz_pdb *pdb, struct bz_dependency *dep);


/*-----------------------------------------------------------------------
 * Single-package databases
 */

/* Takes control of package */
struct bz_pdb *
bz_single_package_pdb_new(const char *pdb_name, struct bz_package *package);


/*-----------------------------------------------------------------------
 * Cached package databases
 */

/* A helper implementation that automatically caches packages for you.  You
 * provide a satsify method that just blindly creates a package for a
 * dependency.  The cache helper translates that into a satisfy method that will
 * check if we've already created a package for a particular dependency, and if
 * so, return it. */
struct bz_pdb *
bz_cached_pdb_new(const char *pdb_name,
                  void *user_data, cork_free_f free_user_data,
                  bz_pdb_satisfy_f satisfy);


/*-----------------------------------------------------------------------
 * Package database registry
 */

/* Takes control of pdb */
void
bz_pdb_register(struct bz_pdb *pdb);

void
bz_pdb_registry_clear(void);

struct bz_package *
bz_satisfy_dependency(struct bz_dependency *dep);

int
bz_install_dependency(struct bz_dependency *dep);

struct bz_package *
bz_satisfy_dependency_string(const char *dep_string);

int
bz_install_dependency_string(const char *dep_string);


#endif /* BUZZY_PACKAGE_H */
