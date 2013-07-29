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

#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/arch.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Helper functions
 */

/* The CMake builder will try to ensure that the "cmake" package is installed,
 * so we need to provide some package database that can satisfy that dependency.
 * Since we're mocking everything, that package database doesn't have to reflect
 * the real package framework on the current host, so we're just going to
 * pretend that the current platform is Arch Linux. */

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

static void
mock_cmake_installed(void)
{
    mock_available_package("cmake", "2.6-1");
    mock_installed_package("cmake", "2.6-1");
}

static void
mock_cmake_uninstalled(void)
{
    mock_available_package("cmake", "2.6-1");
    mock_uninstalled_package("cmake");
    mock_package_installation("cmake", "2.6-1");
}

static void
mock_cmake_unavailable(void)
{
    mock_unavailable_package("cmake");
    mock_unavailable_package("libcmake");
}


/*-----------------------------------------------------------------------
 * CMake builder
 */

static void
test_stage_package(struct bz_env *env, bool force,
                   const char *expected_actions)
{
    struct cork_path  *source_dir = cork_path_new("/home/test/source");
    struct bz_pdb  *pdb;
    struct bz_builder  *builder;

    fail_if_error(pdb = bz_arch_native_pdb());
    bz_pdb_register(pdb);

    bz_mock_file_exists(cork_path_get(source_dir), true);
    bz_env_add_override(env, "source_dir", bz_path_value_new(source_dir));
    bz_env_add_override(env, "force", bz_string_value_new(force? "1": "0"));
    bz_env_add_override(env, "verbose", bz_string_value_new("0"));
    fail_if_error(builder = bz_cmake_builder_new(env));
    fail_if_error(bz_builder_stage(builder));
    test_actions(expected_actions);
    bz_builder_free(builder);
}

START_TEST(test_cmake_stage_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    mock_cmake_installed();
    bz_mock_subprocess
        ("cmake /home/test/source"
         " -DCMAKE_INSTALL_PREFIX=/usr"
         " -DCMAKE_INSTALL_LIBDIR=lib"
         " -DCMAKE_BUILD_TYPE=RelWithDebInfo",
         NULL, NULL, 0);
    bz_mock_subprocess("make", NULL, NULL, 0);
    bz_mock_subprocess("make install", NULL, NULL, 0);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_stage_package(env, false,
        "[1] Build jansson 2.4 (cmake)\n"
        "[2] Stage jansson 2.4 (cmake)\n"
    );
    verify_commands_run(
        "$ pacman -Sddp --print-format %v cmake\n"
        "$ pacman -Q cmake\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson-buzzy/build\n"
        "$ cmake /home/test/source"
            " -DCMAKE_INSTALL_PREFIX=/usr"
            " -DCMAKE_INSTALL_LIBDIR=lib"
            " -DCMAKE_BUILD_TYPE=RelWithDebInfo\n"
        "$ make\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson-buzzy/stage\n"
        "$ make install\n"
    );
    bz_env_free(env);
}
END_TEST


START_TEST(test_cmake_uninstalled_stage_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    mock_cmake_uninstalled();
    bz_mock_subprocess
        ("cmake /home/test/source"
         " -DCMAKE_INSTALL_PREFIX=/usr"
         " -DCMAKE_INSTALL_LIBDIR=lib"
         " -DCMAKE_BUILD_TYPE=RelWithDebInfo",
         NULL, NULL, 0);
    bz_mock_subprocess("make", NULL, NULL, 0);
    bz_mock_subprocess("make install", NULL, NULL, 0);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_stage_package(env, false,
        "[1] Install native Arch package cmake 2.6\n"
        "[2] Build jansson 2.4 (cmake)\n"
        "[3] Stage jansson 2.4 (cmake)\n"
    );
    verify_commands_run(
        "$ pacman -Sddp --print-format %v cmake\n"
        "$ pacman -Q cmake\n"
        "$ sudo pacman -S --noconfirm cmake\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson-buzzy/build\n"
        "$ cmake /home/test/source"
            " -DCMAKE_INSTALL_PREFIX=/usr"
            " -DCMAKE_INSTALL_LIBDIR=lib"
            " -DCMAKE_BUILD_TYPE=RelWithDebInfo\n"
        "$ make\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson-buzzy/stage\n"
        "$ make install\n"
    );
    bz_env_free(env);
}
END_TEST


static void
test_unavailable(struct bz_env *env)
{
    struct cork_path  *source_dir = cork_path_new("/home/test/source");
    struct bz_pdb  *pdb;
    struct bz_builder  *builder;

    fail_if_error(pdb = bz_arch_native_pdb());
    bz_pdb_register(pdb);

    bz_mock_file_exists(cork_path_get(source_dir), true);
    bz_env_add_override(env, "source_dir", bz_path_value_new(source_dir));
    bz_env_add_override(env, "verbose", bz_string_value_new("0"));
    fail_if_error(builder = bz_cmake_builder_new(env));
    fail_unless_error(bz_builder_stage(builder));
    bz_builder_free(builder);
}

START_TEST(test_cmake_unavailable_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    mock_cmake_unavailable();
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_unavailable(env);
    verify_commands_run(
        "$ pacman -Sddp --print-format %v cmake\n"
        "$ pacman -Sddp --print-format %v libcmake\n"
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
    Suite  *s = suite_create("cmake");

    TCase  *tc_cmake_package = tcase_create("cmake");
    tcase_add_test(tc_cmake_package, test_cmake_stage_package_01);
    tcase_add_test(tc_cmake_package, test_cmake_uninstalled_stage_package_01);
    tcase_add_test(tc_cmake_package, test_cmake_unavailable_01);
    suite_add_tcase(s, tc_cmake_package);

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
