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

#include "buzzy/version.h"


/*-----------------------------------------------------------------------
 * RPM version strings
 */

void
bz_version_to_rpm(struct bz_version *version, struct cork_buffer *dest);

struct bz_version *
bz_version_from_rpm(const char *rpm);


#endif /* BUZZY_DISTRO_RPM_H */
