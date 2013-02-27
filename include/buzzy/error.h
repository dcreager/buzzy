/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_ERROR_H
#define BUZZY_ERROR_H

#include <libcork/core.h>


/*-----------------------------------------------------------------------
 * Error handling
 */

/* hash of "buzzy.h" */
#define BZ_ERROR  0xb8a66a70

enum bz_error {
    BZ_CIRCULAR_ACTIONS,
    BZ_INVALID_DEPENDENCY,
    BZ_INVALID_VERSION,
    BZ_SUBPROCESS_ERROR
};

#define bz_set_error(code, ...) (cork_error_set(BZ_ERROR, code, __VA_ARGS__))
#define bz_circular_actions(...) \
    bz_set_error(BZ_CIRCULAR_ACTIONS, __VA_ARGS__)
#define bz_invalid_dependency(...) \
    bz_set_error(BZ_INVALID_DEPENDENCY, __VA_ARGS__)
#define bz_invalid_version(...) \
    bz_set_error(BZ_INVALID_VERSION, __VA_ARGS__)
#define bz_subprocess_error(...) \
    bz_set_error(BZ_SUBPROCESS_ERROR, __VA_ARGS__)


#endif /* BUZZY_ERROR_H */
