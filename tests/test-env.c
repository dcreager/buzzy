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

#include "buzzy/env.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Simple string values
 */

static void
test_string_value(const char *value)
{
    struct bz_value_provider  *provider;
    const char  *actual;
    fail_if_error(provider = bz_string_value_new(value));
    fail_if_error(actual = bz_value_provider_get(provider, NULL));
    fail_unless_streq("String values", value, actual);
    bz_value_provider_free(provider);
}

START_TEST(test_string_values)
{
    DESCRIBE_TEST;
    test_string_value("");
    test_string_value("abc");
    test_string_value("hello world");
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("env");

    TCase  *tc_providers = tcase_create("providers");
    tcase_add_test(tc_providers, test_string_values);
    suite_add_tcase(s, tc_providers);

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
