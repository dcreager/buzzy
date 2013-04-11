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

#include "buzzy/repo.h"

#include "helpers.h"


/*-----------------------------------------------------------------------
 * Finding a repository
 */

START_TEST(test_repo_01)
{
    DESCRIBE_TEST;
    struct bz_repo  *repo;
    bz_start_mocks();
    bz_repo_registry_reset();
    bz_global_env_reset();
    fail_if_error(bz_load_variable_definitions());
    bz_mock_file_exists("/a/b/c/.buzzy", false);
    bz_mock_file_exists("/a/b/.buzzy", true);
    bz_mock_file_exists("/a/b/.buzzy/repo.yaml", false);
    bz_mock_file_exists("/a/b/.buzzy/package.yaml", false);
    bz_mock_file_exists("/a/b/.buzzy/../.git", false);
    fail_if_error(repo = bz_filesystem_repo_find("/a/b/c"));
    fail_if(repo == NULL, "Cannot create repo");
}
END_TEST

START_TEST(test_missing_repo_01)
{
    DESCRIBE_TEST;
    struct bz_repo  *repo;
    bz_start_mocks();
    bz_repo_registry_reset();
    bz_global_env_reset();
    fail_if_error(bz_load_variable_definitions());
    bz_mock_file_exists("/a/b/c/.buzzy", false);
    bz_mock_file_exists("/a/b/.buzzy", false);
    bz_mock_file_exists("/a/.buzzy", false);
    bz_mock_file_exists("/.buzzy", false);
    fail_if_error(repo = bz_filesystem_repo_find("/a/b/c"));
    fail_unless(repo == NULL, "Shouldn't be able to find missing repo");
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("repo");

    TCase  *tc_repo = tcase_create("repo");
    tcase_add_test(tc_repo, test_repo_01);
    tcase_add_test(tc_repo, test_missing_repo_01);
    suite_add_tcase(s, tc_repo);

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