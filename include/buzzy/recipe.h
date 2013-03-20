/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_RECIPE_H
#define BUZZY_RECIPE_H

#include <libcork/core.h>

#include "buzzy/callbacks.h"
#include "buzzy/env.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Recipes
 */

struct bz_recipe;

typedef int
(*bz_recipe_build_f)(void *user_data, struct bz_env *env);

typedef int
(*bz_recipe_test_f)(void *user_data, struct bz_env *env);

typedef int
(*bz_recipe_stage_f)(void *user_data, struct bz_env *env);

struct bz_recipe *
bz_recipe_new(struct bz_env *env, const char *builder_name,
              struct bz_action *source_action,
              void *user_data, bz_free_f user_data_free,
              bz_recipe_build_f build,
              bz_recipe_test_f test,
              bz_recipe_stage_f stage);

void
bz_recipe_free(struct bz_recipe *recipe);

/* The recipe is responsible for freeing all of the following actions. */

struct bz_action *
bz_recipe_build_action(struct bz_recipe *recipe);

struct bz_action *
bz_recipe_test_action(struct bz_recipe *recipe);

struct bz_action *
bz_recipe_stage_action(struct bz_recipe *recipe);


#endif /* BUZZY_RECIPE_H */
