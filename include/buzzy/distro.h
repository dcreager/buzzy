/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_DISTRO_H
#define BUZZY_DISTRO_H

#include <libcork/core.h>


/*-----------------------------------------------------------------------
 * Detect built-in package databases
 */

int
bz_distro_add_env_overrides(void);

int
bz_pdb_discover(void);


#endif /* BUZZY_DISTRO_H */
