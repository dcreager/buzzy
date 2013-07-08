Compare a bunch of Buzzy version strings from the command line.

Basic version comparisons.
  $ buzzy vercmp 2.0   2.0.0
  0
  $ buzzy vercmp 2.0   2.0.1
  -1
  $ buzzy vercmp 2.0   2.1
  -1
  $ buzzy vercmp 2.0.0 2.0.1
  -1
  $ buzzy vercmp 2.0.0 2.1
  -1
  $ buzzy vercmp 2.0.1 2.1
  -1

2.0's alpha release is older than everything else.
  $ buzzy vercmp 2.0~alpha 2.0
  -1
  $ buzzy vercmp 3.0~alpha 2.0.0
  1
  $ buzzy vercmp 2.0~alpha 2.0+dev
  -1
  $ buzzy vercmp 2.0~alpha 2.0.1
  -1
  $ buzzy vercmp 2.0~alpha 2.1
  -1

2.0's dev postrelease is newer than the other 2.0 versions, but older than
everything else.
  $ buzzy vercmp 2.0+dev 2.0~alpha
  1
  $ buzzy vercmp 2.0+dev 2.0
  1
  $ buzzy vercmp 2.0+dev 2.0.0
  1
  $ buzzy vercmp 2.0+dev 2.0.1
  -1
  $ buzzy vercmp 2.0+dev 2.1
  -1
