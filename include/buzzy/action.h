/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_ACTION_H
#define BUZZY_ACTION_H

#include <libcork/core.h>
#include <libcork/ds.h>

#include "buzzy/callbacks.h"


/*-----------------------------------------------------------------------
 * Actions
 */

typedef void
(*bz_action_message_f)(void *user_data, struct cork_buffer *dest);

typedef int
(*bz_action_is_needed_f)(void *user_data, bool *is_needed);

typedef int
(*bz_action_perform_f)(void *user_data);


struct bz_action;

struct bz_action *
bz_action_new(void *user_data, bz_user_data_free_f user_data_free,
              bz_action_message_f message,
              bz_action_is_needed_f is_needed,
              bz_action_perform_f perform);

void
bz_action_free(struct bz_action *action);

void
bz_action_add_pre(struct bz_action *action, struct bz_action *pre);

void
bz_action_add_post(struct bz_action *action, struct bz_action *post);


/*-----------------------------------------------------------------------
 * Phases
 */

struct bz_action_phase;

struct bz_action_phase *
bz_action_phase_new(const char *message);

void
bz_action_phase_free(struct bz_action_phase *phase);

void
bz_action_phase_add(struct bz_action_phase *phase, struct bz_action *action);

int
bz_action_phase_perform(struct bz_action_phase *phase);


#endif /* BUZZY_ACTION_H */
