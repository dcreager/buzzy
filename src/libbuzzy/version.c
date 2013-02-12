/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/version.h"


#if !defined(BZ_DEBUG_VERSIONS)
#define BZ_DEBUG_VERSIONS  0
#endif

#if BZ_DEBUG_VERSIONS
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * Version parts
 */

const char *
bz_version_part_kind_name(enum bz_version_part_kind kind)
{
    switch (kind) {
        case BZ_VERSION_PRERELEASE:
            return "prerelease";

        case BZ_VERSION_FINAL:
            return "final";

        case BZ_VERSION_POSTRELEASE:
            return "postrelease";

        case BZ_VERSION_RELEASE:
            return "release";

        default:
            cork_unreachable();
    }
}

static void
bz_version_part_init(struct bz_version_part *part,
                     enum bz_version_part_kind kind)
{
    part->kind = kind;
    part->int_value = 0;
    cork_buffer_init(&part->string_value);
}

static void
bz_version_part_done(struct bz_version_part *part)
{
    cork_buffer_done(&part->string_value);
}

static int
bz_version_part_set_string_value(struct bz_version_part *part,
                                 const char *version, const char *part_start,
                                 const char *part_end)
{
    if (CORK_UNLIKELY(part_start == part_end)) {
        bz_invalid_version("Invalid version %s", version);
        return -1;
    } else {
        cork_buffer_set(&part->string_value, part_start, part_end - part_start);
        return 0;
    }
}

static void
bz_version_part_to_string(const struct bz_version_part *part,
                          struct cork_buffer *dest)
{
    switch (part->kind) {
        case BZ_VERSION_RELEASE:
            cork_buffer_append(dest, ".", 1);
            break;

        case BZ_VERSION_PRERELEASE:
            cork_buffer_append(dest, "~", 1);
            break;

        case BZ_VERSION_POSTRELEASE:
            cork_buffer_append(dest, "+", 1);
            break;

        case BZ_VERSION_FINAL:
            cork_buffer_append(dest, "]", 1);
            return;

        default:
            cork_unreachable();
    }

    cork_buffer_append(dest, part->string_value.buf, part->string_value.size);
}


/*-----------------------------------------------------------------------
 * Error handling
 */

void
bz_version_free(struct bz_version *version)
{
    size_t  i;
    for (i = 0; i < cork_array_size(&version->parts); i++) {
        struct bz_version_part  *part = &cork_array_at(&version->parts, i);
        bz_version_part_done(part);
    }
    cork_array_done(&version->parts);
    cork_array_done(&version->compare_parts);
    cork_buffer_done(&version->string);
    cork_buffer_done(&version->compare_string);
    free(version);
}

static void
bz_version_set_compare_parts(struct bz_version *version)
{
    size_t  i;
    cork_array(struct bz_version_part *)  temp_parts;

    /* For comparison purposes, we need to ignore any sequence of "0" release
     * parts that aren't immediately followed by a non-"0" release part. */

#define add_compare_part(p) \
    do { \
        cork_array_append(&version->compare_parts, p); \
        bz_version_part_to_string(p, &version->compare_string); \
    } while (0)

    cork_array_init(&temp_parts);
    cork_buffer_append(&version->compare_string, "[", 1);
    for (i = 0; i < cork_array_size(&version->parts); i++) {
        struct bz_version_part  *part = &cork_array_at(&version->parts, i);
        if (part->kind == BZ_VERSION_RELEASE) {
            if (part->int_value == 0) {
                /* This is a "0" release part.  It may or may not end up in the
                 * comparison list, depending on what appears later.  Queue it
                 * up for now. */
                cork_array_append(&temp_parts, part);
            } else {
                /* We found a non-"0" release part.  If there were any "0"
                 * release parts queued up, we can now add them to the
                 * comparison list. */
                size_t  j;
                for (j = 0; j < cork_array_size(&temp_parts); j++) {
                    add_compare_part(cork_array_at(&temp_parts, j));
                }
                temp_parts.size = 0;
                add_compare_part(part);
            }
        } else {
            /* We found a prerelease or postrelease part.  Any queud up "0"
             * release parts should be ignored. */
            temp_parts.size = 0;
            add_compare_part(part);
        }
    }

    cork_array_done(&temp_parts);
}

struct bz_version *
bz_version_new_from_string(const char *string)
{
    struct bz_version  *version;
    struct bz_version_part  *part;
    const char  *part_start = string;
    const char  *ch;
    bool  int_value;

    /* Parse the contents of the version string. */

    DEBUG("---\nParse version \"%s\"\n", string);

    if (CORK_UNLIKELY(*string == '\0')) {
        bz_invalid_version("Invalid version \"\"");
        return NULL;
    }

#define start_part(kind, start) \
    do { \
        DEBUG("  Create new %s version part\n", \
              bz_version_part_kind_name(kind)); \
        part = cork_array_append_get(&version->parts); \
        bz_version_part_init(part, kind); \
        int_value = true; \
        part_start = start; \
    } while (0)

#define finish_part() \
    do { \
        if (BZ_VERSION_PART_IS_INTEGRAL(part)) { \
            DEBUG("    Integral value: %u\n", part->int_value); \
        } \
        ei_check(bz_version_part_set_string_value \
                 (part, string, part_start, ch)); \
        DEBUG("    String value: %s\n", (char *) part->string_value.buf); \
    } while (0)

    version = cork_new(struct bz_version);
    cork_array_init(&version->parts);
    cork_array_init(&version->compare_parts);
    cork_buffer_init(&version->string);
    cork_buffer_set_string(&version->string, string);
    cork_buffer_init(&version->compare_string);
    start_part(BZ_VERSION_RELEASE, string);

    for (ch = string; *ch != '\0'; ch++) {
        if (*ch == '.') {
            finish_part();
            start_part(BZ_VERSION_RELEASE, ch + 1);
            continue;
        }

        if (*ch == '~') {
            finish_part();
            start_part(BZ_VERSION_PRERELEASE, ch + 1);
            continue;
        }

        if (*ch == '+') {
            finish_part();
            start_part(BZ_VERSION_POSTRELEASE, ch + 1);
            continue;
        }

        if (int_value) {
            if (isdigit(*ch)) {
                part->int_value *= 10;
                part->int_value += (*ch - '0');
            } else {
                int_value = false;
                part->int_value = BZ_VERSION_PART_USE_STRING;
            }
        }
    }

    finish_part();
    start_part(BZ_VERSION_FINAL, ch);

    /* Once we've parsed all of the parts in the version string, determine which
     * of those parts should be used when comparing versions. */
    bz_version_set_compare_parts(version);
    return version;

error:
    bz_version_free(version);
    return NULL;
}

const char *
bz_version_to_string(const struct bz_version *version)
{
    return version->string.buf;
}

const char *
bz_version_to_compare_string(const struct bz_version *version)
{
    return version->compare_string.buf;
}

int
bz_version_cmp(const struct bz_version *v1, const struct bz_version *v2)
{
    size_t  i;
    size_t  v1_count = cork_array_size(&v1->compare_parts);
    size_t  v2_count = cork_array_size(&v2->compare_parts);
    size_t  min_count = (v1_count < v2_count)? v1_count: v2_count;

    DEBUG("---\nComparing versions %s %s and %s %s\n",
          bz_version_to_string(v1), bz_version_to_compare_string(v1),
          bz_version_to_string(v2), bz_version_to_compare_string(v2));

    for (i = 0; i < min_count; i++) {
        const struct bz_version_part  *part1 =
            cork_array_at(&v1->compare_parts, i);
        const struct bz_version_part  *part2 =
            cork_array_at(&v2->compare_parts, i);
        bool  integral1 = BZ_VERSION_PART_IS_INTEGRAL(part1);
        bool  integral2 = BZ_VERSION_PART_IS_INTEGRAL(part2);

        if (part1->kind < part2->kind) {
            DEBUG("  %s < %s\n",
                  bz_version_part_kind_name(part1->kind),
                  bz_version_part_kind_name(part2->kind));
            return -1;
        } else if (part1->kind > part2->kind) {
            DEBUG("  %s < %s\n",
                  bz_version_part_kind_name(part1->kind),
                  bz_version_part_kind_name(part2->kind));
            return 1;
        } else {
            DEBUG("  %s == %s\n",
                  bz_version_part_kind_name(part1->kind),
                  bz_version_part_kind_name(part2->kind));
        }

        if (integral1 != integral2) {
            bz_invalid_version("Cannot compare %s to %s",
                               bz_version_to_string(v1),
                               bz_version_to_string(v2));
            return -1;
        }

        if (integral1) {
            if (part1->int_value < part2->int_value) {
                DEBUG("  %u < %u\n", part1->int_value, part2->int_value);
                return -1;
            } else if (part1->int_value > part2->int_value) {
                DEBUG("  %u > %u\n", part1->int_value, part2->int_value);
                return 1;
            } else {
                DEBUG("  %u == %u\n", part1->int_value, part2->int_value);
            }
        } else {
            int  cmp = strcmp(part1->string_value.buf, part2->string_value.buf);
            if (cmp == 0) {
                DEBUG("  %s == %s\n",
                      (char *) part1->string_value.buf,
                      (char *) part2->string_value.buf);
            } else if (cmp < 0) {
                DEBUG("  %s < %s\n",
                      (char *) part1->string_value.buf,
                      (char *) part2->string_value.buf);
                return -1;
            } else {
                DEBUG("  %s > %s\n",
                      (char *) part1->string_value.buf,
                      (char *) part2->string_value.buf);
                return 1;
            }
        }
    }

    if (v1_count < v2_count) {
        DEBUG("  %s has fewer parts\n", bz_version_to_string(v1));
        return -1;
    } else if (v1_count > v2_count) {
        DEBUG("  %s has more parts\n", bz_version_to_string(v1));
        return 1;
    } else {
        DEBUG("  Equal!\n");
        return 0;
    }
}
