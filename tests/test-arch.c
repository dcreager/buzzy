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
    cork_buffer_printf(&buf1, "pacman -Sddp --print-format %%v %s", package);
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
    cork_buffer_printf(&buf1, "pacman -Sddp --print-format %%v %s", package);
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
    test_arch_version("2.0~alpha.1", "2.0alpha1");
    test_arch_version("2.0~1", "2.0pre1");
    test_arch_version("2.0+hotfix.1", "2.0.hotfix1");
    test_arch_version("2.0+1", "2.0.post1");
    test_arch_version("2.0+git.20130529", "2.0.git20130529");
    test_arch_version("2.0+git+20130529", "2.0.git.post20130529");
    test_arch_version("2.0+git.123+abc.3", "2.0.git123.abc3");
    test_arch_version("2.0+git+123+abc+3", "2.0.git.post123.abc.post3");

    test_version_from_arch("2.0", "2.0-1");
    test_version_from_arch("2.0+rev.2", "2.0-2");
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
    reset_everything();
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
    reset_everything();
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
    reset_everything();
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
    bz_mocked_actions_clear();
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep, NULL));
    fail_if_error(bz_package_install(package));
    test_actions(expected_actions);
    bz_dependency_free(dep);
}

static void
test_arch_pdb_unknown_dep(struct bz_pdb *pdb, const char *dep_str)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep, NULL));
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
        "[1] Install native Arch package jansson 2.4\n"
    );

    test_arch_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Arch package jansson 2.4\n"
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
        "[1] Install native Arch package jansson 2.4\n"
    );

    test_arch_pdb_dep(pdb, "jansson", "Nothing to do!\n");

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

    test_arch_pdb_dep(pdb, "jansson", "Nothing to do!\n");
    test_arch_pdb_dep(pdb, "jansson >= 2.4", "Nothing to do!\n");

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

START_TEST(test_arch_pdb_uninstalled_override_package_01)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed, where we give the package name as an override. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();
    mock_available_package("libjansson0", "2.4-1");
    mock_uninstalled_package("libjansson0");
    mock_package_installation("libjansson0", "2.4-1");

    fail_if_error(pdb = bz_arch_native_pdb());
    bz_env_add_override
        (env, "native.jansson", bz_string_value_new("libjansson0"));

    test_arch_pdb_dep(pdb, "jansson",
        "[1] Install native Arch package libjansson0 2.4\n"
    );

    test_arch_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Arch package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_arch_pdb_uninstalled_override_package_02)
{
    DESCRIBE_TEST;
    struct bz_env  *env;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed, where we give the package name as an override. */
    reset_everything();
    bz_start_mocks();
    env = bz_global_env();
    mock_available_package("libjansson0", "2.4-1");
    mock_uninstalled_package("libjansson0");
    mock_package_installation("libjansson0", "2.4-1");

    fail_if_error(pdb = bz_arch_native_pdb());
    bz_env_add_override
        (env, "native.arch.jansson", bz_string_value_new("libjansson0"));

    test_arch_pdb_dep(pdb, "jansson",
        "[1] Install native Arch package libjansson0 2.4\n"
    );

    test_arch_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native Arch package libjansson0 2.4\n"
    );

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
test_create_package(struct bz_env *env, bool force,
                    const char *expected_actions)
{
    struct cork_path  *binary_package_dir = cork_path_new(".");
    struct cork_path  *staging_dir = cork_path_new("/tmp/staging");
    struct bz_pdb  *pdb;
    struct bz_packager  *packager;

    fail_if_error(pdb = bz_arch_native_pdb());
    bz_pdb_register(pdb);

    mock_available_package("pacman", "4.0.3-7");
    mock_installed_package("pacman", "4.0.3-7");
    bz_mock_file_exists(cork_path_get(staging_dir), true);
    bz_env_add_override(env, "binary_package_dir",
                        bz_path_value_new(binary_package_dir));
    bz_env_add_override(env, "staging_dir", bz_path_value_new(staging_dir));
    bz_env_add_override(env, "force", bz_string_value_new(force? "1": "0"));
    bz_env_add_override(env, "verbose", bz_string_value_new("0"));
    fail_if_error(packager = bz_pacman_packager_new(env));
    fail_if_error(bz_packager_package(packager));
    test_actions(expected_actions);
    bz_packager_free(packager);
}

START_TEST(test_arch_create_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    bz_mock_file_exists("./jansson-2.4-1-x86_64.pkg.tar.xz", false);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_create_package(env, false,
        "[1] Package jansson 2.4 (pacman)\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1-x86_64.pkg.tar.xz ]\n"
        "$ pacman -Sddp --print-format %v pacman\n"
        "$ pacman -Q pacman\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/PKGBUILD"
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
    bz_env_free(env);
}
END_TEST

START_TEST(test_arch_create_package_license_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    bz_mock_file_exists("./jansson-2.4-1-x86_64.pkg.tar.xz", false);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    fail_if_error(bz_env_add_override
                  (env, "license", bz_string_value_new("MIT")));
    test_create_package(env, false,
        "[1] Package jansson 2.4 (pacman)\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1-x86_64.pkg.tar.xz ]\n"
        "$ pacman -Sddp --print-format %v pacman\n"
        "$ pacman -Q pacman\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/PKGBUILD"
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
    bz_env_free(env);
}
END_TEST

START_TEST(test_arch_create_package_deps_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_array  *deps;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    bz_mock_file_exists("./jansson-2.4-1-x86_64.pkg.tar.xz", false);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    deps = bz_array_new();
    bz_array_append(deps, bz_string_value_new("libfoo"));
    bz_array_append(deps, bz_string_value_new("libbar >= 2.5~alpha.1"));
    fail_if_error(bz_env_add_override
                  (env, "dependencies", bz_array_as_value(deps)));
    test_create_package(env, false,
        "[1] Package jansson 2.4 (pacman)\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1-x86_64.pkg.tar.xz ]\n"
        "$ pacman -Sddp --print-format %v pacman\n"
        "$ pacman -Q pacman\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/PKGBUILD"
            " <<EOF\n"
        "pkgname='jansson'\n"
        "pkgver='2.4'\n"
        "pkgrel='1'\n"
        "arch=('x86_64')\n"
        "license=('unknown')\n"
        "depends=('libfoo' 'libbar>=2.5alpha1')\n"
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '/tmp/staging' \"${pkgdir}\"\n"
        "}\n"
        "EOF\n"
        "$ makepkg -sf\n"
    );
    bz_env_free(env);
}
END_TEST

START_TEST(test_arch_create_existing_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    bz_mock_file_exists("./jansson-2.4-1-x86_64.pkg.tar.xz", true);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_create_package(env, false, "Nothing to do!\n");
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1-x86_64.pkg.tar.xz ]\n"
    );
    bz_env_free(env);
}
END_TEST

START_TEST(test_arch_create_existing_package_02)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_subprocess("makepkg -sf", NULL, NULL, 0);
    bz_mock_file_exists("./jansson-2.4-1-x86_64.pkg.tar.xz", true);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_create_package(env, true,
        "[1] Package jansson 2.4 (pacman)\n"
    );
    verify_commands_run(
        "$ pacman -Sddp --print-format %v pacman\n"
        "$ pacman -Q pacman\n"
        "$ uname -m\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/PKGBUILD"
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
    bz_env_free(env);
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
    tcase_add_test(tc_arch_pdb, test_arch_pdb_uninstalled_override_package_01);
    tcase_add_test(tc_arch_pdb, test_arch_pdb_uninstalled_override_package_02);
    suite_add_tcase(s, tc_arch_pdb);

    TCase  *tc_arch_package = tcase_create("arch-package");
    tcase_add_test(tc_arch_package, test_arch_create_package_01);
    tcase_add_test(tc_arch_package, test_arch_create_package_license_01);
    tcase_add_test(tc_arch_package, test_arch_create_package_deps_01);
    tcase_add_test(tc_arch_package, test_arch_create_existing_package_01);
    tcase_add_test(tc_arch_package, test_arch_create_existing_package_02);
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
