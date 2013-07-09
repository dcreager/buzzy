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

#include "buzzy/built.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/value.h"
#include "buzzy/version.h"
#include "buzzy/distro/homebrew.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Helper functions
 */

static void
mock_package(const char *package, const char *available, const char *installed,
             bool head_available)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "brew info %s", package);
    if (available == NULL) {
        cork_buffer_printf
            (&buf2, "Error: No available formula for %s\n", package);
        bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 1);
    } else {
        cork_buffer_printf(&buf2, "%s: stable %s", package, available);
        if (head_available) {
            cork_buffer_append_string(&buf2, ", HEAD");
        }
        cork_buffer_append_string(&buf2, "\nhttp://example.com/fake/url\n");
        if (installed == NULL) {
            cork_buffer_append_string(&buf2, "Not installed\n");
        } else {
            cork_buffer_append_printf
                (&buf2, "/usr/local/Cellar/%s/%s (8 files, 4M) *\n",
                 package, installed);
        }
        bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    }
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_package_installation(const char *package, const char *version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "brew install %s", package);
    bz_mock_subprocess(buf1.buf, NULL, NULL, 0);
    cork_buffer_done(&buf1);
}


/*-----------------------------------------------------------------------
 * Platform detection
 */

START_TEST(test_homebrew_detect)
{
    DESCRIBE_TEST;
    /* Make sure that we can detect whether the current machine is running
     * homebrew on Mac OS X.  For this test, we don't care about the result, we
     * just want to make sure the check can be performed on all platforms. */
    CORK_ATTR_UNUSED bool  homebrew_present;
    fail_if_error(bz_homebrew_is_present(&homebrew_present));
    fprintf(stderr, "Homebrew on Mac OS X %s present\n",
            homebrew_present? "is": "is not");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native packages
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the homebrew Linux packaging tools
 * to actually be installed. */

START_TEST(test_homebrew_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, but has not
     * yet been installed. */
    reset_everything();
    bz_start_mocks();
    mock_package("jansson", "2.4", NULL, true);

    fail_if_error(version = bz_homebrew_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_homebrew_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST

START_TEST(test_homebrew_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, and has been
     * installed. */
    reset_everything();
    bz_start_mocks();
    mock_package("jansson", "2.4", "2.4", false);

    fail_if_error(version = bz_homebrew_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_homebrew_native_version_installed("jansson"));
    test_and_free_version(version, "2.4");
}
END_TEST

START_TEST(test_homebrew_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that isn't available in the native package database. */
    reset_everything();
    bz_start_mocks();
    mock_package("jansson", NULL, NULL, false);

    fail_if_error(version = bz_homebrew_native_version_available("jansson"));
    fail_unless(version == NULL, "Unexpected version");

    fail_if_error(version = bz_homebrew_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the homebrew Linux packaging tools
 * to actually be installed. */

static void
test_homebrew_pdb_dep(struct bz_pdb *pdb, const char *dep_str,
                  const char *expected_actions)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    bz_mocked_actions_clear();
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep, NULL));
    fail_if_error(bz_package_install(package));
    test_actions(expected_actions);
    bz_dependency_free(dep);
}

static void
test_homebrew_pdb_unknown_dep(struct bz_pdb *pdb, const char *dep_str)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep, NULL));
    fail_unless(package == NULL, "Should not be able to build %s", dep_str);
    bz_dependency_free(dep);
}

START_TEST(test_homebrew_pdb_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed. */
    bz_start_mocks();
    mock_package("jansson", "2.4", NULL, false);
    mock_package_installation("jansson", "2.4");

    fail_if_error(pdb = bz_homebrew_native_pdb());

    test_homebrew_pdb_dep(pdb, "jansson",
        "[1] Install native Homebrew package jansson 2.4\n"
    );

    test_homebrew_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Homebrew package jansson 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_homebrew_pdb_uninstalled_native_package_02)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* Test that if we try to install the same dependency twice, the second
     * attempt is a no-op. */
    bz_start_mocks();
    mock_package("jansson", "2.4", NULL, true);
    mock_package_installation("jansson", "2.4");

    fail_if_error(pdb = bz_homebrew_native_pdb());

    test_homebrew_pdb_dep(pdb, "jansson",
        "[1] Install native Homebrew package jansson 2.4\n"
    );

    test_homebrew_pdb_dep(pdb, "jansson", "Nothing to do!\n");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_homebrew_pdb_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, and has been
     * installed. */
    bz_start_mocks();
    mock_package("jansson", "2.4", "2.4", false);

    fail_if_error(pdb = bz_homebrew_native_pdb());

    test_homebrew_pdb_dep(pdb, "jansson", "Nothing to do!\n");
    test_homebrew_pdb_dep(pdb, "jansson >= 2.4", "Nothing to do!\n");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_homebrew_pdb_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that isn't available in the native package database. */
    bz_start_mocks();
    mock_package("jansson", NULL, NULL, false);
    mock_package("libjansson", NULL, NULL, false);

    fail_if_error(pdb = bz_homebrew_native_pdb());

    test_homebrew_pdb_unknown_dep(pdb, "jansson");
    test_homebrew_pdb_unknown_dep(pdb, "jansson >= 2.4");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_homebrew_pdb_uninstalled_override_package_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed, where we give the package name as an override. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();
    mock_package("libjansson0", "2.4", NULL, false);
    mock_package_installation("libjansson0", "2.4");

    fail_if_error(pdb = bz_homebrew_native_pdb());
    bz_env_add_override
        (env, "native.jansson", bz_string_value_new("libjansson0"));

    test_homebrew_pdb_dep(pdb, "jansson",
        "[1] Install native Homebrew package libjansson0 2.4\n"
    );

    test_homebrew_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Homebrew package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_homebrew_pdb_uninstalled_override_package_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed, where we give the package name as an override. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();
    mock_package("libjansson0", "2.4", NULL, true);
    mock_package_installation("libjansson0", "2.4");

    fail_if_error(pdb = bz_homebrew_native_pdb());
    bz_env_add_override
        (env, "native.homebrew.jansson", bz_string_value_new("libjansson0"));

    test_homebrew_pdb_dep(pdb, "jansson",
        "[1] Install native Homebrew package libjansson0 2.4\n"
    );

    test_homebrew_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Homebrew package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("homebrew");

    TCase  *tc_homebrew = tcase_create("homebrew");
    tcase_add_test(tc_homebrew, test_homebrew_detect);
    tcase_add_test(tc_homebrew, test_homebrew_uninstalled_native_package_01);
    tcase_add_test(tc_homebrew, test_homebrew_installed_native_package_01);
    tcase_add_test(tc_homebrew, test_homebrew_nonexistent_native_package_01);
    suite_add_tcase(s, tc_homebrew);

    TCase  *tc_homebrew_pdb = tcase_create("homebrew-pdb");
    tcase_add_test(tc_homebrew_pdb,
                   test_homebrew_pdb_uninstalled_native_package_01);
    tcase_add_test(tc_homebrew_pdb,
                   test_homebrew_pdb_uninstalled_native_package_02);
    tcase_add_test(tc_homebrew_pdb,
                   test_homebrew_pdb_installed_native_package_01);
    tcase_add_test(tc_homebrew_pdb,
                   test_homebrew_pdb_nonexistent_native_package_01);
    tcase_add_test(tc_homebrew_pdb,
                   test_homebrew_pdb_uninstalled_override_package_01);
    tcase_add_test(tc_homebrew_pdb,
                   test_homebrew_pdb_uninstalled_override_package_02);
    suite_add_tcase(s, tc_homebrew_pdb);

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
