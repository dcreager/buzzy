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
    reset_everything();
    bz_start_mocks();
    bz_mock_file_exists("/a/b/c/.buzzy", false);
    bz_mock_file_exists("/a/b", true);
    bz_mock_file_exists("/a/b/.buzzy", true);
    bz_mock_file_exists("/a/b/.buzzy/repo.yaml", false);
    bz_mock_file_exists("/a/b/.buzzy/links.yaml", false);
    bz_mock_file_exists("/a/b/.buzzy/package.yaml", false);
    bz_mock_file_exists("/a/b/.git", false);
    fail_if_error(repo = bz_local_filesystem_repo_find("/a/b/c"));
    fail_if(repo == NULL, "Cannot create repo");
    fail_if_error(bz_repo_registry_load_all());
    test_actions(
        "Nothing to do!\n"
    );
}
END_TEST

START_TEST(test_git_repo_01)
{
    DESCRIBE_TEST;
    struct bz_repo  *repo;
    reset_everything();
    bz_start_mocks();
    /* The repo directory itself is mocked to not exist, so that we perform the
     * clone. */
    bz_mock_file_exists
        ("/home/test/.cache/buzzy/repos/buzzy-test-c0bd3d81", false);
    bz_mock_file_exists
        ("/home/test/.cache/buzzy/repos/buzzy-test-c0bd3d81/"
         ".buzzy/repo.yaml", false);
    bz_mock_file_exists
        ("/home/test/.cache/buzzy/repos/buzzy-test-c0bd3d81/"
         ".buzzy/links.yaml", false);
    bz_mock_file_exists
        ("/home/test/.cache/buzzy/repos/buzzy-test-c0bd3d81/"
         ".buzzy/package.yaml", false);
    bz_mock_file_exists
        ("/home/test/.cache/buzzy/repos/buzzy-test-c0bd3d81/"
         ".git", false);
    bz_mock_subprocess
        ("git clone --recursive --branch master "
         "git://github.com/dcreager/buzzy-test.git "
         "/home/test/.cache/buzzy/repos/buzzy-test-c0bd3d81",
         NULL, NULL, 0);
    fail_if_error(repo = bz_git_repo_new
                  ("git://github.com/dcreager/buzzy-test.git", "master"));
    fail_if(repo == NULL, "Cannot create repo");
    fail_if_error(bz_repo_load(repo));
    test_actions(
        "[1] Clone git://github.com/dcreager/buzzy-test.git (master)\n"
    );
    bz_repo_free(repo);
}
END_TEST

START_TEST(test_missing_repo_01)
{
    DESCRIBE_TEST;
    struct bz_repo  *repo;
    reset_everything();
    bz_start_mocks();
    bz_mock_file_exists("/a/b/c/.buzzy", false);
    bz_mock_file_exists("/a/b/.buzzy", false);
    bz_mock_file_exists("/a/.buzzy", false);
    bz_mock_file_exists("/.buzzy", false);
    fail_if_error(repo = bz_local_filesystem_repo_find("/a/b/c"));
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
    tcase_add_test(tc_repo, test_git_repo_01);
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
