/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_BUILDERS_CMAKE_H
#define BUZZY_BUILDERS_CMAKE_H

#include <libcork/core.h>

#include "buzzy/action.h"
#include "buzzy/env.h"
#include "buzzy/recipe.h"


struct bz_recipe *
bz_cmake_new(struct bz_env *env, struct bz_action *source_action);


#endif /* BUZZY_BUILDERS_CMAKE_H */
