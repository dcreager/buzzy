/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2011-2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef TESTS_HELPERS_H
#define TESTS_HELPERS_H

#include <string.h>

#include "libcork/core.h"
#include "libcork/os.h"

#include "buzzy/action.h"
#include "buzzy/mock.h"
#include "buzzy/os.h"
#include "buzzy/version.h"

#if !defined(PRINT_EXPECTED_FAILURES)
#define PRINT_EXPECTED_FAILURES  1
#endif

#if PRINT_EXPECTED_FAILURES
#define print_expected_failure() \
    printf("[expected: %s]\n", cork_error_message());
#else
#define print_expected_failure()  /* do nothing */
#endif


#define DESCRIBE_TEST \
    fprintf(stderr, "--- %s\n", __func__);


#define fail_if_error(call) \
    do { \
        call; \
        if (cork_error_occurred()) { \
            fail("%s", cork_error_message()); \
        } \
    } while (0)

#define fail_unless_error(call, ...) \
    do { \
        call; \
        if (!cork_error_occurred()) { \
            fail(__VA_ARGS__); \
        } else { \
            print_expected_failure(); \
        } \
        cork_error_clear(); \
    } while (0)

#define fail_unless_equal(what, format, expected, actual) \
    (fail_unless((expected) == (actual), \
                 "%s not equal (expected " format \
                 ", got " format ")", \
                 (what), (expected), (actual)))

#define fail_unless_streq(what, expected, actual) \
    (fail_unless(strcmp((expected) == NULL? "": (expected), \
                        (actual) == NULL? "": (actual)) == 0, \
                 "%s not equal (expected \"%s\", got \"%s\")", \
                 (char *) (what), (char *) (expected), (char *) (actual)))


static void
initialize_tests(void)
{
    /* Set the XDG directory variables so that we have reproducible filenames in
     * our test cases. */
    cork_env_add(NULL, "HOME", "/home/test");
    cork_env_add(NULL, "XDG_RUNTIME_DIR", "/run/users/test");
    cork_env_remove(NULL, "XDG_CACHE_HOME");
    cork_env_remove(NULL, "XDG_CACHE_DIRS");
    cork_env_remove(NULL, "XDG_DATA_HOME");
    cork_env_remove(NULL, "XDG_DATA_DIRS");
}


CORK_ATTR_UNUSED
static void
test_and_free_version(struct bz_version *version, const char *expected)
{
    fail_unless_streq("Versions", expected, bz_version_to_string(version));
    bz_version_free(version);
}


CORK_ATTR_UNUSED
static void
verify_commands_run(const char *expected_commands)
{
    fail_unless_streq("Executed commands", expected_commands,
                      bz_mocked_commands_run());
}


CORK_ATTR_UNUSED
static void
test_action_phase_flags(struct bz_action_phase *phase,
                        const char *expected_actions, unsigned int flags)
{
    bz_mocked_actions_clear();
    fail_if_error(bz_action_phase_perform(phase, flags));
    fail_unless_streq("Action list", expected_actions,
                      bz_mocked_actions_run());
}

CORK_ATTR_UNUSED
static void
test_action_phase(struct bz_action_phase *phase, const char *expected_actions)
{
    test_action_phase_flags(phase, expected_actions, 0);
}


CORK_ATTR_UNUSED
static void
test_action(struct bz_action *action, const char *expected_actions)
{
    struct bz_action_phase  *phase;
    bz_mocked_actions_clear();
    phase = bz_action_phase_new("Test actions");
    bz_action_phase_add(phase, action);
    test_action_phase(phase, expected_actions);
    bz_action_phase_free(phase);
}


#endif /* TESTS_HELPERS_H */
