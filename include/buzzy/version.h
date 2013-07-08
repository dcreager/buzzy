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

/* Invariants: If kind == FINAL, then string_value must be empty; otherwise,
 * string_value must be non-empty.  If string_value contains an integer, then
 * int_value must contain that same integer; otherwise, int_value must contain
 * BZ_VERSION_PART_USE_STRING. */
struct bz_version_part {
    enum bz_version_part_kind  kind;
    struct cork_buffer  string_value;
    unsigned int  int_value;
};

/* Invariants: There must be at least one part, and the first part must be a
 * RELEASE part. */
struct bz_version {
    cork_array(struct bz_version_part)  parts;
    cork_array(struct bz_version_part *)  compare_parts;
    struct cork_buffer  string;
    struct cork_buffer  compare_string;
};

#define bz_version_part_count(version) \
    (cork_array_size(&(version)->parts))

#define bz_version_get_part(version, index) \
    (&cork_array_at(&(version)->parts, (index)))

#define bz_version_compare_part_count(version) \
    (cork_array_size(&(version)->compare_parts))

#define bz_version_get_compare_part(version, index) \
    (cork_array_at(&(version)->compare_parts, (index)))


struct bz_version *
bz_version_copy(struct bz_version *version);

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


/* The following functions should only be used when you're parsing a Buzzy
 * version from a string.  Most of the time you should just construct a version
 * instance using bz_version_from_string. */

struct bz_version *
bz_version_new(void);

int
bz_version_add_part(struct bz_version *version,
                    enum bz_version_part_kind kind,
                    const char *string_value, size_t size);

void
bz_version_finalize(struct bz_version *version);


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
