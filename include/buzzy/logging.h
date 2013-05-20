/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_LOGGING_H
#define BUZZY_LOGGING_H

#include <libcork/core.h>


/*-----------------------------------------------------------------------
 * Actions
 */

void
bz_log_action(const char *fmt, ...)
    CORK_ATTR_PRINTF(1, 2);

void
bz_reset_action_count(void);


#endif /* BUZZY_LOGGING_H */
