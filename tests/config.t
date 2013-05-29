Make sure that we can pick up configuration files that override global default
values.

Start with reproducible directory names.

  $ TESTDIR="$PWD"
  $ export HOME=/home/test
  $ export XDG_RUNTIME_DIR=/run/users/test
  $ unset XDG_CACHE_HOME
  $ unset XDG_CACHE_DIRS
  $ unset XDG_DATA_HOME
  $ unset XDG_DATA_DIRS


The baseline default value, which is precompiled.

  $ buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy


Create some configuration files that override the default.

  $ mkdir config-empty
  $ touch config-empty/buzzy.yaml

  $ mkdir config1
  $ cat > config1/buzzy.yaml <<EOF
  > work_dir: /home/override
  > EOF

  $ mkdir config2
  $ cat > config2/buzzy.yaml <<EOF
  > work_dir: /home/override2
  > EOF

  $ mkdir share-empty
  $ touch share-empty/buzzy.yaml

  $ mkdir -p share1/buzzy
  $ cat > share1/buzzy/config.yaml <<EOF
  > work_dir: /home/override
  > EOF


Load single configuration files in a variety of ways.

  $ XDG_CONFIG_HOME=$TESTDIR/config-empty buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy

  $ XDG_CONFIG_DIRS=$TESTDIR/config-empty buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy

  $ XDG_CONFIG_HOME=$TESTDIR/config1 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override

  $ XDG_CONFIG_DIRS=$TESTDIR/config1 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override

  $ XDG_CONFIG_HOME=$TESTDIR/config2 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override2

  $ XDG_CONFIG_DIRS=$TESTDIR/config2 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override2

  $ XDG_DATA_HOME=$TESTDIR/share-empty buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy

  $ XDG_DATA_DIRS=$TESTDIR/share-empty buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy

  $ XDG_DATA_HOME=$TESTDIR/share1 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override

  $ XDG_DATA_DIRS=$TESTDIR/share1 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override


Directories that appear earlier in the list should take precedence.

  $ XDG_CONFIG_DIRS=$TESTDIR/config1:$TESTDIR/config2 buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override


The user-provided configration files should take precedence over the
$PREFIX/share directories.

  $ XDG_DATA_DIRS=$TESTDIR/share1 \
  > XDG_CONFIG_DIRS=$TESTDIR/config2 \
  >     buzzy doc work_dir
  work_dir
    A directory for Buzzy's intermediate build products
  
    Current value: /home/override2
