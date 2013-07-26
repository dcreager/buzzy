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


/* When installing a package, there are three separate, orthogonal steps:
 *
 *   - obtaining and unpacking the source
 *   - building the code and staging a fake installation
 *   - packaging the staged installation and installing the package
 *
 * These three steps are handled by the "unpacker", "builder", and "packager"
 * interfaces defined in this file.  The three steps can be specified completely
 * independently of each other.  Once you have all three, you can create a
 * bz_package instance for the package.
 */

struct bz_package;
struct bz_package_list;

typedef int
(*bz_package_is_needed_f)(void *user_data, bool *is_needed);

typedef int
(*bz_package_step_f)(void *user_data);


/*-----------------------------------------------------------------------
 * Standard messages
 */

int
bz_build_message(struct bz_env *env, const char *builder_name);

int
bz_test_message(struct bz_env *env, const char *builder_name);

int
bz_stage_message(struct bz_env *env, const char *builder_name);


int
bz_package_message(struct bz_env *env, const char *packager_name);

int
bz_install_message(struct bz_env *env, const char *packager_name);

int
bz_uninstall_message(struct bz_env *env, const char *packager_name);


/*-----------------------------------------------------------------------
 * Builders
 */

struct bz_builder;

struct bz_builder *
bz_builder_new(struct bz_env *env, const char *builder_name,
               void *user_data, cork_free_f free_user_data,
               bz_package_is_needed_f build_needed,
               bz_package_step_f build,
               bz_package_is_needed_f test_needed,
               bz_package_step_f test,
               bz_package_is_needed_f stage_needed,
               bz_package_step_f stage);

void
bz_builder_free(struct bz_builder *builder);

void
bz_builder_set_package(struct bz_builder *builder, struct bz_package *pkg);

int
bz_builder_build(struct bz_builder *builder);

int
bz_builder_test(struct bz_builder *builder);

int
bz_builder_stage(struct bz_builder *builder);


struct bz_builder *
bz_package_builder_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Packagers
 */

struct bz_packager;

struct bz_packager *
bz_packager_new(struct bz_env *env, const char *packager_name,
                void *user_data, cork_free_f free_user_data,
                bz_package_is_needed_f package_needed,
                bz_package_step_f package,
                bz_package_is_needed_f install_needed,
                bz_package_step_f install,
                bz_package_is_needed_f uninstall_needed,
                bz_package_step_f uninstall);

void
bz_packager_free(struct bz_packager *packager);

void
bz_packager_set_package(struct bz_packager *packager, struct bz_package *pkg);

int
bz_packager_package(struct bz_packager *packager);

int
bz_packager_install(struct bz_packager *packager);

int
bz_packager_uninstall(struct bz_packager *packager);


struct bz_packager *
bz_package_packager_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Packages
 */

struct bz_package *
bz_package_new(const char *name, struct bz_version *version, struct bz_env *env,
               struct bz_builder *builder, struct bz_packager *packager);

void
bz_package_free(struct bz_package *package);

struct bz_env *
bz_package_env(struct bz_package *package);

const char *
bz_package_name(struct bz_package *package);

struct bz_version *
bz_package_version(struct bz_package *package);

struct bz_package_list *
bz_package_build_deps(struct bz_package *package);

int
bz_package_install_build_deps(struct bz_package *package);

struct bz_package_list *
bz_package_deps(struct bz_package *package);

int
bz_package_install_deps(struct bz_package *package);

int
bz_package_build(struct bz_package *package);

int
bz_package_test(struct bz_package *package);

int
bz_package_stage(struct bz_package *package);

int
bz_package_package(struct bz_package *package);

int
bz_package_install(struct bz_package *package);

int
bz_package_uninstall(struct bz_package *package);


/*-----------------------------------------------------------------------
 * Built packages
 */

struct bz_package *
bz_built_package_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Available builders
 */

struct bz_value *
bz_builder_detector_new(void);

struct bz_builder *
bz_noop_builder_new(struct bz_env *env);

struct bz_builder *
bz_autotools_builder_new(struct bz_env *env);

struct bz_builder *
bz_cmake_builder_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Available packagers
 */

struct bz_value *
bz_packager_detector_new(void);

struct bz_packager *
bz_homebrew_packager_new(struct bz_env *env);

struct bz_packager *
bz_noop_packager_new(struct bz_env *env);

struct bz_packager *
bz_pacman_packager_new(struct bz_env *env);

struct bz_packager *
bz_rpm_packager_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Lists of packages
 */

size_t
bz_package_list_count(struct bz_package_list *list);

struct bz_package *
bz_package_list_get(struct bz_package_list *list, size_t index);

int
bz_package_list_install(struct bz_package_list *list);


/*-----------------------------------------------------------------------
 * Package databases
 */

struct bz_pdb;

/* The pdb is responsible for freeing any packages it creates. */
typedef struct bz_package *
(*bz_pdb_satisfy_f)(void *user_data, struct bz_dependency *dep,
                    struct bz_value *ctx);


struct bz_pdb *
bz_pdb_new(const char *pdb_name,
           void *user_data, cork_free_f free_user_data,
           bz_pdb_satisfy_f satisfy);

void
bz_pdb_free(struct bz_pdb *pdb);

/* `requestor` is the environment that wants the dependency to be installed.
 * You can use that to customize which package that should be used to provide
 * the dependency.  If `requestor` is NULL, we'll use the global environment. */
struct bz_package *
bz_pdb_satisfy_dependency(struct bz_pdb *pdb, struct bz_dependency *dep,
                          struct bz_value *ctx);


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
bz_satisfy_dependency(struct bz_dependency *dep, struct bz_value *ctx);

int
bz_install_dependency(struct bz_dependency *dep, struct bz_value *ctx);

struct bz_package *
bz_satisfy_dependency_string(const char *dep_string, struct bz_value *ctx);

int
bz_install_dependency_string(const char *dep_string, struct bz_value *ctx);


#endif /* BUZZY_PACKAGE_H */
