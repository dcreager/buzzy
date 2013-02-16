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
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("arch");

    TCase  *tc_arch = tcase_create("arch");
    tcase_add_test(tc_arch, test_arch_detect);
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
