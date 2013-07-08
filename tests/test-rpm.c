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
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/rpm.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Helper functions
 */

static void
mock_available_package(const char *package, const char *available_version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "yum info --cacheonly %s", package);
    cork_buffer_printf(&buf2, "Version: %s\nRelease: 1\n", available_version);
    bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_unavailable_package(const char *package)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "yum info --cacheonly %s", package);
    cork_buffer_set_string(&buf2, "Error: No matching Packages to list\n");
    bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 1);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_installed_package(const char *package, const char *installed_version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "rpm --qf %%{V}-%%{R} -q %s", package);
    cork_buffer_printf(&buf2, "%s-1", installed_version);
    bz_mock_subprocess(buf1.buf, buf2.buf, NULL, 0);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_uninstalled_package(const char *package)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    struct cork_buffer  buf2 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "rpm --qf %%{V}-%%{R} -q %s", package);
    cork_buffer_printf(&buf2, "package %s is not installed\n", package);
    bz_mock_subprocess(buf1.buf, NULL, buf2.buf, 1);
    cork_buffer_done(&buf1);
    cork_buffer_done(&buf2);
}

static void
mock_package_installation(const char *package, const char *version)
{
    struct cork_buffer  buf1 = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf1, "sudo yum install -y %s", package);
    bz_mock_subprocess(buf1.buf, NULL, NULL, 0);
    cork_buffer_done(&buf1);
}


/*-----------------------------------------------------------------------
 * Platform detection
 */

START_TEST(test_redhat_detect)
{
    DESCRIBE_TEST;
    /* Make sure that we can detect whether the current machine is running RPM
     * Linux.  For this test, we don't care about the result, we just want to
     * make sure the check can be performed on all platforms. */
    CORK_ATTR_UNUSED bool  present;
    fail_if_error(bz_redhat_is_present(&present));
    fprintf(stderr, "RedHat Linux %s present\n", present? "is": "is not");
}
END_TEST


/*-----------------------------------------------------------------------
 * RPM version strings
 */

static void
test_version_to_rpm(const char *buzzy, const char *rpm)
{
    struct bz_version  *version;
    struct cork_buffer  rpm_version = CORK_BUFFER_INIT();
    fail_if_error(version = bz_version_from_string(buzzy));
    bz_version_to_rpm(version, &rpm_version);
    fail_unless_streq("Versions", rpm, rpm_version.buf);
    cork_buffer_done(&rpm_version);
    bz_version_free(version);
}

static void
test_version_from_rpm(const char *buzzy, const char *rpm)
{
    struct bz_version  *version;
    fail_if_error(version = bz_version_from_rpm(rpm));
    fail_unless_streq("Versions", buzzy, bz_version_to_string(version));
    bz_version_free(version);
}

static void
test_rpm_version(const char *buzzy, const char *rpm)
{
    test_version_to_rpm(buzzy, rpm);
    test_version_from_rpm(buzzy, rpm);
}

START_TEST(test_rpm_versions)
{
    DESCRIBE_TEST;
    test_rpm_version("2.0", "2.0-1");
    test_rpm_version("2.0~alpha", "2.0-0.alpha.1");
    test_rpm_version("2.0~alpha.1", "2.0-0.alpha.1.1.1");
    test_rpm_version("2.0~1", "2.0-0.1.1");
    test_rpm_version("2.0+hotfix.1", "2.0-2.hotfix.1.1.1");
    test_rpm_version("2.0+1", "2.0-2.1.1");
    test_rpm_version("2.0+git+20130529", "2.0-2.git.2.20130529.1");
    test_rpm_version("2.0+rev+2", "2.0-2.rev.2.2.1");

    /* A bunch of examples from
     * https://fedoraproject.org/wiki/Packaging:NamingGuidelines#Package_Versioning
     */
    test_version_from_rpm("1.4~1", "1.4-0.1");
    test_version_from_rpm("1.4~1.a", "1.4-0.1.a");
    test_version_from_rpm("1.4~2.a", "1.4-0.2.a");
    test_version_from_rpm("1.4~3.a", "1.4-0.3.a");
    test_version_from_rpm("1.4~4.b", "1.4-0.4.b");
    test_version_from_rpm("1.4~5.b", "1.4-0.5.b");
    test_version_from_rpm("1.4", "1.4-1");
    test_version_from_rpm("1.4+1.a", "1.4-1.a");
    test_version_from_rpm("1.4+2", "1.4-2");
    test_version_from_rpm("1.4+2.1", "1.4-2.1");
    test_version_from_rpm("1.4+2.1.a", "1.4-2.1.a");

    test_version_from_rpm("0.9.2~1.beta.1", "0.9.2-0.1.beta1");
    test_version_from_rpm("0.9.2~2.beta.1", "0.9.2-0.2.beta1");

    test_version_from_rpm("0~1.20040110.svn", "0-0.1.20040110svn");
    test_version_from_rpm("0~2.20040110.svn", "0-0.2.20040110svn");
    test_version_from_rpm("0~3.20040204.svn", "0-0.3.20040204svn");
    test_version_from_rpm("1.0", "1.0-1");

    test_version_from_rpm("1.1.0~1.BETA", "1.1.0-0.1.BETA");
    test_version_from_rpm("1.1.0~2.BETA.1", "1.1.0-0.2.BETA1");
    test_version_from_rpm("1.1.0~3.BETA.2", "1.1.0-0.3.BETA2");
    test_version_from_rpm("1.1.0~4.CR.1", "1.1.0-0.4.CR1");
    test_version_from_rpm("1.1.0~5.CR.2", "1.1.0-0.5.CR2");
    test_version_from_rpm("1.1.0", "1.1.0-1");
    test_version_from_rpm("1.1.0+2.GA.1", "1.1.0-2.GA1");
    test_version_from_rpm("1.1.0+3.CP.1", "1.1.0-3.CP1");
    test_version_from_rpm("1.1.0+4.CP.2", "1.1.0-4.CP2");
    test_version_from_rpm("1.1.0+5.SP.1", "1.1.0-5.SP1");
    test_version_from_rpm("1.1.0+6.SP.1.CP.1", "1.1.0-6.SP1_CP1");

    /* And some tests of our own */
    test_version_from_rpm("0.9.8", "0.9.8-1");
    test_version_from_rpm("0.9.8+1.1.el.6", "0.9.8-1.1.el6");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native packages
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the RPM Linux packaging tools
 * to actually be installed. */

START_TEST(test_yum_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, but has not
     * yet been installed. */
    reset_everything();
    bz_start_mocks();
    mock_available_package("jansson", "2.4");
    mock_uninstalled_package("jansson");

    fail_if_error(version = bz_yum_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_rpm_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST

START_TEST(test_yum_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that is available in the native package database, and has been
     * installed. */
    reset_everything();
    bz_start_mocks();
    mock_available_package("jansson", "2.4");
    mock_installed_package("jansson", "2.4");

    fail_if_error(version = bz_yum_native_version_available("jansson"));
    test_and_free_version(version, "2.4");

    fail_if_error(version = bz_rpm_native_version_installed("jansson"));
    test_and_free_version(version, "2.4");
}
END_TEST

START_TEST(test_yum_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    /* A package that isn't available in the native package database. */
    reset_everything();
    bz_start_mocks();
    mock_unavailable_package("jansson");
    mock_uninstalled_package("jansson");

    fail_if_error(version = bz_yum_native_version_available("jansson"));
    fail_unless(version == NULL, "Unexpected version");

    fail_if_error(version = bz_rpm_native_version_installed("jansson"));
    fail_unless(version == NULL, "Unexpected version");
}
END_TEST


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the RPM packaging tools to
 * actually be installed. */

static void
test_yum_pdb_dep(struct bz_pdb *pdb, const char *dep_str,
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
test_yum_pdb_unknown_dep(struct bz_pdb *pdb, const char *dep_str)
{
    struct bz_dependency  *dep;
    struct bz_package  *package;
    fail_if_error(dep = bz_dependency_from_string(dep_str));
    fail_if_error(package = bz_pdb_satisfy_dependency(pdb, dep, NULL));
    fail_unless(package == NULL, "Should not be able to build %s", dep_str);
    bz_dependency_free(dep);
}

START_TEST(test_yum_pdb_uninstalled_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, but has not
     * yet been installed. */
    bz_start_mocks();
    mock_unavailable_package("jansson-devel");
    mock_unavailable_package("libjansson-devel");
    mock_available_package("jansson", "2.4");
    mock_uninstalled_package("jansson");
    mock_package_installation("jansson", "2.4");

    fail_if_error(pdb = bz_yum_native_pdb());

    test_yum_pdb_dep(pdb, "jansson",
        "[1] Install native RPM package jansson 2.4\n"
    );

    test_yum_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native RPM package jansson 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_yum_pdb_uninstalled_native_package_02)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* Test that if we try to install the same dependency twice, the second
     * attempt is a no-op. */
    bz_start_mocks();
    mock_unavailable_package("jansson-devel");
    mock_unavailable_package("libjansson-devel");
    mock_available_package("jansson", "2.4");
    mock_uninstalled_package("jansson");
    mock_package_installation("jansson", "2.4");

    fail_if_error(pdb = bz_yum_native_pdb());

    test_yum_pdb_dep(pdb, "jansson",
        "[1] Install native RPM package jansson 2.4\n"
    );

    test_yum_pdb_dep(pdb, "jansson", "Nothing to do!\n");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_yum_pdb_installed_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that is available in the native package database, and has been
     * installed. */
    bz_start_mocks();
    mock_unavailable_package("jansson-devel");
    mock_unavailable_package("libjansson-devel");
    mock_available_package("jansson", "2.4");
    mock_installed_package("jansson", "2.4");

    fail_if_error(pdb = bz_yum_native_pdb());

    test_yum_pdb_dep(pdb, "jansson", "Nothing to do!\n");
    test_yum_pdb_dep(pdb, "jansson >= 2.4", "Nothing to do!\n");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_yum_pdb_nonexistent_native_package_01)
{
    DESCRIBE_TEST;
    struct bz_pdb  *pdb;

    /* A package that isn't available in the native package database. */
    bz_start_mocks();
    mock_unavailable_package("jansson-devel");
    mock_unavailable_package("libjansson-devel");
    mock_unavailable_package("jansson");
    mock_unavailable_package("libjansson");

    fail_if_error(pdb = bz_yum_native_pdb());

    test_yum_pdb_unknown_dep(pdb, "jansson");
    test_yum_pdb_unknown_dep(pdb, "jansson >= 2.4");

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_yum_pdb_uninstalled_override_package_01)
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

    fail_if_error(pdb = bz_yum_native_pdb());
    bz_env_add_override
        (env, "native.jansson", bz_string_value_new("libjansson0"));

    test_yum_pdb_dep(pdb, "jansson",
        "[1] Install native RPM package libjansson0 2.4\n"
    );

    test_yum_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native RPM package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST

START_TEST(test_yum_pdb_uninstalled_override_package_02)
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

    fail_if_error(pdb = bz_yum_native_pdb());
    bz_env_add_override
        (env, "native.rpm.jansson", bz_string_value_new("libjansson0"));

    test_yum_pdb_dep(pdb, "jansson",
        "[1] Install native RPM package libjansson0 2.4\n"
    );

    test_yum_pdb_dep(pdb, "jansson >= 2.4",
        "[1] Install native RPM package libjansson0 2.4\n"
    );

    bz_pdb_free(pdb);
}
END_TEST


/*-----------------------------------------------------------------------
 * Building RPM packages
 */

/* Since we're mocking the subprocess commands for each of these test cases, the
 * tests can run on any platform; we don't need the RPM Linux packaging tools
 * to actually be installed. */

static void
mock_rpmbuild(const char *package_name, const char *version)
{
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    cork_buffer_printf(&buf,
        "rpmbuild "
        "--define _sourcedir . "
        "--define _rpmdir . "
        "--define _builddir . "
        "--define buildroot /tmp/staging "
        "--define _srcrpmdir . "
        "--define _specdir . "
        "--define _build_name_fmt "
            "%%%%{NAME}-%%%%{VERSION}-%%%%{RELEASE}.%%%%{ARCH}.rpm "
        "--quiet -bb "
        "/home/test/.cache/buzzy/build/%s/%s/pkg/%s.spec",
        package_name, version, package_name
    );
    bz_mock_subprocess(buf.buf, NULL, NULL, 0);
    cork_buffer_done(&buf);
}

static void
test_create_package(struct bz_env *env, bool force,
                    const char *expected_actions)
{
    struct cork_path  *binary_package_dir = cork_path_new(".");
    struct cork_path  *staging_dir = cork_path_new("/tmp/staging");
    struct bz_pdb  *pdb;
    struct bz_packager  *packager;

    fail_if_error(pdb = bz_yum_native_pdb());
    bz_pdb_register(pdb);

    mock_unavailable_package("librpm-build");
    mock_unavailable_package("librpm-build-devel");
    mock_unavailable_package("rpm-build-devel");
    mock_available_package("rpm-build", "4.8.0");
    mock_installed_package("rpm-build", "4.8.0");
    bz_mock_file_exists(cork_path_get(staging_dir), true);
    bz_env_add_override(env, "binary_package_dir",
                        bz_path_value_new(binary_package_dir));
    bz_env_add_override(env, "staging_dir", bz_path_value_new(staging_dir));
    bz_env_add_override(env, "force", bz_string_value_new(force? "1": "0"));
    bz_env_add_override(env, "verbose", bz_string_value_new("0"));
    fail_if_error(packager = bz_rpm_packager_new(env));
    fail_if_error(bz_packager_package(packager));
    test_actions(expected_actions);
    bz_packager_free(packager);
}

START_TEST(test_rpm_create_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    mock_rpmbuild("jansson", "2.4");
    bz_mock_file_exists("./jansson-2.4-1.x86_64.rpm", false);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_create_package(env, false,
        "[1] Package jansson 2.4 (RPM)\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1.x86_64.rpm ]\n"
        "$ yum info --cacheonly rpm-build-devel\n"
        "$ yum info --cacheonly librpm-build-devel\n"
        "$ yum info --cacheonly rpm-build\n"
        "$ rpm --qf %{V}-%{R} -q rpm-build\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec"
            " <<EOF\n"
        "Summary: jansson\n"
        "Name: jansson\n"
        "Version: 2.4\n"
        "Release: 1\n"
        "License: unknown\n"
        "Group: Buzzy\n"
        "Source: .\n"
        "BuildRoot: /tmp/staging\n"
        "\n"
        "%description\n"
        "No package description\n"
        "\n"
        "%prep\n"
        "\n"
        "%clean\n"
        "\n"
        "%files\n"
        "EOF\n"
        "$ rpmbuild "
            "--define _sourcedir . "
            "--define _rpmdir . "
            "--define _builddir . "
            "--define buildroot /tmp/staging "
            "--define _srcrpmdir . "
            "--define _specdir . "
            "--define _build_name_fmt "
                "%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm "
            "--quiet -bb "
            "/home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec\n"
    );
    bz_env_free(env);
}
END_TEST

START_TEST(test_rpm_create_package_license_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    mock_rpmbuild("jansson", "2.4");
    bz_mock_file_exists("./jansson-2.4-1.x86_64.rpm", false);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    fail_if_error(bz_env_add_override
                  (env, "license", bz_string_value_new("MIT")));
    test_create_package(env, false,
        "[1] Package jansson 2.4 (RPM)\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1.x86_64.rpm ]\n"
        "$ yum info --cacheonly rpm-build-devel\n"
        "$ yum info --cacheonly librpm-build-devel\n"
        "$ yum info --cacheonly rpm-build\n"
        "$ rpm --qf %{V}-%{R} -q rpm-build\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec"
            " <<EOF\n"
        "Summary: jansson\n"
        "Name: jansson\n"
        "Version: 2.4\n"
        "Release: 1\n"
        "License: MIT\n"
        "Group: Buzzy\n"
        "Source: .\n"
        "BuildRoot: /tmp/staging\n"
        "\n"
        "%description\n"
        "No package description\n"
        "\n"
        "%prep\n"
        "\n"
        "%clean\n"
        "\n"
        "%files\n"
        "EOF\n"
        "$ rpmbuild "
            "--define _sourcedir . "
            "--define _rpmdir . "
            "--define _builddir . "
            "--define buildroot /tmp/staging "
            "--define _srcrpmdir . "
            "--define _specdir . "
            "--define _build_name_fmt "
                "%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm "
            "--quiet -bb "
            "/home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec\n"
    );
    bz_env_free(env);
}
END_TEST

START_TEST(test_rpm_create_package_deps_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_array  *deps;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    mock_rpmbuild("jansson", "2.4");
    bz_mock_file_exists("./jansson-2.4-1.x86_64.rpm", false);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    deps = bz_array_new();
    bz_array_append(deps, bz_string_value_new("libfoo"));
    bz_array_append(deps, bz_string_value_new("libbar >= 2.5~alpha.1"));
    fail_if_error(bz_env_add_override
                  (env, "dependencies", bz_array_as_value(deps)));
    test_create_package(env, false,
        "[1] Package jansson 2.4 (RPM)\n"
    );
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1.x86_64.rpm ]\n"
        "$ yum info --cacheonly rpm-build-devel\n"
        "$ yum info --cacheonly librpm-build-devel\n"
        "$ yum info --cacheonly rpm-build\n"
        "$ rpm --qf %{V}-%{R} -q rpm-build\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec"
            " <<EOF\n"
        "Summary: jansson\n"
        "Name: jansson\n"
        "Version: 2.4\n"
        "Release: 1\n"
        "License: unknown\n"
        "Group: Buzzy\n"
        "Source: .\n"
        "BuildRoot: /tmp/staging\n"
        "Requires: libfoo, libbar >= 2.5-0.alpha.1.1.1\n"
        "\n"
        "%description\n"
        "No package description\n"
        "\n"
        "%prep\n"
        "\n"
        "%clean\n"
        "\n"
        "%files\n"
        "EOF\n"
        "$ rpmbuild "
            "--define _sourcedir . "
            "--define _rpmdir . "
            "--define _builddir . "
            "--define buildroot /tmp/staging "
            "--define _srcrpmdir . "
            "--define _specdir . "
            "--define _build_name_fmt "
                "%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm "
            "--quiet -bb "
            "/home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec\n"
    );
    bz_env_free(env);
}
END_TEST

START_TEST(test_rpm_create_existing_package_01)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    bz_mock_file_exists("./jansson-2.4-1.x86_64.rpm", true);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_create_package(env, false, "Nothing to do!\n");
    verify_commands_run(
        "$ uname -m\n"
        "$ [ -f ./jansson-2.4-1.x86_64.rpm ]\n"
    );
    bz_env_free(env);
}
END_TEST

START_TEST(test_rpm_create_existing_package_02)
{
    DESCRIBE_TEST;
    struct bz_version  *version;
    struct bz_env  *env;
    reset_everything();
    bz_start_mocks();
    bz_mock_subprocess("uname -m", "x86_64\n", NULL, 0);
    mock_rpmbuild("jansson", "2.4");
    bz_mock_file_exists("./jansson-2.4-1.x86_64.rpm", true);
    fail_if_error(version = bz_version_from_string("2.4"));
    fail_if_error(env = bz_package_env_new(NULL, "jansson", version));
    test_create_package(env, true,
        "[1] Package jansson 2.4 (RPM)\n"
    );
    verify_commands_run(
        "$ yum info --cacheonly rpm-build-devel\n"
        "$ yum info --cacheonly librpm-build-devel\n"
        "$ yum info --cacheonly rpm-build\n"
        "$ rpm --qf %{V}-%{R} -q rpm-build\n"
        "$ uname -m\n"
        "$ [ -f /tmp/staging ]\n"
        "$ mkdir -p /home/test/.cache/buzzy/build/jansson/2.4/pkg\n"
        "$ mkdir -p .\n"
        "$ cat > /home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec"
            " <<EOF\n"
        "Summary: jansson\n"
        "Name: jansson\n"
        "Version: 2.4\n"
        "Release: 1\n"
        "License: unknown\n"
        "Group: Buzzy\n"
        "Source: .\n"
        "BuildRoot: /tmp/staging\n"
        "\n"
        "%description\n"
        "No package description\n"
        "\n"
        "%prep\n"
        "\n"
        "%clean\n"
        "\n"
        "%files\n"
        "EOF\n"
        "$ rpmbuild "
            "--define _sourcedir . "
            "--define _rpmdir . "
            "--define _builddir . "
            "--define buildroot /tmp/staging "
            "--define _srcrpmdir . "
            "--define _specdir . "
            "--define _build_name_fmt "
                "%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm "
            "--quiet -bb "
            "/home/test/.cache/buzzy/build/jansson/2.4/pkg/jansson.spec\n"
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
    Suite  *s = suite_create("rpm");

    TCase  *tc_rpm = tcase_create("rpm");
    tcase_add_test(tc_rpm, test_redhat_detect);
    tcase_add_test(tc_rpm, test_rpm_versions);
    tcase_add_test(tc_rpm, test_yum_uninstalled_native_package_01);
    tcase_add_test(tc_rpm, test_yum_installed_native_package_01);
    tcase_add_test(tc_rpm, test_yum_nonexistent_native_package_01);
    suite_add_tcase(s, tc_rpm);

    TCase  *tc_yum_pdb = tcase_create("yum-pdb");
    tcase_add_test(tc_yum_pdb, test_yum_pdb_uninstalled_native_package_01);
    tcase_add_test(tc_yum_pdb, test_yum_pdb_uninstalled_native_package_02);
    tcase_add_test(tc_yum_pdb, test_yum_pdb_installed_native_package_01);
    tcase_add_test(tc_yum_pdb, test_yum_pdb_nonexistent_native_package_01);
    tcase_add_test(tc_yum_pdb, test_yum_pdb_uninstalled_override_package_01);
    tcase_add_test(tc_yum_pdb, test_yum_pdb_uninstalled_override_package_02);
    suite_add_tcase(s, tc_yum_pdb);

    TCase  *tc_rpm_package = tcase_create("rpm-package");
    tcase_add_test(tc_rpm_package, test_rpm_create_package_01);
    tcase_add_test(tc_rpm_package, test_rpm_create_package_license_01);
    tcase_add_test(tc_rpm_package, test_rpm_create_package_deps_01);
    tcase_add_test(tc_rpm_package, test_rpm_create_existing_package_01);
    tcase_add_test(tc_rpm_package, test_rpm_create_existing_package_02);
    suite_add_tcase(s, tc_rpm_package);

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
