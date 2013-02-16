/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/version.h"
#include "buzzy/distro/arch.h"


#if !defined(BZ_DEBUG_ARCH)
#define BZ_DEBUG_ARCH  0
#endif

#if BZ_DEBUG_ARCH
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_arch_is_present(bool *dest)
{
    int  rc;
    struct stat  info;
    rc = stat("/etc/arch-release", &info);
    if (rc == 0) {
        *dest = true;
        return 0;
    } else if (errno == ENOENT) {
        *dest = false;
        return 0;
    } else {
        cork_system_error_set();
        return -1;
    }
}


/*-----------------------------------------------------------------------
 * Arch version strings
 */

#define cork_buffer_append_copy(dest, src) \
    (cork_buffer_append((dest), (src)->buf, (src)->size))

void
bz_version_to_arch(struct bz_version *version, struct cork_buffer *dest)
{
    size_t  i;
    size_t  count;
    struct bz_version_part  *part;

    count = bz_version_part_count(version);
    assert(count > 0);

    part = bz_version_get_part(version, 0);
    assert(part->kind == BZ_VERSION_RELEASE);
    cork_buffer_append_copy(dest, &part->string_value);

    for (i = 1; i < count; i++) {
        const char  *string_value;
        part = bz_version_get_part(version, i);
        string_value = part->string_value.buf;
        switch (part->kind) {
            case BZ_VERSION_RELEASE:
                cork_buffer_append(dest, ".", 1);
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_PRERELEASE:
                /* Arch considers a tag to be a prerelease tag if it starts with
                 * an alphanumeric, and is not separated from the preceding
                 * numeric release tag. */
                assert(part->string_value.size > 0);
                if (!isalpha(*string_value)) {
                    cork_buffer_append(dest, "pre", 3);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_POSTRELEASE:
                /* Arch considers a tag to be a postrelease tag if it starts
                 * with an alphanumeric, and has a "." separating it from the
                 * preceding numeric release tag. */
                assert(part->string_value.size > 0);
                if (isalpha(*string_value)) {
                    cork_buffer_append(dest, ".", 1);
                } else {
                    cork_buffer_append(dest, ".post", 5);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            default:
                break;
        }
    }
}


struct bz_version *
bz_version_from_arch(const char *arch_version)
{
    int  cs;
    const char  *p = arch_version;
    const char  *pe = strchr(arch_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    enum bz_version_part_kind  kind;
    const char  *start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    DEBUG("---\nParse Arch version \"%s\"\n", arch_version);
    version = bz_version_new();

    %%{
        machine arch_version;

        action start_release {
            kind = BZ_VERSION_RELEASE;
            start = fpc;
            DEBUG("  Create new release version part\n");
        }

        action start_prerelease {
            kind = BZ_VERSION_PRERELEASE;
            start = fpc;
            DEBUG("  Create new prerelease version part\n");
        }

        action start_postrelease {
            kind = BZ_VERSION_POSTRELEASE;
            start = fpc;
            DEBUG("  Create new postrelease version part\n");
        }

        action add_part {
            size_t  size = fpc - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }

        action start_revision {
            start = fpc + 1;
        }

        action add_revision {
            cork_buffer_set(&buf, "rev", 3);
            cork_buffer_append(&buf, start, fpc - start);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }

        first_release = digit+
                      >start_release %add_part;

        release = '.' digit+
                  >start_release %add_part;

        pre_prerelease = "pre" (alnum+ >start_prerelease)
                       %add_part;

        other_prerelease = ((alpha alnum+) - pre_prerelease)
                         >start_prerelease %add_part;

        prerelease = pre_prerelease | other_prerelease;

        post_postrelease = ".post" (alnum+ >start_postrelease)
                         %add_part;

        other_postrelease = ('.' alpha >start_postrelease alnum+)
                          %add_part;

        postrelease = post_postrelease | other_postrelease;

        skip_rev_1 = "-1";

        rev = (('-' (digit+)) - skip_rev_1)
              >start_revision %add_revision;

        part = release | prerelease | postrelease;

        main := first_release part** rev?;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) arch_version_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Invalid Arch version \"%s\"", arch_version);
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    return version;
}
