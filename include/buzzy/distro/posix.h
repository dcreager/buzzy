/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_POSIX_H
#define BUZZY_DISTRO_POSIX_H

#include <libcork/core.h>


/*-----------------------------------------------------------------------
 * Platform detection
 */

const char *
bz_posix_current_architecture(void);

struct bz_value *
bz_posix_architecture_value_new(void);


#endif /* BUZZY_DISTRO_POSIX_H */
