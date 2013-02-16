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


#endif /* BUZZY_DISTRO_ARCH_H */
