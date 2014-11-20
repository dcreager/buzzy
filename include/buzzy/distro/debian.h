/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_DEBIAN_H
#define BUZZY_DISTRO_DEBIAN_H

#include <libcork/core.h>
#include <libcork/ds.h>

#include "buzzy/package.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_debian_is_present(bool *dest);

const char *
bz_debian_current_architecture(void);

struct bz_value *
bz_debian_architecture_value_new(void);


/*-----------------------------------------------------------------------
 * DEBIAN version strings
 */

void
bz_version_to_deb(struct bz_version *version, struct cork_buffer *dest);

struct bz_version *
bz_version_from_deb(const char *deb);


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Returns the version of the specified package that could be installed using
 * the native apt-get package repositories. */
struct bz_version *
bz_apt_native_version_available(const char *native_package_name);

/* Returns the version of the specified package that has been installed on the
 * current machine using apt-get.  That package need not have come from the
 * default native package database; it should return a result for packages that
 * we build and install ourselves, too. */
struct bz_version *
bz_deb_native_version_installed(const char *native_package_name);


/* A package database that can install native packages that are defined in a apt
 * database. */
struct bz_pdb *
bz_apt_native_pdb(void);


#endif /* BUZZY_DISTRO_DEBIAN_H */
