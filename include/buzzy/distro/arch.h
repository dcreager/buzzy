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


#endif /* BUZZY_DISTRO_ARCH_H */
