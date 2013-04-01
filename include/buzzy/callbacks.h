/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_CALLBACKS_H
#define BUZZY_CALLBACKS_H

#include <libcork/core.h>


typedef void
(*bz_free_f)(void *user_data);

#define bz_user_data_free(obj) \
    do { \
        if ((obj)->user_data_free != NULL) { \
            (obj)->user_data_free((obj)->user_data); \
        } \
    } while (0)


#endif /* BUZZY_CALLBACKS_H */
