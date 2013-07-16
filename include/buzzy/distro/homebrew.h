/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_HOMEBREW_H
#define BUZZY_DISTRO_HOMEBREW_H

#include <libcork/core.h>

#include "buzzy/package.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_homebrew_is_present(bool *dest);


/*-----------------------------------------------------------------------
 * Homebrew-specific values
 */

struct bz_value *
bz_homebrew_pkgconfig_path_value_new(void);


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Returns the version of the specified package that could be installed using
 * the native pacman package repositories. */
struct bz_version *
bz_homebrew_native_version_available(const char *native_package_name);

/* Returns the version of the specified package that has been installed on the
 * current machine using pacman.  That package need not have come from the
 * default homebrew package database; it should return a result for packages that we
 * build and install ourselves, too. */
struct bz_version *
bz_homebrew_native_version_installed(const char *native_package_name);


struct bz_pdb *
bz_homebrew_native_pdb(void);


#endif /* BUZZY_DISTRO_HOMEBREW_H */
