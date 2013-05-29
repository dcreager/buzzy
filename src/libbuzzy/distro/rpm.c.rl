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
#include <sys/stat.h>

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/native.h"
#include "buzzy/os.h"
#include "buzzy/version.h"
#include "buzzy/distro/rpm.h"

#define CLOG_CHANNEL  "rpm"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_redhat_is_present(bool *dest)
{
    int  rc;
    struct stat  info;
    rc = stat("/etc/redhat-release", &info);
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
    bool  need_punct_before_digit = false;
    struct bz_version_part  *part;

    count = bz_version_part_count(version);
    assert(count > 0);

    for (i = 0; i < count; i++) {
        const char  *string_value;
        const char  *last_char;
        part = bz_version_get_part(version, i);
        string_value = part->string_value.buf;

        switch (part->kind) {
            case BZ_VERSION_RELEASE:
                if (need_punct_before_digit ||
                    !BZ_VERSION_PART_IS_INTEGRAL(part)) {
                    if (seen_non_release_tag) {
                        cork_buffer_append(dest, ".1.", 3);
                    } else {
                        cork_buffer_append(dest, ".", 1);
                    }
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
                }
                return;

            default:
                break;
        }

        cork_buffer_append_copy(dest, &part->string_value);

        assert(part->string_value.size > 0);
        last_char = strchr(string_value, '\0') - 1;
        need_punct_before_digit = isdigit(*last_char);
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

        no_dot_release = digit+
                       >start_release %add_part;

        release = ('.' | '_' | '+')* part
                  >start_release %add_part;

        rpm_v = no_dot_release release**;


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


/*-----------------------------------------------------------------------
 * Native package database
 */

struct bz_version *
bz_yum_native_version_available(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *v_start = NULL;
    char  *v_end = NULL;
    char  *r_start = NULL;
    char  *r_end = NULL;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct cork_buffer  buf;
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "yum", "info", native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine rpm_version_available;

        version_char = alnum | digit | '.' | '_';

        version = (version_char)+ >{ v_start = fpc; } %{ v_end = fpc; };
        version_line = "Version" space* ':' space+ version '\n';

        release = (version_char)+ >{ r_start = fpc; } %{ r_end = fpc; };
        release_line = "Release" space* ':' space+ release '\n';

        other_line = (any - '\n')* '\n';

        line = version_line | release_line | other_line;

        main := line*;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) rpm_version_available_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from yum");
        cork_buffer_done(&out);
        return NULL;
    }

    if (v_start == NULL || v_end == NULL || r_start == NULL || r_end == NULL) {
        bz_invalid_version("Unexpected output from yum");
        cork_buffer_done(&out);
        return NULL;
    }

    cork_buffer_init(&buf);
    cork_buffer_append(&buf, v_start, v_end - v_start);
    cork_buffer_append(&buf, "-", 1);
    cork_buffer_append(&buf, r_start, r_end - r_start);
    result = bz_version_from_rpm(buf.buf);
    cork_buffer_done(&out);
    cork_buffer_done(&buf);
    return result;
}

struct bz_version *
bz_rpm_native_version_installed(const char *native_package_name)
{
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "rpm", "--qf", "%{V}-%{R}", "-q", native_package_name, NULL));

    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    result = bz_version_from_rpm(out.buf);
    cork_buffer_done(&out);
    return result;
}


static int
bz_yum_native__install(const char *native_package_name,
                       struct bz_version *version)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "yum", "install", "-y", native_package_name,
         NULL);
}

static int
bz_yum_native__uninstall(const char *native_package_name)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "yum", "remove", "-y", native_package_name,
         NULL);
}

struct bz_pdb *
bz_yum_native_pdb(void)
{
    return bz_native_pdb_new
        ("RPM", "rpm",
         bz_yum_native_version_available,
         bz_rpm_native_version_installed,
         bz_yum_native__install,
         bz_yum_native__uninstall,
         "%s-devel", "lib%s-devel", "%s", "lib%s", NULL);
}
