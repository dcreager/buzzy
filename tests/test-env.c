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
#include "buzzy/value.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Simple string values
 */

static void
test_string_value(const char *content)
{
    struct bz_value  *value;
    const char  *actual;
    fail_if_error(value = bz_string_value_new(content));
    fail_if_error(actual = bz_value_get_string(value, NULL));
    fail_unless_streq("String values", content, actual);
    bz_value_free(value);
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
    struct bz_value  *value;
    fail_if_error(value = bz_interpolated_value_new(template_value));
    if (expected == NULL) {
        fail_unless_error(bz_value_get_string(value, NULL));
    } else {
        const char  *actual;
        fail_if_error(actual = bz_value_get_string(value, NULL));
        fail_unless_streq("Interpolated values", expected, actual);
    }
    bz_value_free(value);
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
 * Hash maps of variables
 */

static void
map_add_string(struct bz_value *map, const char *key, const char *content)
{
    struct bz_value  *value;
    fail_if_error(value = bz_string_value_new(content));
    fail_if_error(bz_map_value_add(map, key, value, true));
}

static void
map_add_interpolated(struct bz_value *map, const char *key,
                     const char *template_value)
{
    struct bz_value  *value;
    fail_if_error(value = bz_interpolated_value_new(template_value));
    fail_if_error(bz_map_value_add(map, key, value, true));
}

static void
test_value(struct bz_value *value, const char *key, const char *expected)
{
    const char  *actual;
    fail_if_error(actual = bz_value_get_string(value, key));
    fail_unless_streq("Environment variable values", expected, actual);
}

static void
test_value_missing(struct bz_value *value, const char *key)
{
    fail_unless_error(bz_value_get_string(value, key));
}

START_TEST(test_map_01)
{
    DESCRIBE_TEST;
    struct bz_value  *map = bz_map_new();
    test_value_missing(map, "a");
    map_add_string(map, "a", "");
    map_add_string(map, "b", "hello");
    test_value(map, "a", "");
    test_value(map, "b", "hello");
    bz_value_free(map);
}
END_TEST


/*-----------------------------------------------------------------------
 * Environments
 */

static void
env_add_string(struct bz_env *env, const char *key, const char *content)
{
    struct bz_value  *value;
    fail_if_error(value = bz_string_value_new(content));
    fail_if_error(bz_env_add_override(env, key, value));
}

static void
test_env(struct bz_env *env, const char *key, const char *expected)
{
    const char  *actual;
    fail_if_error(actual = bz_env_get_string(env, key));
    fail_unless_streq("Environment variable values", expected, actual);
}

static void
test_env_error(struct bz_env *env, const char *key)
{
    fail_unless_error(bz_env_get_string(env, key));
}

static void
test_env_missing(struct bz_env *env, const char *key)
{
    fail_unless_error(bz_env_get_string(env, key));
}


START_TEST(test_env_override_00)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();
    struct bz_value  *map2 = bz_map_new();
    struct bz_value  *map3 = bz_map_new();
    struct bz_value  *map4 = bz_map_new();
    bz_env_add_set(env, map1);
    bz_env_add_set(env, map2);
    bz_env_add_backup_set(env, map3);
    bz_env_add_backup_set(env, map4);
    test_env_missing(env, "b");
    bz_env_free(env);
}
END_TEST

START_TEST(test_env_override_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();
    struct bz_value  *map2 = bz_map_new();
    struct bz_value  *map3 = bz_map_new();
    struct bz_value  *map4 = bz_map_new();
    bz_env_add_set(env, map1);
    bz_env_add_set(env, map2);
    bz_env_add_backup_set(env, map3);
    bz_env_add_backup_set(env, map4);
    map_add_string(map4, "b", "backup");
    test_env(env, "b", "backup");
    bz_env_free(env);
}
END_TEST

START_TEST(test_env_override_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();
    struct bz_value  *map2 = bz_map_new();
    struct bz_value  *map3 = bz_map_new();
    struct bz_value  *map4 = bz_map_new();
    bz_env_add_set(env, map1);
    bz_env_add_set(env, map2);
    bz_env_add_backup_set(env, map3);
    bz_env_add_backup_set(env, map4);
    map_add_string(map4, "b", "backup");
    map_add_string(map3, "b", "backup 2");
    test_env(env, "b", "backup 2");
    bz_env_free(env);
}
END_TEST

START_TEST(test_env_override_03)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();
    struct bz_value  *map2 = bz_map_new();
    struct bz_value  *map3 = bz_map_new();
    struct bz_value  *map4 = bz_map_new();
    bz_env_add_set(env, map1);
    bz_env_add_set(env, map2);
    bz_env_add_backup_set(env, map3);
    bz_env_add_backup_set(env, map4);
    map_add_string(map4, "b", "backup");
    map_add_string(map3, "b", "backup 2");
    map_add_string(map2, "b", "hello");
    test_env(env, "b", "hello");
    bz_env_free(env);
}
END_TEST

START_TEST(test_env_override_04)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();
    struct bz_value  *map2 = bz_map_new();
    struct bz_value  *map3 = bz_map_new();
    struct bz_value  *map4 = bz_map_new();
    bz_env_add_set(env, map1);
    bz_env_add_set(env, map2);
    bz_env_add_backup_set(env, map3);
    bz_env_add_backup_set(env, map4);
    map_add_string(map4, "b", "backup");
    map_add_string(map3, "b", "backup 2");
    map_add_string(map2, "b", "hello");
    map_add_string(map1, "b", "world");
    test_env(env, "b", "world");
    bz_env_free(env);
}
END_TEST

START_TEST(test_env_override_05)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();
    struct bz_value  *map2 = bz_map_new();
    struct bz_value  *map3 = bz_map_new();
    struct bz_value  *map4 = bz_map_new();
    bz_env_add_set(env, map1);
    bz_env_add_set(env, map2);
    bz_env_add_backup_set(env, map3);
    bz_env_add_backup_set(env, map4);
    map_add_string(map4, "b", "backup");
    map_add_string(map3, "b", "backup 2");
    map_add_string(map2, "b", "hello");
    map_add_string(map1, "b", "world");
    env_add_string(env, "b", "overridden");
    test_env(env, "b", "overridden");
    bz_env_free(env);
}
END_TEST


START_TEST(test_env_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();

    bz_env_add_set(env, map1);

    map_add_string(map1, "a", "hello");
    map_add_interpolated(map1, "b", "${a} world");
    map_add_interpolated(map1, "c", "${a} ${b}");
    map_add_interpolated(map1, "d", "${missing}");
    test_env(env, "a", "hello");
    test_env(env, "b", "hello world");
    test_env(env, "c", "hello hello world");
    test_env_error(env, "d");

    bz_env_free(env);
}
END_TEST


static void
test_env_path(struct bz_env *env, const char *key, const char *expected)
{
    struct cork_path  *path;
    const char  *actual;
    fail_if_error(path = bz_env_get_path(env, key));
    actual = cork_path_get(path);
    fail_unless_streq("Environment variable values", expected, actual);
}

START_TEST(test_env_path_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *map1 = bz_map_new();

    bz_env_add_set(env, map1);
    bz_env_set_base_path(env, "/base/path");

    map_add_string(map1, "path1", "a/b");
    map_add_string(map1, "path2", "/absolute/a/b");
    test_env(env, "path1", "a/b");
    test_env_path(env, "path1", "/base/path/a/b");
    test_env(env, "path2", "/absolute/a/b");
    test_env_path(env, "path2", "/absolute/a/b");

    bz_env_free(env);
}
END_TEST


static const char  YAML_01[] =
    "a: hello\n"
    "b: ${a} world\n"
    "c: ${a} ${b}\n"
    "d: ${missing}\n"
    "e: [1,2,3]\n"
    "f:\n"
    "  a: nested\n"
    "  b:\n"
    "    c: ${f.a} more\n"
    ;

START_TEST(test_env_yaml_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env = bz_env_new("test");
    struct bz_value  *value1;

    fail_if_error(value1 = bz_yaml_value_new_from_string(YAML_01));
    bz_env_add_set(env, value1);

    test_env(env, "a", "hello");
    test_env(env, "b", "hello world");
    test_env(env, "c", "hello hello world");
    test_env_error(env, "d");
    test_env_error(env, "e");
    test_env_error(env, "a.b");
    test_env(env, "f.a", "nested");
    test_env_error(env, "f.b");
    test_env(env, "f.b.c", "nested more");
    test_env_missing(env, "missing");

    bz_env_free(env);
}
END_TEST


static void
value_global_default(const char *key, const char *template_value)
{
    struct bz_value  *value;
    fail_if_error(value = bz_interpolated_value_new(template_value));
    fail_if_error(bz_env_set_global_default(key, value, NULL, NULL));
}

START_TEST(test_global_env_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    bz_global_env_reset();
    env = bz_global_env();
    value_global_default("a", "${b} value");
    test_env_error(env, "a");
}
END_TEST

START_TEST(test_global_env_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    bz_global_env_reset();
    env = bz_global_env();
    value_global_default("a", "${b} value");
    value_global_default("b", "test");
    test_env(env, "b", "test");
    test_env(env, "a", "test value");
}
END_TEST


START_TEST(test_package_env_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    bz_global_env_reset();
    env = bz_package_env_new_empty(NULL, "test");
    value_global_default("a", "${b} value");
    test_env_error(env, "a");
    bz_env_free(env);
}
END_TEST

START_TEST(test_package_env_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    bz_global_env_reset();
    env = bz_package_env_new_empty(NULL, "test");
    value_global_default("a", "${b} value");
    value_global_default("b", "test");
    test_env(env, "b", "test");
    test_env(env, "a", "test value");
    bz_env_free(env);
}
END_TEST

START_TEST(test_package_env_03)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_value  *map1 = bz_map_new();
    bz_global_env_reset();
    env = bz_package_env_new_empty(NULL, "test");
    value_global_default("a", "${b} value");
    value_global_default("b", "test");
    bz_env_add_set(env, map1);
    map_add_string(map1, "b", "overridden");
    test_env(env, "b", "overridden");
    test_env(env, "a", "overridden value");
    bz_env_free(env);
}
END_TEST


/*-----------------------------------------------------------------------
 * Built-in variables
 */

START_TEST(test_builtin_vars_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    bz_global_env_reset();
    fail_if_error(bz_load_variable_definitions());
    env = bz_global_env();
    test_env(env, "cache_dir", "/home/test/.cache");
    test_env(env, "work_dir", "/home/test/.cache/buzzy");
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("env");

    TCase  *tc_values = tcase_create("values");
    tcase_add_test(tc_values, test_string_values);
    tcase_add_test(tc_values, test_interpolated_values);
    suite_add_tcase(s, tc_values);

    TCase  *tc_map = tcase_create("map");
    tcase_add_test(tc_map, test_map_01);
    suite_add_tcase(s, tc_map);

    TCase  *tc_env = tcase_create("env");
    tcase_add_test(tc_env, test_env_override_00);
    tcase_add_test(tc_env, test_env_override_01);
    tcase_add_test(tc_env, test_env_override_02);
    tcase_add_test(tc_env, test_env_override_03);
    tcase_add_test(tc_env, test_env_override_04);
    tcase_add_test(tc_env, test_env_override_05);
    tcase_add_test(tc_env, test_env_01);
    tcase_add_test(tc_env, test_env_path_01);
    tcase_add_test(tc_env, test_env_yaml_01);
    tcase_add_test(tc_env, test_global_env_01);
    tcase_add_test(tc_env, test_global_env_02);
    tcase_add_test(tc_env, test_package_env_01);
    tcase_add_test(tc_env, test_package_env_02);
    tcase_add_test(tc_env, test_package_env_03);
    suite_add_tcase(s, tc_env);

    TCase  *tc_builtin_vars = tcase_create("builtin-vars");
    tcase_add_test(tc_builtin_vars, test_builtin_vars_01);
    suite_add_tcase(s, tc_builtin_vars);

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
