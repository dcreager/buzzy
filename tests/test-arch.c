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
#include "buzzy/version.h"
#include "buzzy/distro/arch.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

START_TEST(test_arch_detect)
{
    DESCRIBE_TEST;
    /* Make sure that we can detect whether the current machine is running Arch
     * Linux.  For this test, we don't care about the result, we just want to
     * make sure the check can be performed on all platforms. */
    CORK_ATTR_UNUSED bool  arch_present;
    fail_if_error(bz_arch_is_present(&arch_present));
    fprintf(stderr, "Arch Linux %s present\n", arch_present? "is": "is not");
}
END_TEST


/*-----------------------------------------------------------------------
 * Arch version strings
 */

static void
test_version_to_arch(const char *buzzy, const char *arch)
{
    struct bz_version  *version;
    struct cork_buffer  arch_version = CORK_BUFFER_INIT();
    fail_if_error(version = bz_version_from_string(buzzy));
    bz_version_to_arch(version, &arch_version);
    fail_unless_streq("Versions", arch, arch_version.buf);
    cork_buffer_done(&arch_version);
    bz_version_free(version);
}

static void
test_version_from_arch(const char *buzzy, const char *arch)
{
    struct bz_version  *version;
    fail_if_error(version = bz_version_from_arch(arch));
    fail_unless_streq("Versions", buzzy, bz_version_to_string(version));
    bz_version_free(version);
}

static void
test_arch_version(const char *buzzy, const char *arch)
{
    test_version_to_arch(buzzy, arch);
    test_version_from_arch(buzzy, arch);
}

START_TEST(test_arch_versions)
{
    DESCRIBE_TEST;
    test_arch_version("2.0", "2.0");
    test_arch_version("2.0~alpha", "2.0alpha");
    test_arch_version("2.0~1", "2.0pre1");
    test_arch_version("2.0+hotfix1", "2.0.hotfix1");
    test_arch_version("2.0+1", "2.0.post1");
    test_arch_version("2.0+git123abc456", "2.0.git123abc456");
    test_version_from_arch("2.0", "2.0-1");
    test_arch_version("2.0+rev2", "2.0-2");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the Arch Linux packaging tools
 * to actually be installed. */

START_TEST(test_arch_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, but has not
     * yet been installed. */
    bz_subprocess_start_mocks();
    bz_subprocess_mock("pacman -Sdp --print-format %v jansson",
                       "2.4-1\n", NULL, 0);
    bz_subprocess_mock("pacman -Q jansson", NULL,
                       "error: package 'jansson' was not found\n", 1);

    fail_if_error(version = bz_arch_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_arch_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST

START_TEST(test_arch_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, and has been
     * installed. */
    bz_subprocess_start_mocks();
    bz_subprocess_mock("pacman -Sdp --print-format %v jansson",
                       "2.4-1\n", NULL, 0);
    bz_subprocess_mock("pacman -Q jansson", "jansson 2.4-1\n", NULL, 0);

    fail_if_error(version = bz_arch_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_arch_native_version_installed("jansson"));
    test_and_free_version(version, "2.4");
}
END_TEST

START_TEST(test_arch_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that isn't available in the native package database. */
    bz_subprocess_start_mocks();
    bz_subprocess_mock("pacman -Sdp --print-format %v jansson",
                       NULL, "error: target not found: jansson\n", 1);
    bz_subprocess_mock("pacman -Q jansson", NULL,
                       "error: package 'jansson' was not found\n", 1);

    fail_if_error(version = bz_arch_native_version_available("jansson"));
    fail_unless(version == NULL, "Unexpected version");

    fail_if_error(version = bz_arch_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("arch");

    TCase  *tc_arch = tcase_create("arch");
    tcase_add_test(tc_arch, test_arch_detect);
    tcase_add_test(tc_arch, test_arch_versions);
    tcase_add_test(tc_arch, test_arch_uninstalled_native_package_01);
    tcase_add_test(tc_arch, test_arch_installed_native_package_01);
    tcase_add_test(tc_arch, test_arch_nonexistent_native_package_01);
    suite_add_tcase(s, tc_arch);

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
