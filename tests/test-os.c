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

#include "buzzy/os.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Running subprocesses
 */

static void
test_run(int dummy, ...)
{
    va_list  args;
    va_start(args, dummy);
    fail_if_error(bz_subprocess_v_run(true, NULL, args));
    va_end(args);
}

static void
test_output(const char *expected_out, const char *expected_err, ...)
{
    va_list  args;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct cork_buffer  err = CORK_BUFFER_INIT();

    va_start(args, expected_err);
    fail_if_error(bz_subprocess_v_get_output(&out, &err, NULL, args));
    va_end(args);
    if (expected_out != NULL) {
        fail_unless_streq("stdout", expected_out, out.buf);
    }
    if (expected_err != NULL) {
        fail_unless_streq("stderr", expected_err, err.buf);
    }
    cork_buffer_done(&out);
    cork_buffer_done(&err);
}

static void
test_file(const char *filename, const char *content)
{
    struct cork_path  *path = cork_path_new(filename);
    struct cork_file  *file;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    cork_buffer_set_string(&buf, content);
    fail_if_error(file = bz_create_file(path, &buf));
    cork_file_free(file);
    cork_buffer_done(&buf);
}

START_TEST(test_run_mocked_01)
{
    DESCRIBE_TEST;

    bz_start_mocks();
    bz_mock_subprocess("echo hello world", "hello world\n", NULL, 0);
    test_run(0, "echo", "hello", "world", NULL);
    test_output("hello world\n", NULL,
                "echo", "hello", "world", NULL);
    verify_commands_run(
        "$ echo hello world\n"
        "$ echo hello world\n"
    );
}
END_TEST

START_TEST(test_run_01)
{
    DESCRIBE_TEST;

    bz_start_mocks();
    bz_mock_subprocess_allow_execute("echo hello world");
    test_run(0, "echo", "hello", "world", NULL);
    test_output("hello world\n", NULL,
                "echo", "hello", "world", NULL);
    verify_commands_run(
        "$ echo hello world\n"
        "$ echo hello world\n"
    );
}
END_TEST

START_TEST(test_create_file_01)
{
    DESCRIBE_TEST;

    bz_start_mocks();
    test_file("test-file.txt", "hello world\n");
    verify_commands_run(
        "$ cat > test-file.txt <<EOF\n"
        "hello world\n"
        "EOF\n"
    );
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
    tcase_add_test(tc_run, test_create_file_01);
    suite_add_tcase(s, tc_run);

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
