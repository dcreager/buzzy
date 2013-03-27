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
 * Hash tables of variables
 */

static void
var_table_add_string(struct bz_var_table *table, const char *key,
                     const char *value)
{
    struct bz_value_provider  *provider;
    fail_if_error(provider = bz_string_value_new(value));
    fail_if_error(bz_var_table_add(table, key, provider));
}

static void
test_var_table(struct bz_var_table *table, const char *key,
               const char *expected)
{
    const char  *actual;
    fail_if_error(actual = bz_var_table_get(table, key, NULL));
    fail_unless_streq("Environment variable values", expected, actual);
}

static void
test_var_table_missing(struct bz_var_table *table, const char *key)
{
    const char  *actual;
    fail_if_error(actual = bz_var_table_get(table, key, NULL));
    fail_unless(actual == NULL, "Unexpected value for %s", key);
}

static void
test_value_set(struct bz_value_set *set, const char *key,
               const char *expected)
{
    const char  *actual;
    fail_if_error(actual = bz_value_set_get(set, key, NULL));
    fail_unless_streq("Environment variable values", expected, actual);
}

static void
test_value_set_missing(struct bz_value_set *set, const char *key)
{
    const char  *actual;
    fail_if_error(actual = bz_value_set_get(set, key, NULL));
    fail_unless(actual == NULL, "Unexpected value for %s", key);
}

START_TEST(test_var_table_01)
{
    DESCRIBE_TEST;
    struct bz_var_table  *table = bz_var_table_new("test");
    struct bz_value_set  *set = bz_var_table_as_set(table);
    test_var_table_missing(table, "a");
    test_value_set_missing(set,   "a");
    var_table_add_string(table, "a", "");
    var_table_add_string(table, "b", "hello");
    test_var_table(table, "a", "");
    test_value_set(set,   "a", "");
    test_var_table(table, "b", "hello");
    test_value_set(set,   "b", "hello");
    bz_value_set_free(set);
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

    TCase  *tc_var_table = tcase_create("var-table");
    tcase_add_test(tc_var_table, test_var_table_01);
    suite_add_tcase(s, tc_var_table);

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
