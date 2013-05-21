Create a bunch of fake repositories to test whether we can detect all of the
builders that we know about.

  $ mkdir -p unknown-repo/.buzzy
  $ cat > unknown-repo/.buzzy/package.yaml <<EOF
  > name: test
  > version: 1.0~rc1
  > EOF
  $ cd unknown-repo
  $ buzzy doc builder
  builder
    What build system is used to build the package
  $ cd ..

  $ mkdir -p cmake-repo/.buzzy
  $ cat > cmake-repo/.buzzy/package.yaml <<EOF
  > name: test
  > version: 1.0~rc1
  > EOF
  $ touch cmake-repo/CMakeLists.txt
  $ cd cmake-repo
  $ buzzy doc builder
  builder
    What build system is used to build the package
  
    Current value: cmake
  $ cd ..
