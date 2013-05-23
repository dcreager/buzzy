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

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/version.h"
#include "buzzy/distro/rpm.h"

#define CLOG_CHANNEL  "rpm"


/*-----------------------------------------------------------------------
 * RPM version strings
 */

/*
 * References:
 * https://fedoraproject.org/wiki/Packaging:NamingGuidelines#Package_Versioning
 * http://fedoraproject.org/wiki/Archive:Tools/RPM/VersionComparison
 *
 * RPM's version comparison algorithm doesn't handle prerelease and postrelease
 * tags directly in the version number very well.  RedHat's package guidelines
 * tell us to encode prereleases and postreleases in the Release portion of a
 * package version, rather than the Version portion.  So, this function has to
 * return a Version and a Release.
 *
 * We put any leading release tags into the Version.  These can be compared
 * using RPM's algorithm without many problems.  (One outstanding issue is for
 * Buzzy, 1.0 == 1.0.0, whereas for RPM, 1.0 < 1.0.0.  We're punting on that for
 * now.)
 *
 * Once we see the first prerelease or postrelease tag, we stop adding the tags
 * to the Version, and start adding them to the Release.  To make sure that
 * preleases, releases, and postreleases all sort correctly, we add two tags to
 * the Release for each Buzzy tag:
 *
 *   ~foo => 0.foo
 *   foo  => 1.foo
 *   +foo => 2.foo
 *
 * The FINAL tag that ends each Buzzy version number also gets added to the
 * Release, as a trailing .1.
 *
 * All of this can produce a fairly ugly RPM version when there are a lot of
 * prerelease and postrelease tags, but it should guarantee that RPM and Buzzy
 * compare two versions in the same way.
 */

void
bz_version_to_rpm(struct bz_version *version, struct cork_buffer *dest)
{
    size_t  i;
    size_t  count;
    bool  seen_non_release_tag = false;
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
                if (seen_non_release_tag) {
                    cork_buffer_append(dest, ".1.", 3);
                } else {
                    cork_buffer_append(dest, ".", 1);
                }
                break;

            case BZ_VERSION_PRERELEASE:
                if (seen_non_release_tag) {
                    cork_buffer_append(dest, ".0.", 3);
                } else {
                    cork_buffer_append(dest, "-0.", 3);
                    seen_non_release_tag = true;
                }
                assert(part->string_value.size > 0);
                if (!isalpha(*string_value)) {
                    cork_buffer_append(dest, "pre", 3);
                }
                break;

            case BZ_VERSION_POSTRELEASE:
                if (seen_non_release_tag) {
                    cork_buffer_append(dest, ".2.", 3);
                } else {
                    cork_buffer_append(dest, "-2.", 3);
                    seen_non_release_tag = true;
                }
                assert(part->string_value.size > 0);
                if (!isalpha(*string_value)) {
                    cork_buffer_append(dest, "post", 4);
                }
                break;

            case BZ_VERSION_FINAL:
                if (seen_non_release_tag) {
                    cork_buffer_append(dest, ".1", 2);
                } else {
                    cork_buffer_append(dest, "-1", 2);
                    seen_non_release_tag = true;
                }
                break;

            default:
                break;
        }

        cork_buffer_append_copy(dest, &part->string_value);
    }
}


struct bz_version *
bz_version_from_rpm(const char *rpm_version)
{
    int  cs;
    const char  *p = rpm_version;
    const char  *pe = strchr(rpm_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    enum bz_version_part_kind  kind;
    const char  *start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    clog_trace("Parse RPM version \"%s\"", rpm_version);
    version = bz_version_new();

    %%{
        machine rpm_version;

        action start_release {
            kind = BZ_VERSION_RELEASE;
            start = fpc;
            clog_trace("  Create new release version part");
        }

        action start_prerelease {
            kind = BZ_VERSION_PRERELEASE;
            start = fpc;
            clog_trace("  Create new prerelease version part");
        }

        action start_postrelease {
            kind = BZ_VERSION_POSTRELEASE;
            start = fpc;
            clog_trace("  Create new postrelease version part");
        }

        action add_part {
            size_t  size = fpc - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }

        action start_revision {
            start = fpc + 1;
            clog_trace("  Create new postrelease version part for revision");
        }

        action add_revision {
            cork_buffer_set(&buf, "rev", 3);
            cork_buffer_append(&buf, start, fpc - start);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }

        digit_part = digit+;
        alpha_part = alpha+;
        part = digit_part | alpha_part;

        first_release = digit+
                      >start_release %add_part;

        release = ('.' | '_' | '+')* part
                  >start_release %add_part;

        rpm_v = first_release release**;


        pre_first_part = '0.' (digit+ >start_prerelease)
                       %add_part;

        post_first_part = ('1'..'9' >start_postrelease) digit*
                        %add_part;

        first_r = pre_first_part | post_first_part;

        rel_part = (first_r release**) - "1";
        rpm_r = rel_part | "1";

        main := rpm_v '-' rpm_r;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) rpm_version_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Invalid RPM version \"%s\"", rpm_version);
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    cork_buffer_done(&buf);
    return version;
}
