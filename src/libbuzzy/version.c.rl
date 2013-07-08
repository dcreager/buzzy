/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
bz_version_part_done(struct bz_version_part *part)
{
    cork_buffer_done(&part->string_value);
}

static void
bz_version_part_to_string(const struct bz_version_part *part,
                          struct cork_buffer *dest, bool hide_punct)
{
    if (!hide_punct) {
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
    }

    cork_buffer_append(dest, part->string_value.buf, part->string_value.size);
}


/*-----------------------------------------------------------------------
 * Versions
 */

struct bz_version *
bz_version_new(void)
{
    struct bz_version  *version = cork_new(struct bz_version);
    cork_array_init(&version->parts);
    cork_array_init(&version->compare_parts);
    cork_buffer_init(&version->string);
    cork_buffer_init(&version->compare_string);
    return version;
}

struct bz_version *
bz_version_copy(struct bz_version *other)
{
    return bz_version_from_string(bz_version_to_string(other));
}

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

#define is_digit(ch)  ((ch) >= '0' && (ch) <= '9')

int
bz_version_add_part(struct bz_version *version,
                    enum bz_version_part_kind kind,
                    const char *string_value, size_t size)
{
    size_t  i;
    struct bz_version_part  *part;
    bool  is_numeric;
    bool  hide_punct;

    assert(string_value != NULL && *string_value != '\0');

    is_numeric = is_digit(string_value[0]);
    for (i = 1; i < size; i++) {
        if (is_digit(string_value[i]) != is_numeric) {
            bz_invalid_version
                ("Version part %.*s must be all numeric or all alpha",
                 (int) size, string_value);
            return -1;
        }
    }

    part = cork_array_append_get(&version->parts);
    part->kind = kind;
    cork_buffer_init(&part->string_value);
    cork_buffer_set(&part->string_value, string_value, size);

    if (is_numeric) {
        /* We've already verified that there are only digits, so the strtoul
         * call must always succeed. */
        part->int_value = strtoul(part->string_value.buf, NULL, 10);
    } else {
        part->int_value = BZ_VERSION_PART_USE_STRING;
    }

    hide_punct = (version->string.size == 0);
    bz_version_part_to_string(part, &version->string, hide_punct);
    return 0;
}

static void
bz_version_add_final_part(struct bz_version *version)
{
    struct bz_version_part  *part;
    part = cork_array_append_get(&version->parts);
    part->kind = BZ_VERSION_FINAL;
    part->int_value = BZ_VERSION_PART_USE_STRING;
    cork_buffer_init(&part->string_value);
    cork_buffer_set(&part->string_value, "", 0);
}

void
bz_version_finalize(struct bz_version *version)
{
    size_t  i;
    cork_array(struct bz_version_part *)  temp_parts;

    /* For comparison purposes, we need to ignore any sequence of "0" release
     * parts that aren't immediately followed by a non-"0" release part. */

#define add_compare_part(p) \
    do { \
        cork_array_append(&version->compare_parts, p); \
        bz_version_part_to_string(p, &version->compare_string, false); \
        DEBUG("    %s\n", (char *) version->compare_string.buf); \
    } while (0)

    DEBUG("  Finding comparison parts\n");

    bz_version_add_final_part(version);
    cork_array_init(&temp_parts);
    cork_buffer_append(&version->compare_string, "[", 1);
    for (i = 0; i < bz_version_part_count(version); i++) {
        struct bz_version_part  *part = bz_version_get_part(version, i);
        DEBUG("    %s part %s\n", bz_version_part_kind_name(part->kind),
              (char *) part->string_value.buf);
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
            /* We found a prerelease or postrelease part.  Any queued up "0"
             * release parts should be ignored. */
            temp_parts.size = 0;
            add_compare_part(part);
        }
    }

    cork_array_done(&temp_parts);
}

struct bz_version *
bz_version_from_string(const char *string)
{
    int  cs;
    const char  *p = string;
    const char  *pe = strchr(string, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    enum bz_version_part_kind  kind;
    const char  *part_start;

    /* Parse the contents of the version string. */
    DEBUG("---\nParse version \"%s\"\n", string);
    version = bz_version_new();

    %%{
        machine buzzy_version;

        action initialize_part {
            part_start = fpc;
        }

        action release_part {
            kind = BZ_VERSION_RELEASE;
            DEBUG("  Create new release version part\n");
        }

        action prerelease_part {
            kind = BZ_VERSION_PRERELEASE;
            DEBUG("  Create new prerelease version part\n");
        }

        action postrelease_part {
            kind = BZ_VERSION_POSTRELEASE;
            DEBUG("  Create new prerelease version part\n");
        }

        action add_part {
            size_t  size = fpc - part_start;
            DEBUG("    String value: %.*s\n", (int) size, part_start);
            ei_check(bz_version_add_part(version, kind, part_start, size));
        }

        part             = alnum+
                           >initialize_part %add_part;
        no_dot_part      = part >release_part;
        release_part     = '.' %release_part part;
        prerelease_part  = '~' %prerelease_part part;
        postrelease_part = '+' %postrelease_part part;
        rest_part        = (no_dot_part | release_part |
                            prerelease_part | postrelease_part);

        main := no_dot_part rest_part**;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) buzzy_version_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Invalid version \"%s\"", string);
        goto error;
    }

    bz_version_finalize(version);
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
