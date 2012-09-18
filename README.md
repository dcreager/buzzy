# Buzzy package manager

Buzzy is a package manager that isn't tied to one particular operating system or
distribution.  It's useful for third-party software developers that would like
to provide native binary packages for the software that they write, without
having to maintain separate packaging scripts for a multitude of operating
systems, distributions, and versions.

Buzzy is currently alpha software, and only supports the following operating
systems:

* Arch Linux

We plan to add support for other Linux distributions, and other operating
systems.


Buzzy is written in Python, and should be compatible with both Python 2 and
Python 3.  (If you encounter any bugs with either Python version, please [file
an issue](https://github.com/redjack/buzzy/issues/).)


## Installing Buzzy

Standard [distutils](http://docs.python.org/distutils/index.html) setup:

    $ python setup.py build
    $ python setup.py install

(The final step might need to be run as root or via `sudo`.)

We also use Buzzy to provide native packages of Buzzy itself.  (It would be kind
of lame of us not to, don't you think?)  You can find binary package releases at
our [Github download page](https://github.com/redjack/buzzy/downloads/).


## Overview

Buzzy makes a distinction between _recipes_ and _packages_.  A recipe is a
platform-agnostic description of a piece of software.  It includes instructions
for downloading and building the software.  However, unlike most package
managers, you do not have an arbitrary scripting language to work with in a
recipe.  Instead, a recipe is a _declarative_ description of the software
package, encoded in a YAML file.

Buzzy automatically translates this descriptive description of the software into
one or more platform-specific packages.  Different platforms might have
different rules for how many packages are created for a recipe, and will
certainly have different rules and customs for translating the descriptive
recipe instructions into the native packaging format.


## Using Buzzy

All Buzzy commands operate in terms of an _environment_ and a _recipe database_.
The recipe database is a directory of YAML recipe descriptions.  Recipe
descriptions can (and should) refer to other recipes --- for instance, to
express dependencies between the packages that Buzzy will produce.  However, the
recipe database must be _self-contained_; i.e., all of the recipes that are
referred to must be in the database.

The environment is a workspace where Buzzy will create the files needed to build
packages on the current platform.  Buzzy does not support cross-compilation; you
must build packages for a particular platform on a machine running that
platform.  Moreover, you must have administrative access (ideally via `sudo`),
so that Buzzy can install packages to satisfy any dependencies that we need to
perform builds.

The recipe database and environment should be separate locations on the location
filesystem.  The recipe database should ideally be maintained in some source
code repository.  This design lets you check out the recipe database (which
doesn't contain anything specific to any platform) onto multiple build machines,
and create environment directories on each of those machines to hold the
platform-specific build artefacts.

Obtaining a copy of your recipe database is outside the scope of these
instructions; use whatever mechanism you need to download or check out the
latest copy.

Before you can perform any Buzzy operations, you must initialize and configure
your environment directory.  Choose a directory to hold your environment.  In
the following instructions, we will call this `$ENV_DIR`; you should replace
that with the actual directory you've chosen.

    $ mkdir $ENV_DIR
    $ cd $ENV_DIR
    $ buzzy configure

Note that all Buzzy commands must be run from the environment directory.  The
`buzzy configure` command will ask you a series of questions about the recipe
database and the packages that you would like to produce.

Once you've run `buzzy configure`, you can build or install individual recipes:

    $ buzzy build $RECIPE_NAME
    $ buzzy install $RECIPE_NAME

Or update all of the recipes in the database:

    $ buzzy update

Installing a recipe will build it first, if needed.  And building or installing
a package will also make sure that all of its dependencies are built and
installed, too.

The packages that are built will be placed into the directory that you specified
during the `buzzy configure` step.  Buzzy will make sure that this directory is
a valid package repository for the current platform.  You can then upload these
packages to a remote server, if desired, using whatever mechanism is right for
your setup.
