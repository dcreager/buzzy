/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_ARCH_H
#define BUZZY_DISTRO_ARCH_H

#include <libcork/core.h>
#include <libcork/ds.h>

#include "buzzy/env.h"
#include "buzzy/package.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_arch_is_present(bool *dest);


/*-----------------------------------------------------------------------
 * Arch version strings
 */

void
bz_version_to_arch(struct bz_version *version, struct cork_buffer *dest);

struct bz_version *
bz_version_from_arch(const char *arch_version);


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Returns the version of the specified package that could be installed using
 * the native pacman package repositories. */
struct bz_version *
bz_arch_native_version_available(const char *native_package_name);

/* Returns the version of the specified package that has been installed on the
 * current machine using pacman.  That package need not have come from the
 * default Arch package database; it should return a result for packages that we
 * build and install ourselves, too. */
struct bz_version *
bz_arch_native_version_installed(const char *native_package_name);


struct bz_pdb *
bz_arch_native_pdb(void);


#endif /* BUZZY_DISTRO_ARCH_H */
