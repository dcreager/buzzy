/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <check.h>

#include "buzzy/action.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Test actions
 */

struct test_action {
    const char  *message;
    bool  needed;
};

static void
test_action__free(void *user_data)
{
    struct test_action  *action = user_data;
    cork_strfree(action->message);
    free(action);
}

static void
test_action__message(void *user_data, struct cork_buffer *dest)
{
    struct test_action  *action = user_data;
    cork_buffer_append_string(dest, action->message);
}

static int
test_action__is_needed(void *user_data, bool *is_needed)
{
    struct test_action  *action = user_data;
    *is_needed = action->needed;
    return 0;
}

static int
test_action__perform(void *user_data)
{
    return 0;
}

static struct bz_action *
test_action_new(const char *message, bool needed)
{
    struct test_action  *action = cork_new(struct test_action);
    action->message = cork_strdup(message);
    action->needed = needed;
    return bz_action_new
        (action, test_action__free,
         test_action__message, test_action__is_needed, test_action__perform);
}


/*-----------------------------------------------------------------------
 * Performing actions
 */

START_TEST(test_actions_01)
{
    DESCRIBE_TEST;
    struct bz_action  *a1 = test_action_new("Alpha", true);
    struct bz_action_phase  *phase = bz_action_phase_new("Running tests");
    bz_action_phase_add(phase, a1);
    test_action_phase(phase,
        "Running tests\n"
        "[1/1] Alpha\n"
    );
    bz_action_phase_free(phase);
    bz_action_free(a1);
}
END_TEST

START_TEST(test_actions_02)
{
    DESCRIBE_TEST;
    struct bz_action  *a1 = test_action_new("Alpha", true);
    struct bz_action  *a2 = test_action_new("Beta", true);
    struct bz_action_phase  *phase = bz_action_phase_new("Running tests");
    bz_action_phase_add(phase, a1);
    bz_action_phase_add(phase, a2);
    test_action_phase(phase,
        "Running tests\n"
        "[1/2] Alpha\n"
        "[2/2] Beta\n"
    );
    bz_action_phase_free(phase);
    bz_action_free(a1);
    bz_action_free(a2);
}
END_TEST

START_TEST(test_actions_03)
{
    DESCRIBE_TEST;
    struct bz_action  *a1 = test_action_new("Alpha", true);
    struct bz_action  *a2 = test_action_new("Beta", true);
    struct bz_action  *a3 = test_action_new("Gamma", true);
    struct bz_action_phase  *phase = bz_action_phase_new("Running tests");
    bz_action_phase_add(phase, a2);
    bz_action_add_pre(a2, a1);
    bz_action_add_post(a2, a3);
    test_action_phase(phase,
        "Running tests\n"
        "[1/3] Alpha\n"
        "[2/3] Beta\n"
        "[3/3] Gamma\n"
    );
    bz_action_phase_free(phase);
    bz_action_free(a1);
    bz_action_free(a2);
    bz_action_free(a3);
}
END_TEST


START_TEST(test_actions_unneeded_01)
{
    DESCRIBE_TEST;
    struct bz_action  *a1 = test_action_new("Alpha", false);
    struct bz_action_phase  *phase = bz_action_phase_new("Running tests");
    bz_action_phase_add(phase, a1);
    test_action_phase(phase,
        "Running tests\n"
    );
    bz_action_phase_free(phase);
    bz_action_free(a1);
}
END_TEST

START_TEST(test_actions_unneeded_02)
{
    DESCRIBE_TEST;
    struct bz_action  *a1 = test_action_new("Alpha", false);
    struct bz_action  *a2 = test_action_new("Beta", true);
    struct bz_action  *a3 = test_action_new("Gamma", true);
    struct bz_action_phase  *phase = bz_action_phase_new("Running tests");
    bz_action_phase_add(phase, a2);
    bz_action_add_pre(a2, a1);
    bz_action_add_post(a2, a3);
    test_action_phase(phase,
        "Running tests\n"
        "[1/2] Beta\n"
        "[2/2] Gamma\n"
    );
    bz_action_phase_free(phase);
    bz_action_free(a1);
    bz_action_free(a2);
    bz_action_free(a3);
}
END_TEST


START_TEST(test_actions_repeated_01)
{
    DESCRIBE_TEST;
    struct bz_action  *a1 = test_action_new("Alpha", true);
    struct bz_action  *a2 = test_action_new("Beta", true);
    struct bz_action  *a3 = test_action_new("Gamma", true);
    struct bz_action_phase  *phase1 = bz_action_phase_new("Running tests");
    struct bz_action_phase  *phase2 = bz_action_phase_new("More tests");

    bz_action_phase_add(phase1, a1);

    bz_action_phase_add(phase2, a2);
    bz_action_add_pre(a2, a1);
    bz_action_add_post(a2, a3);

    test_action_phase(phase1,
        "Running tests\n"
        "[1/1] Alpha\n"
    );

    test_action_phase(phase2,
        "More tests\n"
        "[1/2] Beta\n"
        "[2/2] Gamma\n"
    );

    bz_action_phase_free(phase1);
    bz_action_phase_free(phase2);
    bz_action_free(a1);
    bz_action_free(a2);
    bz_action_free(a3);
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("action");

    TCase  *tc_action = tcase_create("action");
    tcase_add_test(tc_action, test_actions_01);
    tcase_add_test(tc_action, test_actions_02);
    tcase_add_test(tc_action, test_actions_03);
    tcase_add_test(tc_action, test_actions_unneeded_01);
    tcase_add_test(tc_action, test_actions_unneeded_02);
    tcase_add_test(tc_action, test_actions_repeated_01);
    suite_add_tcase(s, tc_action);

    return s;
}


int
main(int argc, const char **argv)
{
    int  number_failed;
    Suite  *suite = test_suite();
    SRunner  *runner = srunner_create(suite);

    initialize_tests();
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
