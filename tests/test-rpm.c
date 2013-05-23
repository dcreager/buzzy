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
#include "buzzy/distro/rpm.h"

#include "helpers.h"


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

START_TEST(test_rpm_versions)
{
    DESCRIBE_TEST;
    test_version_to_rpm("2.0", "2.0-1");
    test_version_to_rpm("2.0~alpha", "2.0-0.alpha.1");
    test_version_to_rpm("2.0~1", "2.0-0.pre1.1");
    test_version_to_rpm("2.0+hotfix1", "2.0-2.hotfix1.1");
    test_version_to_rpm("2.0+1", "2.0-2.post1.1");
    test_version_to_rpm("2.0+git123abc456", "2.0-2.git123abc456.1");
    test_version_to_rpm("2.0+rev2", "2.0-2.rev2.1");

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
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("rpm");

    TCase  *tc_rpm = tcase_create("rpm");
    tcase_add_test(tc_rpm, test_rpm_versions);
    suite_add_tcase(s, tc_rpm);

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
