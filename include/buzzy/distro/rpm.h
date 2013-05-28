/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_RPM_H
#define BUZZY_DISTRO_RPM_H

#include <libcork/core.h>
#include <libcork/ds.h>

#include "buzzy/package.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_redhat_is_present(bool *dest);


/*-----------------------------------------------------------------------
 * RPM version strings
 */

void
bz_version_to_rpm(struct bz_version *version, struct cork_buffer *dest);

struct bz_version *
bz_version_from_rpm(const char *rpm);


/*-----------------------------------------------------------------------
 * Native package database
 */

/* Returns the version of the specified package that could be installed using
 * the native RPM/Yum package repositories. */
struct bz_version *
bz_yum_native_version_available(const char *native_package_name);

/* Returns the version of the specified package that has been installed on the
 * current machine using RPM (or anything that delegates to RPM for actual
 * package installation, such as yum).  That package need not have come from the
 * default native package database; it should return a result for packages that
 * we build and install ourselves, too. */
struct bz_version *
bz_rpm_native_version_installed(const char *native_package_name);


/* A package database that can install native packages that are defined in a yum
 * database. */
struct bz_pdb *
bz_yum_native_pdb(void);


#endif /* BUZZY_DISTRO_RPM_H */
