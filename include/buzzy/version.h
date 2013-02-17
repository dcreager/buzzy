/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_VERSION_H
#define BUZZY_VERSION_H

#include <libcork/core.h>
#include <libcork/ds.h>


/*-----------------------------------------------------------------------
 * Version parts
 */

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
bz_version_from_string(const char *string);

void
bz_version_free(struct bz_version *version);

const char *
bz_version_to_string(const struct bz_version *version);

const char *
bz_version_to_compare_string(const struct bz_version *version);

int
bz_version_cmp(const struct bz_version *v1, const struct bz_version *v2);


/*-----------------------------------------------------------------------
 * Dependencies
 */

struct bz_dependency {
    const char  *package_name;
    /* NULL if any version works */
    struct bz_version  *min_version;
    struct cork_buffer  string;
};

/* Takes control of min_version */
struct bz_dependency *
bz_dependency_new(const char *package_name, struct bz_version *min_version);

struct bz_dependency *
bz_dependency_from_string(const char *string);

void
bz_dependency_free(struct bz_dependency *dep);

const char *
bz_dependency_to_string(const struct bz_dependency *dep);


#endif /* BUZZY_VERSION_H */