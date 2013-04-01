  $ export HOME=/home/test
  $ export XDG_RUNTIME_DIR=/run/users/test
  $ unset XDG_CACHE_HOME
  $ unset XDG_CACHE_DIRS
  $ unset XDG_DATA_HOME
  $ unset XDG_DATA_DIRS

  $ buzzy doc not_a_real_variable
  No variable named not_a_real_variable
  [1]

  $ buzzy doc cache_path
  cache_path
    A directory for user-specific nonessential data files
  
    On POSIX systems, this defaults to the value of the $XDG_CACHE_HOME environment variable, or $HOME/.cache if that's not defined.  Note that this is not a Buzzy-specific directory; this should refer to the root of the current user's cache directory.
  
    Current value: /home/test/.cache

  $ buzzy doc work_path
  work_path
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy
