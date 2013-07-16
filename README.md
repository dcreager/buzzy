# Buzzy package manager

Buzzy is a package manager that isn't tied to one particular operating system or
distribution.  It's useful for third-party software developers that would like
to provide native binary packages for the software that they write, without
having to maintain separate packaging scripts for a multitude of operating
systems, distributions, and versions.

Buzzy is currently alpha software, and only supports the following operating
systems:

* Arch Linux
* RedHat Enterprise Linux 5 and 6
* Mac OS X (using Homebrew)

We plan to add support for other Linux distributions and operating systems.

Buzzy is written in C, with all of its library dependencies included in the
source repository.  Then means that you don't need any language runtimes to
build, install, or use Buzzy.  (This is especially useful, since we don't have
to worry about juggling language versions across different operating systems.
I'm looking at you, Python on RHEL5!)


## Installing Buzzy

We use Buzzy to build native packages of Buzzy itself.  (It would be kind of
lame of us not to, don't you think?)  You can find binary package releases at
our [Github releases page](https://github.com/redjack/buzzy/releases/).  So the
easiest way to install Buzzy is to download one of those binaries and install it
using your normal package manager.  Buzzy (purposefully) has no runtime
dependencies, so the single binary is all you need.


## Building Buzzy

Presumably, your platform is one that Buzzy can build native packages for.
(Otherwise, what's the point of installing Buzzy?)  You can build your own
native Buzzy package in two stages.

First, follow a standard CMake setup to build a "bootstrap" copy of Buzzy:

    $ mkdir .build
    $ cd .build
    $ cmake ..
    $ make

Then use this bootstrap copy to build and install a proper native package of
Buzzy:

    $ src/buzzy install


## Overview

A Buzzy package is a platform-agnostic description of a piece of software.  It
usually includes instructions for downloading and building the software from
source, but doesn't have to — Buzzy packages can also refer to packages that are
available in your native package database.

Unlike most package managers, you do not have an arbitrary scripting language to
work with when writing a Buzzy package description.  Instead, a recipe is a
_declarative_ description of the software package, encoded in a YAML file.

Buzzy automatically translates this descriptive description of the software into
one or more platform-specific packages.  Different platforms might have
different rules for how many packages are created for a recipe, and will
certainly have different rules and customs for translating the descriptive
recipe instructions into the native packaging format.  Where possible, we use
native system packages to satisfy recipe dependencies, rather that building our
own custom copies of software that's already provided by the native package
manager.
