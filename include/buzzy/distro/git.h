/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_GIT_H
#define BUZZY_DISTRO_GIT_H

#include <libcork/core.h>

#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * git version strings
 */

struct bz_version *
bz_version_from_git_describe(const char *git_version);


#endif /* BUZZY_DISTRO_GIT_H */
