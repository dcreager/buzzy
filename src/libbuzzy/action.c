/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/error.h"


#if !defined(BZ_DEBUG_ACTIONS)
#define BZ_DEBUG_ACTIONS  0
#endif

#if BZ_DEBUG_ACTIONS
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * Actions
 */

typedef cork_array(struct bz_action *)  bz_action_array;

typedef int  tribool;
#define unknown  (-1)

struct bz_action {
    void  *user_data;
    bz_user_data_free_f  user_data_free;
    bz_action_message_f  message;
    bz_action_is_needed_f  is_needed;
    bz_action_perform_f  perform;
    bz_action_array  pre_actions;
    bz_action_array  post_actions;
    tribool  needed;
    bool  performing;
    bool  performed;
};

struct bz_action *
bz_action_new(void *user_data, bz_user_data_free_f user_data_free,
              bz_action_message_f message,
              bz_action_is_needed_f is_needed,
              bz_action_perform_f perform)
{
    struct bz_action  *action = cork_new(struct bz_action);
    action->user_data = user_data;
    action->user_data_free = user_data_free;
    action->message = message;
    action->is_needed = is_needed;
    action->perform = perform;
    cork_array_init(&action->pre_actions);
    cork_array_init(&action->post_actions);
    action->needed = unknown;
    action->performing = false;
    action->performed = false;
    return action;
}

void
bz_action_free(struct bz_action *action)
{
    if (action->user_data_free != NULL) {
        action->user_data_free(action->user_data);
    }
    cork_array_done(&action->pre_actions);
    cork_array_done(&action->post_actions);
    free(action);
}

void
bz_action_add_pre(struct bz_action *action, struct bz_action *pre)
{
    assert(!action->performing);
    assert(!action->performed);
    cork_array_append(&action->pre_actions, pre);
}

void
bz_action_add_post(struct bz_action *action, struct bz_action *post)
{
    assert(!action->performing);
    assert(!action->performed);
    cork_array_append(&action->post_actions, post);
}

static int
bz_action_is_needed(struct bz_action *action)
{
    bool  is_needed;
    assert(action->needed == unknown);
    rii_check(action->is_needed(action->user_data, &is_needed));
    action->needed = is_needed;
    return 0;
}


/*-----------------------------------------------------------------------
 * Mocking actions for test cases
 */

static bool  mocks_enabled = false;
static struct cork_buffer  mock_results;

static void
free_mock_results(void)
{
    if (mocks_enabled) {
        cork_buffer_done(&mock_results);
        mocks_enabled = false;
    }
}

typedef void
(*action_printer)(struct cork_buffer *buf);

static void
bz_action_print(struct cork_buffer *buf)
{
    printf("%s\n", (char *) buf->buf);
}

static void
bz_action_mock_print(struct cork_buffer *buf)
{
    cork_buffer_append(&mock_results, buf->buf, buf->size);
    cork_buffer_append(&mock_results, "\n", 1);
}

/* Start by using the "real" printer */
static action_printer  printer = bz_action_print;

void
bz_action_start_mocks(void)
{
    /* Free any existing mocks first. */
    free_mock_results();

    cork_buffer_init(&mock_results);
    cork_buffer_append(&mock_results, "", 0);
    mocks_enabled = true;
    printer = bz_action_mock_print;
}

const char *
bz_action_mock_results(void)
{
    assert(mocks_enabled);
    return mock_results.buf;
}


/*-----------------------------------------------------------------------
 * Phases
 */

struct bz_action_phase {
    const char  *message;
    struct cork_buffer  buf;
    bz_action_array  actions;
    size_t  action_count;
    size_t  action_index;
    size_t  count_length;
};

struct bz_action_phase *
bz_action_phase_new(const char *message)
{
    struct bz_action_phase  *phase = cork_new(struct bz_action_phase);
    phase->message = cork_strdup(message);
    cork_buffer_init(&phase->buf);
    cork_array_init(&phase->actions);
    phase->action_count = 0;
    return phase;
}

void
bz_action_phase_free(struct bz_action_phase *phase)
{
    cork_strfree(phase->message);
    cork_buffer_done(&phase->buf);
    cork_array_done(&phase->actions);
    free(phase);
}

void
bz_action_phase_add(struct bz_action_phase *phase, struct bz_action *action)
{
    cork_array_append(&phase->actions, action);
}


static int
bz_action_phase_count_actions(struct bz_action_phase *phase,
                              bz_action_array *actions);

static int
bz_action_phase_count_action(struct bz_action_phase *phase,
                             struct bz_action *action)
{
    if (action->performed) {
        /* We already performed this action in some previous phase, so it
         * shouldn't be counted. */
        DEBUG("  [already performed %p]\n", action);
        return 0;
    }

    if (action->needed != unknown) {
        /* If we've already checked whether this action is needed, then we've
         * encountered it at some other pointing during the DFS of this phase's
         * actions.  So it shouldn't be counted a second time. */
        DEBUG("  [already checked %p]\n", action);
        return 0;
    }

    /* Check whether this action needs to be performed. */
    rii_check(bz_action_is_needed(action));
    if (action->needed) {
        DEBUG("  new action %p\n", action);
        phase->action_count++;

        /* If this action is needed, then check whether its pre- and
         * post-actions are also needed. */
        rii_check(bz_action_phase_count_actions(phase, &action->pre_actions));
        rii_check(bz_action_phase_count_actions(phase, &action->post_actions));
    } else {
        DEBUG("  [not needed %p]\n", action);
    }

    return 0;
}

static int
bz_action_phase_count_actions(struct bz_action_phase *phase,
                              bz_action_array *actions)
{
    size_t  i;
    for (i = 0; i < cork_array_size(actions); i++) {
        struct bz_action  *action = cork_array_at(actions, i);
        rii_check(bz_action_phase_count_action(phase, action));
    }
    return 0;
}


static int
bz_action_phase_perform_actions(struct bz_action_phase *phase,
                                bz_action_array *actions);

static void
bz_action_phase_print_action(struct bz_action_phase *phase,
                             struct bz_action *action)
{
    phase->action_index++;
    cork_buffer_printf(&phase->buf, "[%*zu/%zu] ",
                       (int) phase->count_length, phase->action_index,
                       phase->action_count);
    action->message(action->user_data, &phase->buf);
    printer(&phase->buf);
}

static int
bz_action_phase_perform_action(struct bz_action_phase *phase,
                               struct bz_action *action)
{
    if (action->performed) {
        /* We already performed this action at some point. */
        DEBUG("  [already performed %p]\n", action);
        return 0;
    }

    if (CORK_UNLIKELY(action->performing)) {
        bz_circular_actions("Circular actions!");
        return -1;
    }

    /* We should have already filled this in when counting the actions. */
    assert(action->needed != unknown);

    if (action->needed) {
        action->performing = true;
        rii_check(bz_action_phase_perform_actions(phase, &action->pre_actions));
        DEBUG("  performing %p\n", action);
        bz_action_phase_print_action(phase, action);
        rii_check(action->perform(action->user_data));
        action->performed = true;
        return bz_action_phase_perform_actions(phase, &action->post_actions);
    } else {
        return 0;
    }
}

static int
bz_action_phase_perform_actions(struct bz_action_phase *phase,
                                bz_action_array *actions)
{
    size_t  i;
    for (i = 0; i < cork_array_size(actions); i++) {
        struct bz_action  *action = cork_array_at(actions, i);
        rii_check(bz_action_phase_perform_action(phase, action));
    }
    return 0;
}


int
bz_action_phase_perform(struct bz_action_phase *phase)
{
    DEBUG("Counting actions for phase \"%s\"\n", phase->message);
    phase->action_count = 0;
    phase->action_index = 0;
    rii_check(bz_action_phase_count_actions(phase, &phase->actions));
    cork_buffer_printf(&phase->buf, "%zu", phase->action_count);
    phase->count_length = phase->buf.size;

    DEBUG("Performing actions for phase \"%s\"\n", phase->message);
    cork_buffer_set_string(&phase->buf, phase->message);
    printer(&phase->buf);
    return bz_action_phase_perform_actions(phase, &phase->actions);
}
