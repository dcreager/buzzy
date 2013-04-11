/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_NATIVE_H
#define BUZZY_NATIVE_H

#include <libcork/core.h>

#include "buzzy/action.h"
#include "buzzy/package.h"
#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * Native packages
 */

typedef struct bz_version *
(*bz_native_detect_f)(const char *native_package_name);

typedef int
(*bz_native_install_f)(const char *native_package_name,
                       struct bz_version *version);

/* Takes control of version */
struct bz_package *
bz_native_package_new(const char *short_distro_name,
                      const char *package_name, const char *native_package_name,
                      struct bz_version *version,
                      bz_native_detect_f version_installed,
                      bz_native_install_f install);


/*-----------------------------------------------------------------------
 * Native package databases
 */

/* Each pattern is a printf format that must contain exactly one %s.  Each
 * pattern will be used to convert Buzzy package names into candidate native
 * package names. */
CORK_ATTR_SENTINEL
struct bz_pdb *
bz_native_pdb_new(const char *short_distro_name,
                  bz_native_detect_f version_available,
                  bz_native_detect_f version_installed,
                  bz_native_install_f install,
                  /* const char *pattern */ ...);


#endif /* BUZZY_NATIVE_H */
