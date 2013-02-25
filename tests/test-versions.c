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

#include "buzzy/version.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Version numbers
 */

static void
test_version_string(const char *in, const char *expected)
{
    struct bz_version  *version;
    fail_if_error(version = bz_version_from_string(in));
    test_and_free_version(version, expected);
}

static void
test_version_compare_string(const char *in, const char *expected)
{
    struct bz_version  *version;
    fail_if_error(version = bz_version_from_string(in));
    fail_unless_streq("Versions", expected,
                      bz_version_to_compare_string(version));
    bz_version_free(version);
}

static void
test_version_cmp(const char *in1, const char *in2, int expected)
{
    int  actual;
    struct bz_version  *v1;
    struct bz_version  *v2;
    fail_if_error(v1 = bz_version_from_string(in1));
    fail_if_error(v2 = bz_version_from_string(in2));
    fail_if_error(actual = bz_version_cmp(v1, v2));
    bz_version_free(v1);
    bz_version_free(v2);
    if (expected == -1) {
        fail_unless(actual < 0, "%s should be less than %s", in1, in2);
    } else if (expected == 1) {
        fail_unless(actual > 0, "%s should be greater than %s", in1, in2);
    } else {
        fail_unless(actual == 0, "%s should be equal to %s", in1, in2);
    }
}

START_TEST(test_versions)
{
    DESCRIBE_TEST;

    test_version_string("2.0",       "2.0");
    test_version_string("2.0.0",     "2.0.0");
    test_version_string("2.0~alpha", "2.0~alpha");
    test_version_string("2.0+dev",   "2.0+dev");
    test_version_string("2.0.1",     "2.0.1");
    test_version_string("2.1",       "2.1");

    test_version_compare_string("2.0",       "[.2]");
    test_version_compare_string("2.0.0",     "[.2]");
    test_version_compare_string("2.0~alpha", "[.2~alpha]");
    test_version_compare_string("2.0+dev",   "[.2+dev]");
    test_version_compare_string("2.0.1",     "[.2.0.1]");
    test_version_compare_string("2.1",       "[.2.1]");

    /* Basic version comparisons */
    test_version_cmp("2.0",   "2.0.0",  0);
    test_version_cmp("2.0",   "2.0.1", -1);
    test_version_cmp("2.0",   "2.1",   -1);
    test_version_cmp("2.0.0", "2.0.1", -1);
    test_version_cmp("2.0.0", "2.1",   -1);
    test_version_cmp("2.0.1", "2.1",   -1);

    /* 2.0's alpha release is older than everything else */
    test_version_cmp("2.0~alpha", "2.0",     -1);
    test_version_cmp("2.0~alpha", "2.0.0",   -1);
    test_version_cmp("2.0~alpha", "2.0+dev", -1);
    test_version_cmp("2.0~alpha", "2.0.1",   -1);
    test_version_cmp("2.0~alpha", "2.1",     -1);

    /* 2.0's dev postrelease is newer than the other 2.0 versions, but older
     * than everything else. */
    test_version_cmp("2.0+dev", "2.0~alpha",  1);
    test_version_cmp("2.0+dev", "2.0",        1);
    test_version_cmp("2.0+dev", "2.0.0",      1);
    test_version_cmp("2.0+dev", "2.0.1",     -1);
    test_version_cmp("2.0+dev", "2.1",       -1);
}
END_TEST


/*-----------------------------------------------------------------------
 * Dependencies
 */

static void
test_dependency_string(const char *in, const char *expected)
{
    struct bz_dependency  *dependency;
    fail_if_error(dependency = bz_dependency_from_string(in));
    fail_unless_streq("dependencies", expected,
                      bz_dependency_to_string(dependency));
    bz_dependency_free(dependency);
}

START_TEST(test_dependencies)
{
    /* A slew of versions */
    test_dependency_string("foo >= 2.0",       "foo >= 2.0");
    test_dependency_string("foo >= 2.0.0",     "foo >= 2.0.0");
    test_dependency_string("foo >= 2.0~alpha", "foo >= 2.0~alpha");
    test_dependency_string("foo >= 2.0+dev",   "foo >= 2.0+dev");
    test_dependency_string("foo >= 2.0.1",     "foo >= 2.0.1");
    test_dependency_string("foo >= 2.1",       "foo >= 2.1");

    /* A slew of whitespace */
    test_dependency_string("foo>=2.0",     "foo >= 2.0");
    test_dependency_string("foo >=2.0",    "foo >= 2.0");
    test_dependency_string("foo  >=2.0",   "foo >= 2.0");
    test_dependency_string("foo>= 2.0",    "foo >= 2.0");
    test_dependency_string("foo>=  2.0",   "foo >= 2.0");
    test_dependency_string("foo  >= 2.0",  "foo >= 2.0");
    test_dependency_string("foo  >=  2.0", "foo >= 2.0");

    /* A slew of package names */
    test_dependency_string("foo >= 2.0",         "foo >= 2.0");
    test_dependency_string("foo-bar >= 2.0",     "foo-bar >= 2.0");
    test_dependency_string("foo-bar-baz >= 2.0", "foo-bar-baz >= 2.0");

    /* Oh wait what about no version at all */
    test_dependency_string("foo", "foo");
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("versions");

    TCase  *tc_versions = tcase_create("versions");
    tcase_add_test(tc_versions, test_versions);
    tcase_add_test(tc_versions, test_dependencies);
    suite_add_tcase(s, tc_versions);

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
