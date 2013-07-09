/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_BUILT_H
#define BUZZY_BUILT_H

#include <libcork/core.h>

#include "buzzy/env.h"
#include "buzzy/package.h"


/* If we have to build a package from source, as opposed to being able to
 * install it from some native package repository, then there are three
 * separate, orthogonal steps:
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
bz_packager_set_builder(struct bz_packager *packager,
                        struct bz_builder *builder);

int
bz_packager_package(struct bz_packager *packager);

int
bz_packager_install(struct bz_packager *packager);

int
bz_packager_uninstall(struct bz_packager *packager);


struct bz_packager *
bz_package_packager_new(struct bz_env *env);


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


#endif /* BUZZY_BUILT_H */
