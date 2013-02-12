/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_VERSION_H
#define BUZZY_VERSION_H

#include <libcork/core.h>
#include <libcork/ds.h>


enum bz_version_part_kind {
    BZ_VERSION_PRERELEASE  = 0,
    BZ_VERSION_FINAL       = 1,
    BZ_VERSION_POSTRELEASE = 2,
    BZ_VERSION_RELEASE     = 3
};

const char *
bz_version_part_kind_name(enum bz_version_part_kind kind);

#define BZ_VERSION_PART_USE_STRING  ((unsigned int) UINT_MAX)
#define BZ_VERSION_PART_IS_INTEGRAL(part) \
    ((part)->int_value != BZ_VERSION_PART_USE_STRING)

struct bz_version_part {
    enum bz_version_part_kind  kind;
    struct cork_buffer  string_value;
    unsigned int  int_value;
};

struct bz_version {
    cork_array(struct bz_version_part)  parts;
    cork_array(struct bz_version_part *)  compare_parts;
    struct cork_buffer  string;
    struct cork_buffer  compare_string;
};


struct bz_version *
bz_version_new_from_string(const char *string);

void
bz_version_free(struct bz_version *version);

const char *
bz_version_to_string(const struct bz_version *version);

const char *
bz_version_to_compare_string(const struct bz_version *version);

int
bz_version_cmp(const struct bz_version *v1, const struct bz_version *v2);


#endif /* BUZZY_VERSION_H */
