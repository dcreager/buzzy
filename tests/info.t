Reproducible directory names.

  $ TEST_TMP=$PWD
  $ export HOME=/home/test
  $ export XDG_RUNTIME_DIR=/run/users/test
  $ unset XDG_CACHE_HOME
  $ unset XDG_CACHE_DIRS
  $ unset XDG_DATA_HOME
  $ unset XDG_DATA_DIRS

Prepare a handful of repository directories, and then verify the output when we
run "buzzy info" in each of them.

  $ mkdir -p not-a-repo
  $ cd not-a-repo
  $ buzzy info
  No repositories found!
  $ cd ..
  $ rm -rf not-a-repo

  $ mkdir -p repo1/.buzzy
  $ cd repo1
  $ buzzy info
  Repositories:
    .*/info.t/repo1 (re)
  $ cd ..
  $ rm -rf repo1

  $ mkdir -p repo1/.buzzy
  $ cat > repo1/.buzzy/links.yaml <<EOF
  > - $TEST_TMP/repo2
  > EOF
  $ mkdir -p repo2/.buzzy
  $ cd repo1
  $ buzzy info
  Repositories:
    .*/info.t/repo1 (re)
    .*/info.t/repo2 (re)
  $ cd ..
  $ rm -rf repo1 repo2

  $ mkdir -p repo1/.buzzy
  $ cat > repo1/.buzzy/links.yaml <<EOF
  > - file://$TEST_TMP/repo2
  > EOF
  $ mkdir -p repo2/.buzzy
  $ cd repo1
  $ buzzy info
  Repositories:
    .*/info.t/repo1 (re)
    .*/info.t/repo2 (re)
  $ cd ..
  $ rm -rf repo1 repo2
