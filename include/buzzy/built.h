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

#include "buzzy/callbacks.h"
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
bz_build_message(struct cork_buffer *dest, struct bz_env *env,
                 const char *builder_name);

int
bz_test_message(struct cork_buffer *dest, struct bz_env *env,
                const char *builder_name);

int
bz_stage_message(struct cork_buffer *dest, struct bz_env *env,
                 const char *builder_name);


int
bz_package_message(struct cork_buffer *dest, struct bz_env *env,
                   const char *packager_name);

int
bz_install_message(struct cork_buffer *dest, struct bz_env *env,
                   const char *packager_name);


/*-----------------------------------------------------------------------
 * Builders
 */

struct bz_builder;

struct bz_builder *
bz_builder_new(struct bz_env *env, const char *builder_name,
               struct bz_action *build,
               struct bz_action *test,
               struct bz_action *stage);

void
bz_builder_free(struct bz_builder *builder);

void
bz_builder_add_prereq(struct bz_builder *builder, struct bz_action *action);

int
bz_builder_add_prereq_package(struct bz_builder *builder, const char *dep);

/* The builder is responsible for freeing all of the following actions. */

struct bz_action *
bz_builder_build_action(struct bz_builder *builder);

struct bz_action *
bz_builder_test_action(struct bz_builder *builder);

struct bz_action *
bz_builder_stage_action(struct bz_builder *builder);


struct bz_builder *
bz_package_builder_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Packagers
 */

struct bz_packager;

struct bz_packager *
bz_packager_new(struct bz_env *env, const char *packager_name,
                struct bz_action *package,
                struct bz_action *install);

void
bz_packager_free(struct bz_packager *packager);

void
bz_packager_add_prereq(struct bz_packager *packager, struct bz_action *action);

int
bz_packager_add_prereq_package(struct bz_packager *packager, const char *dep);

/* The packager is responsible for freeing all of the following actions. */

struct bz_action *
bz_packager_package_action(struct bz_packager *packager);

struct bz_action *
bz_packager_install_action(struct bz_packager *packager);


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

struct bz_value_provider *
bz_builder_detector_new(void);

struct bz_builder *
bz_noop_builder_new(struct bz_env *env);

struct bz_builder *
bz_cmake_builder_new(struct bz_env *env);


/*-----------------------------------------------------------------------
 * Available packagers
 */

struct bz_value_provider *
bz_packager_detector_new(void);

struct bz_packager *
bz_noop_packager_new(struct bz_env *env);

struct bz_packager *
bz_pacman_packager_new(struct bz_env *env);


#endif /* BUZZY_BUILT_H */
