/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_BUILDERS_H
#define BUZZY_BUILDERS_H

#include <libcork/core.h>
#include <libcork/os.h>

#include "buzzy/action.h"
#include "buzzy/env.h"
#include "buzzy/recipe.h"


/*-----------------------------------------------------------------------
 * Detect builders
 */

typedef struct bz_recipe *
(*bz_builder_new_f)(struct bz_env *env, struct bz_action *stage_action);

struct bz_value_provider *
bz_builder_detector_new(void);


#endif /* BUZZY_BUILDERS_H */
