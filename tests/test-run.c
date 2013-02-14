/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <check.h>

#include "buzzy/run.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Running subprocesses
 */

static char  *echo_01_params[] = { "echo", "hello", "world", NULL };

static void
test_run(const char *program, char * const *params)
{
    fail_if_error(bz_subprocess_run(program, params, false, NULL));
}

static void
test_output(const char *program, char * const *params,
            const char *expected_out, const char *expected_err)
{
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct cork_buffer  err = CORK_BUFFER_INIT();
    fail_if_error(bz_subprocess_get_output(program, params, &out, &err));
    if (expected_out != NULL) {
        fail_unless_streq("stdout", expected_out, out.buf);
    }
    if (expected_err != NULL) {
        fail_unless_streq("stderr", expected_err, err.buf);
    }
    cork_buffer_done(&out);
    cork_buffer_done(&err);
}

START_TEST(test_run_mocked_01)
{
    DESCRIBE_TEST;

    bz_subprocess_start_mocks();
    bz_subprocess_mock("echo hello world", "hello world\n", NULL, 0);
    test_run("echo", echo_01_params);
    test_output("echo", echo_01_params, "hello world\n", NULL);
}
END_TEST

START_TEST(test_run_01)
{
    DESCRIBE_TEST;

    bz_subprocess_start_mocks();
    bz_subprocess_mock_allow_execute("echo hello world");
    test_run("echo", echo_01_params);
    test_output("echo", echo_01_params, "hello world\n", NULL);
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("run");

    TCase  *tc_run = tcase_create("run");
    tcase_add_test(tc_run, test_run_mocked_01);
    tcase_add_test(tc_run, test_run_01);
    suite_add_tcase(s, tc_run);

    return s;
}


int
main(int argc, const char **argv)
{
    int  number_failed;
    Suite  *suite = test_suite();
    SRunner  *runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
