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

#include "buzzy/action.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/arch.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Helper functions
 */

static void
mock_available_package(const char *package, const char *available_version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "pacman -Sdp --print-format %%v %s", package);
    cork_buffer_printf(&buf2, "%s\n", available_version);
    bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_unavailable_package(const char *package)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "pacman -Sdp --print-format %%v %s", package);
    cork_buffer_printf(&buf2, "error: target not found: %s\n", package);
    bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 1);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_installed_package(const char *package, const char *installed_version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "pacman -Q %s", package);
    cork_buffer_printf(&buf2, "%s %s\n", package, installed_version);
    bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_uninstalled_package(const char *package)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "pacman -Q %s", package);
    cork_buffer_printf(&buf2, "error: package '%s' was not found\n", package);
    bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 1);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_package_installation(const char *package, const char *version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "sudo pacman -S --noconfirm %s", package);
    bz_mock_subprocess(buf1.buf, NULL, NULL, 0);
    cork_buffer_done(&buf1);
}


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
 * Native packages
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
    bz_start_mocks();
    mock_available_package("jansson", "2.4-1");
    mock_uninstalled_package("jansson");

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
    bz_start_mocks();
    mock_available_package("jansson", "2.4-1");
    mock_installed_package("jansson", "2.4-1");

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
    bz_start_mocks();
    mock_unavailable_package("jansson");
    mock_uninstalled_package("jansson");

    fail_if_error(version = bz_arch_native_version_available("jansson"));
    fail_unless(version == NULL, "Unexpected version");

    fail_if_error(version = bz_arch_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the Arch Linux packaging tools
 * to actually be installed. */

static void
test_arch_pdb_dep(struct bz_pdb *pdb, const char *dep_str,
                  const char *expected_actions)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    struct bz_action  *action;
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep));
    fail_if_error(action = bz_package_install_action(package));
    test_action(action, expected_actions);
    bz_dependency_free(dep);
}

static void
test_arch_pdb_unknown_dep(struct bz_pdb *pdb, const char *dep_str)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep));
    fail_unless(package == NULL, "Should not be able to build %s", dep_str);
    bz_dependency_free(dep);
}

START_TEST(test_arch_pdb_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed. */
    bz_start_mocks();
    mock_available_package("jansson", "2.4-1");
    mock_uninstalled_package("jansson");
    mock_package_installation("jansson", "2.4-1");

    fail_if_error(pdb = bz_arch_native_pdb());

    test_arch_pdb_dep(pdb, "jansson",
        "Test actions\n"
        "[1/1] Install native Arch package jansson 2.4\n"
    );

    test_arch_pdb_dep(pdb, "jansson >= 2.4",
        "Test actions\n"
        "[1/1] Install native Arch package jansson 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_arch_pdb_uninstalled_native_package_02)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* Test that if we try to install the same dependency twice, the second
     * attempt is a no-op. */
    bz_start_mocks();
    mock_available_package("jansson", "2.4-1");
    mock_uninstalled_package("jansson");
    mock_package_installation("jansson", "2.4-1");

    fail_if_error(pdb = bz_arch_native_pdb());

    test_arch_pdb_dep(pdb, "jansson",
        "Test actions\n"
        "[1/1] Install native Arch package jansson 2.4\n"
    );

    test_arch_pdb_dep(pdb, "jansson",
        "Test actions\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_arch_pdb_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, and has been
     * installed. */
    bz_start_mocks();
    mock_available_package("jansson", "2.4-1");
    mock_installed_package("jansson", "2.4-1");

    fail_if_error(pdb = bz_arch_native_pdb());

    test_arch_pdb_dep(pdb, "jansson",
        "Test actions\n"
    );

    test_arch_pdb_dep(pdb, "jansson >= 2.4",
        "Test actions\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_arch_pdb_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that isn't available in the native package database. */
    bz_start_mocks();
    mock_unavailable_package("jansson");
    mock_uninstalled_package("jansson");
    mock_unavailable_package("libjansson");
    mock_uninstalled_package("libjansson");

    fail_if_error(pdb = bz_arch_native_pdb());

    test_arch_pdb_unknown_dep(pdb, "jansson");
    test_arch_pdb_unknown_dep(pdb, "jansson >= 2.4");

    bz_pdb_free(pdb);
}
END_TEST


/*-----------------------------------------------------------------------
 * Building Arch packages
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the Arch Linux packaging tools
 * to actually be installed. */

static void
test_create_package(struct bz_package_spec *spec, const char *expected_actions)
{
    struct cork_path  *package_path = cork_path_new(".");
    struct cork_path  *staging_path = cork_path_new("/tmp/staging");
    struct bz_action  *action;

    bz_mock_file_exists(cork_path_get(staging_path), true);
    action = bz_pacman_create_package
        (spec, package_path, staging_path, NULL, false);
    test_action(action, expected_actions);
    bz_action_free(action);
}

START_TEST(test_arch_create_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_package_spec  *spec;
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(spec = bz_package_spec_new("jansson", version));
    test_create_package(spec,
        "Test actions\n"
        "[1/1] Package jansson 2.4\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/arch/package/jansson/2.4\n"
        "$ cat > /home/test/.cache/buzzy/arch/package/jansson/2.4/PKGBUILD"
            " <<EOF\n"
        "pkgname='jansson'\n"
        "pkgver='2.4'\n"
        "pkgrel='1'\n"
        "arch=('x86_64')\n"
        "license=('unknown')\n"
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '/tmp/staging' \"${pkgdir}\"\n"
        "}\n"
        "EOF\n"
        "$ makepkg -sf\n"
    );
    bz_package_spec_free(spec);
}
END_TEST

START_TEST(test_arch_create_package_license_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_package_spec  *spec;
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(spec = bz_package_spec_new("jansson", version));
    fail_if_error(bz_package_spec_set_license(spec, "MIT"));
    test_create_package(spec,
        "Test actions\n"
        "[1/1] Package jansson 2.4\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/arch/package/jansson/2.4\n"
        "$ cat > /home/test/.cache/buzzy/arch/package/jansson/2.4/PKGBUILD"
            " <<EOF\n"
        "pkgname='jansson'\n"
        "pkgver='2.4'\n"
        "pkgrel='1'\n"
        "arch=('x86_64')\n"
        "license=('MIT')\n"
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '/tmp/staging' \"${pkgdir}\"\n"
        "}\n"
        "EOF\n"
        "$ makepkg -sf\n"
    );
    bz_package_spec_free(spec);
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

    TCase  *tc_arch_pdb = tcase_create("arch-pdb");
    tcase_add_test(tc_arch_pdb, test_arch_pdb_uninstalled_native_package_01);
    tcase_add_test(tc_arch_pdb, test_arch_pdb_uninstalled_native_package_02);
    tcase_add_test(tc_arch_pdb, test_arch_pdb_installed_native_package_01);
    tcase_add_test(tc_arch_pdb, test_arch_pdb_nonexistent_native_package_01);
    suite_add_tcase(s, tc_arch_pdb);

    TCase  *tc_arch_package = tcase_create("arch-package");
    tcase_add_test(tc_arch_package, test_arch_create_package_01);
    tcase_add_test(tc_arch_package, test_arch_create_package_license_01);
    suite_add_tcase(s, tc_arch_package);

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
