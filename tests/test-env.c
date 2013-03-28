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
 * Interpolated values
 */

static void
test_good_interpolated_value(const char *template_value, const char *expected)
{
    struct bz_value_provider  *provider;
    struct bz_env  *env = bz_env_new("test");
    fail_if_error(provider = bz_interpolated_value_new(template_value));
    if (expected == NULL) {
        fail_unless_error(bz_value_provider_get(provider, env));
    } else {
        const char  *actual;
        fail_if_error(actual = bz_value_provider_get(provider, env));
        fail_unless_streq("Interpolated values", expected, actual);
    }
    bz_value_provider_free(provider);
    bz_env_free(env);
}

static void
test_bad_interpolated_value(const char *template_value)
{
    fail_unless_error(bz_interpolated_value_new(template_value));
}

START_TEST(test_interpolated_values)
{
    DESCRIBE_TEST;
    test_good_interpolated_value("", "");
    test_good_interpolated_value("abc", "abc");
    test_good_interpolated_value("hello world", "hello world");
    test_good_interpolated_value("${var}", NULL);
    test_good_interpolated_value("${var_with_digit_00}", NULL);
    test_good_interpolated_value("${var_with_underscore}", NULL);
    test_good_interpolated_value("${nested.var}", NULL);
    test_good_interpolated_value("embedded $$ dollar sign",
                                 "embedded $ dollar sign");
    test_bad_interpolated_value("${unclosed");
    test_bad_interpolated_value("${invalid char");
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
var_table_add_interpolated(struct bz_var_table *table, const char *key,
                           const char *template_value)
{
    struct bz_value_provider  *provider;
    fail_if_error(provider = bz_interpolated_value_new(template_value));
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
 * Environments
 */

static void
test_env(struct bz_env *env, const char *key, const char *expected)
{
    const char  *actual;
    fail_if_error(actual = bz_env_get(env, key));
    fail_unless_streq("Environment variable values", expected, actual);
}

static void
test_env_error(struct bz_env *env, const char *key)
{
    fail_unless_error(bz_env_get(env, key));
}

static void
test_env_missing(struct bz_env *env, const char *key)
{
    const char  *actual;
    fail_if_error(actual = bz_env_get(env, key));
    fail_unless(actual == NULL, "Unexpected value for %s", key);
}

START_TEST(test_env_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_var_table  *table1 = bz_var_table_new("test1");
    struct bz_value_set  *set1 = bz_var_table_as_set(table1);
    struct bz_var_table  *table2 = bz_var_table_new("test2");
    struct bz_value_set  *set2 = bz_var_table_as_set(table2);
    struct bz_var_table  *table3 = bz_var_table_new("test3");
    struct bz_value_set  *set3 = bz_var_table_as_set(table3);
    struct bz_var_table  *table4 = bz_var_table_new("test4");
    struct bz_value_set  *set4 = bz_var_table_as_set(table4);

    bz_env_add_set(env, set1);
    bz_env_add_set(env, set2);
    bz_env_add_backup_set(env, set3);
    bz_env_add_backup_set(env, set4);

    test_env_missing(env, "a");
    var_table_add_string(table1, "a", "");
    test_env(env, "a", "");

    test_env_missing(env, "b");
    var_table_add_string(table4, "b", "backup");
    test_env(env, "b", "backup");
    var_table_add_string(table3, "b", "backup 2");
    test_env(env, "b", "backup 2");
    var_table_add_string(table2, "b", "hello");
    test_env(env, "b", "hello");
    var_table_add_string(table1, "b", "world");
    test_env(env, "b", "world");

    bz_env_free(env);
}
END_TEST


START_TEST(test_env_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_var_table  *table1 = bz_var_table_new("test1");
    struct bz_value_set  *set1 = bz_var_table_as_set(table1);

    bz_env_add_set(env, set1);

    var_table_add_string(table1, "a", "hello");
    var_table_add_interpolated(table1, "b", "${a} world");
    var_table_add_interpolated(table1, "c", "${a} ${b}");
    var_table_add_interpolated(table1, "d", "${missing}");
    test_env(env, "a", "hello");
    test_env(env, "b", "hello world");
    test_env(env, "c", "hello hello world");
    test_env_error(env, "d");

    bz_env_free(env);
}
END_TEST


static void
set_global_default(const char *key, const char *template_value)
{
    struct bz_value_provider  *provider;
    fail_if_error(provider = bz_interpolated_value_new(template_value));
    fail_if_error(bz_env_set_global_default(key, provider));
}

START_TEST(test_global_env_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    bz_global_env_reset();
    env = bz_global_env();
    test_env_missing(env, "a");
    set_global_default("a", "${b} value");
    test_env_error(env, "a");
    set_global_default("b", "test");
    test_env(env, "b", "test");
    test_env(env, "a", "test value");
}
END_TEST

START_TEST(test_package_env_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_var_table  *table1 = bz_var_table_new("test1");
    struct bz_value_set  *set1 = bz_var_table_as_set(table1);

    bz_global_env_reset();
    env = bz_package_env_new("test");
    test_env_missing(env, "a");
    set_global_default("a", "${b} value");
    test_env_error(env, "a");
    set_global_default("b", "test");
    test_env(env, "b", "test");
    test_env(env, "a", "test value");

    bz_env_add_set(env, set1);
    var_table_add_string(table1, "b", "overridden");
    test_env(env, "b", "overridden");
    test_env(env, "a", "overridden value");

    bz_env_free(env);
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
    tcase_add_test(tc_providers, test_interpolated_values);
    suite_add_tcase(s, tc_providers);

    TCase  *tc_var_table = tcase_create("var-table");
    tcase_add_test(tc_var_table, test_var_table_01);
    suite_add_tcase(s, tc_var_table);

    TCase  *tc_env = tcase_create("env");
    tcase_add_test(tc_env, test_env_01);
    tcase_add_test(tc_env, test_env_02);
    tcase_add_test(tc_env, test_global_env_01);
    tcase_add_test(tc_env, test_package_env_01);
    suite_add_tcase(s, tc_env);

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
