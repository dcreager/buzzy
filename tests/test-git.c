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
#include "buzzy/distro/git.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * git version strings
 */

static void
test_git_version(const char *git, const char *buzzy)
{
    struct bz_version  *version;
    fail_if_error(version = bz_version_from_git_describe(git));
    fail_unless_streq("Versions", buzzy, bz_version_to_string(version));
    bz_version_free(version);
}

START_TEST(test_git_versions)
{
    DESCRIBE_TEST;

    test_git_version("1.0", "1.0");
    test_git_version("1.0-4-g1a2b3c4", "1.0+4+git1a2b3c4");
    test_git_version("1.0-dev", "1.0+dev");
    test_git_version("1.0-dev4-g1a2b3c4", "1.0+dev4+git1a2b3c4");
    test_git_version("1.0-dev-4-g1a2b3c4", "1.0+dev4+git1a2b3c4");
    test_git_version("1.0-devel", "1.0+devel");
    test_git_version("1.0-devel-4-g1a2b3c4", "1.0+devel4+git1a2b3c4");
    test_git_version("1.0-pre1", "1.0~1");
    test_git_version("1.0--pre1", "1.0~1");
    test_git_version("1.0--beta1", "1.0~beta1");

    test_git_version("v1.0", "1.0");
    test_git_version("v1.0-4-g1a2b3c4", "1.0+4+git1a2b3c4");
    test_git_version("v1.0-dev", "1.0+dev");
    test_git_version("v1.0-dev-4-g1a2b3c4", "1.0+dev4+git1a2b3c4");
    test_git_version("v1.0-devel", "1.0+devel");
    test_git_version("v1.0-devel-4-g1a2b3c4", "1.0+devel4+git1a2b3c4");
    test_git_version("v1.0-pre1", "1.0~1");
    test_git_version("v1.0--pre1", "1.0~1");
    test_git_version("v1.0--beta1", "1.0~beta1");

    test_git_version("test-pkg-1.0", "1.0");
    test_git_version("test-pkg-1.0-4-g1a2b3c4", "1.0+4+git1a2b3c4");
    test_git_version("test-pkg-1.0-dev", "1.0+dev");
    test_git_version("test-pkg-1.0-dev-4-g1a2b3c4", "1.0+dev4+git1a2b3c4");
    test_git_version("test-pkg-1.0-devel", "1.0+devel");
    test_git_version("test-pkg-1.0-devel-4-g1a2b3c4", "1.0+devel4+git1a2b3c4");
    test_git_version("test-pkg-1.0-pre1", "1.0~1");
    test_git_version("test-pkg-1.0--pre1", "1.0~1");
    test_git_version("test-pkg-1.0--beta1", "1.0~beta1");
}
END_TEST


/*-----------------------------------------------------------------------
 * git version values
 */

static void
test_git_version_value(const char *git, const char *buzzy)
{
    struct bz_value_provider  *provider;
    struct bz_env  *env = bz_env_new("test");
    const char  *actual;
    bz_start_mocks();
    bz_mock_subprocess("git describe", git, NULL, 0);
    bz_mock_subprocess("git status --porcelain", NULL, NULL, 0);
    fail_if_error(provider = bz_git_version_value_new());
    bz_env_add_override(env, "version", provider);
    bz_env_add_override(env, "source_dir", bz_string_value_new("."));
    fail_if_error(actual = bz_env_get_string(env, "version", true));
    fail_unless_streq("Versions", buzzy, actual);
    bz_env_free(env);
}

START_TEST(test_git_version_values)
{
    DESCRIBE_TEST;

    test_git_version_value("1.0\n", "1.0");
    test_git_version_value("1.0-4-g1a2b3c4\n", "1.0+4+git1a2b3c4");
    test_git_version_value("1.0-dev\n", "1.0+dev");
    test_git_version_value("1.0-dev4-g1a2b3c4\n", "1.0+dev4+git1a2b3c4");
    test_git_version_value("1.0-dev-4-g1a2b3c4\n", "1.0+dev4+git1a2b3c4");
    test_git_version_value("1.0-devel\n", "1.0+devel");
    test_git_version_value("1.0-devel-4-g1a2b3c4\n", "1.0+devel4+git1a2b3c4");
    test_git_version_value("1.0-pre1\n", "1.0~1");
    test_git_version_value("1.0--pre1\n", "1.0~1");
    test_git_version_value("1.0--beta1\n", "1.0~beta1");
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("git");

    TCase  *tc_git = tcase_create("git");
    tcase_add_test(tc_git, test_git_versions);
    tcase_add_test(tc_git, test_git_version_values);
    suite_add_tcase(s, tc_git);

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
