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

#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/debian.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Helper functions
 */

static void
mock_available_package(const char *package, const char *available_version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "apt-cache show --no-all-versions %s", package);
    cork_buffer_printf(&buf2, "Version: %s\n", available_version);
    bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_unavailable_package(const char *package)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "apt-cache show --no-all-versions %s", package);
    cork_buffer_printf(&buf2, "N: Unable to locate package %s\n", package);
    bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 100);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_installed_package(const char *package, const char *installed_version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf
        (&buf1, "dpkg-query -W -f ${Status}\\n${Version} %s", package);
    cork_buffer_printf(&buf2, "install ok installed\n%s", installed_version);
    bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_uninstalled_package(const char *package)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf
        (&buf1, "dpkg-query -W -f ${Status}\\n${Version} %s", package);
    cork_buffer_printf
        (&buf2, "dpkg-query: no packages found match %s\n", package);
    bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 1);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_package_installation(const char *package, const char *version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "sudo apt-get install -y %s", package);
    bz_mock_subprocess(buf1.buf, NULL, NULL, 0);
    cork_buffer_done(&buf1);
}


/*-----------------------------------------------------------------------
 * Platform detection
 */

START_TEST(test_debian_detect)
{
    DESCRIBE_TEST;
    /* Make sure that we can detect whether the current machine is running
     * Debian Linux.  For this test, we don't care about the result, we just
     * want to make sure the check can be performed on all platforms. */
    CORK_ATTR_UNUSED bool  present;
    fail_if_error(bz_debian_is_present(&present));
    fprintf(stderr, "Debian Linux %s present\n", present? "is": "is not");
}
END_TEST


/*-----------------------------------------------------------------------
 * DEBIAN version strings
 */

static void
test_version_to_deb(const char *buzzy, const char *deb)
{
    struct bz_version  *version;
    struct cork_buffer  deb_version = CORK_BUFFER_INIT();
    fail_if_error(version = bz_version_from_string(buzzy));
    bz_version_to_deb(version, &deb_version);
    fail_unless_streq("Versions", deb, deb_version.buf);
    cork_buffer_done(&deb_version);
    bz_version_free(version);
}

static void
test_version_from_deb(const char *buzzy, const char *deb)
{
    struct bz_version  *version;
    fail_if_error(version = bz_version_from_deb(deb));
    fail_unless_streq("Versions", buzzy, bz_version_to_string(version));
    bz_version_free(version);
}

static void
test_deb_version(const char *buzzy, const char *deb)
{
    test_version_to_deb(buzzy, deb);
    test_version_from_deb(buzzy, deb);
}

START_TEST(test_deb_versions)
{
    DESCRIBE_TEST;
    test_deb_version("2.0", "2.0");
    test_deb_version("2.0~alpha", "2.0~alpha");
    test_deb_version("2.0~alpha.1", "2.0~alpha.1");
    test_deb_version("2.0~1", "2.0~1");
    test_deb_version("2.0+hotfix.1", "2.0+hotfix.1");
    test_deb_version("2.0+1", "2.0+1");
    test_deb_version("2.0+git+20130529", "2.0+git+20130529");
    test_deb_version("2.0+rev.2", "2.0-2");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native packages
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the DEBIAN Linux packaging tools
 * to actually be installed. */

START_TEST(test_apt_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, but has not
     * yet been installed. */
    reset_everything();
    bz_start_mocks();
    mock_available_package("jansson", "2.4");
    mock_uninstalled_package("jansson");

    fail_if_error(version = bz_apt_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_deb_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST

START_TEST(test_apt_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, and has been
     * installed. */
    reset_everything();
    bz_start_mocks();
    mock_available_package("jansson", "2.4");
    mock_installed_package("jansson", "2.4");

    fail_if_error(version = bz_apt_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_deb_native_version_installed("jansson"));
    test_and_free_version(version, "2.4");
}
END_TEST

START_TEST(test_apt_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that isn't available in the native package database. */
    reset_everything();
    bz_start_mocks();
    mock_unavailable_package("jansson");
    mock_uninstalled_package("jansson");

    fail_if_error(version = bz_apt_native_version_available("jansson"));
    fail_unless(version == NULL, "Unexpected version");

    fail_if_error(version = bz_deb_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the DEBIAN packaging tools to
 * actually be installed. */

static void
test_apt_pdb_dep(struct bz_pdb *pdb, const char *dep_str,
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
test_apt_pdb_unknown_dep(struct bz_pdb *pdb, const char *dep_str)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep, NULL));
    fail_unless(package == NULL, "Should not be able to build %s", dep_str);
    bz_dependency_free(dep);
}

START_TEST(test_apt_pdb_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed. */
    bz_start_mocks();
    mock_unavailable_package("jansson-dev");
    mock_unavailable_package("libjansson-dev");
    mock_unavailable_package("libjansson");
    mock_available_package("jansson", "2.4");
    mock_uninstalled_package("jansson");
    mock_package_installation("jansson", "2.4");

    fail_if_error(pdb = bz_apt_native_pdb());

    test_apt_pdb_dep(pdb, "jansson",
        "[1] Install native Debian package jansson 2.4\n"
    );

    test_apt_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Debian package jansson 2.4\n"
    );

    test_apt_pdb_unknown_dep(pdb, "jansson >= 2.5");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_apt_pdb_uninstalled_native_package_02)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* Test that if we try to install the same dependency twice, the second
     * attempt is a no-op. */
    bz_start_mocks();
    mock_unavailable_package("jansson-dev");
    mock_unavailable_package("libjansson-dev");
    mock_available_package("jansson", "2.4");
    mock_uninstalled_package("jansson");
    mock_package_installation("jansson", "2.4");

    fail_if_error(pdb = bz_apt_native_pdb());

    test_apt_pdb_dep(pdb, "jansson",
        "[1] Install native Debian package jansson 2.4\n"
    );

    test_apt_pdb_dep(pdb, "jansson", "Nothing to do!\n");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_apt_pdb_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, and has been
     * installed. */
    bz_start_mocks();
    mock_unavailable_package("jansson-dev");
    mock_unavailable_package("libjansson-dev");
    mock_available_package("jansson", "2.4");
    mock_installed_package("jansson", "2.4");

    fail_if_error(pdb = bz_apt_native_pdb());

    test_apt_pdb_dep(pdb, "jansson", "Nothing to do!\n");
    test_apt_pdb_dep(pdb, "jansson >= 2.4", "Nothing to do!\n");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_apt_pdb_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that isn't available in the native package database. */
    bz_start_mocks();
    mock_unavailable_package("jansson-dev");
    mock_unavailable_package("libjansson-dev");
    mock_unavailable_package("jansson");
    mock_unavailable_package("libjansson");

    fail_if_error(pdb = bz_apt_native_pdb());

    test_apt_pdb_unknown_dep(pdb, "jansson");
    test_apt_pdb_unknown_dep(pdb, "jansson >= 2.4");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_apt_pdb_uninstalled_override_package_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed, where we give the package name as an override. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();
    mock_available_package("libjansson0", "2.4");
    mock_uninstalled_package("libjansson0");
    mock_package_installation("libjansson0", "2.4");

    fail_if_error(pdb = bz_apt_native_pdb());
    bz_env_add_override
        (env, "native.jansson", bz_string_value_new("libjansson0"));

    test_apt_pdb_dep(pdb, "jansson",
        "[1] Install native Debian package libjansson0 2.4\n"
    );

    test_apt_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Debian package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_apt_pdb_uninstalled_override_package_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed, where we give the package name as an override. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();
    mock_available_package("libjansson0", "2.4");
    mock_uninstalled_package("libjansson0");
    mock_package_installation("libjansson0", "2.4");

    fail_if_error(pdb = bz_apt_native_pdb());
    bz_env_add_override
        (env, "native.debian.jansson", bz_string_value_new("libjansson0"));

    test_apt_pdb_dep(pdb, "jansson",
        "[1] Install native Debian package libjansson0 2.4\n"
    );

    test_apt_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Debian package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_apt_pdb_preinstalled_package_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is preinstalled on this system. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();

    fail_if_error(pdb = bz_apt_native_pdb());
    bz_env_add_override
        (env, "preinstalled.debian.jansson", bz_string_value_new("2.4"));

    test_apt_pdb_dep(pdb, "jansson",
        "[1] Preinstalled native Debian package jansson 2.4\n"
    );

    test_apt_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Preinstalled native Debian package jansson 2.4\n"
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
    Suite  *s = suite_create("deb");

    TCase  *tc_deb = tcase_create("deb");
    tcase_add_test(tc_deb, test_debian_detect);
    tcase_add_test(tc_deb, test_deb_versions);
    tcase_add_test(tc_deb, test_apt_uninstalled_native_package_01);
    tcase_add_test(tc_deb, test_apt_installed_native_package_01);
    tcase_add_test(tc_deb, test_apt_nonexistent_native_package_01);
    suite_add_tcase(s, tc_deb);

    TCase  *tc_apt_pdb = tcase_create("apt-pdb");
    tcase_add_test(tc_apt_pdb, test_apt_pdb_uninstalled_native_package_01);
    tcase_add_test(tc_apt_pdb, test_apt_pdb_uninstalled_native_package_02);
    tcase_add_test(tc_apt_pdb, test_apt_pdb_installed_native_package_01);
    tcase_add_test(tc_apt_pdb, test_apt_pdb_nonexistent_native_package_01);
    tcase_add_test(tc_apt_pdb, test_apt_pdb_uninstalled_override_package_01);
    tcase_add_test(tc_apt_pdb, test_apt_pdb_uninstalled_override_package_02);
    tcase_add_test(tc_apt_pdb, test_apt_pdb_preinstalled_package_01);
    suite_add_tcase(s, tc_apt_pdb);

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
