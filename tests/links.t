Test repositories that are linked together.

  $ TESTDIR="$PWD"
  $ export HOME=/home/test
  $ export XDG_RUNTIME_DIR=/run/users/test
  $ unset XDG_CACHE_HOME
  $ unset XDG_CACHE_DIRS
  $ unset XDG_DATA_HOME
  $ unset XDG_DATA_DIRS


Create a couple of filesystem repositories, with one linked to the other.

  $ mkdir -p parent-repo/.buzzy
  $ cat > parent-repo/.buzzy/repo.yaml <<EOF
  > var1: hello
  > var2: \${var1} world
  > EOF

  $ mkdir -p child-repo/.buzzy
  $ cat > child-repo/.buzzy/links.yaml <<EOF
  > - $TESTDIR/parent-repo
  > EOF
  $ cat > child-repo/.buzzy/repo.yaml <<EOF
  > var1: goodbye
  > EOF


  $ cd parent-repo
  $ buzzy doc var1
  No documentation for var1
  
    Current value: hello
  $ buzzy doc var2
  No documentation for var2
  
    Current value: hello world
  $ cd ..


  $ cd child-repo
  $ buzzy doc var1
  No documentation for var1
  
    Current value: goodbye
  $ buzzy doc var2
  No documentation for var2
  
    Current value: goodbye world
  $ cd ..
