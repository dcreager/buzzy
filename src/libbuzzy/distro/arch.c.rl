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

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/native.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/arch.h"

#define CLOG_CHANNEL  "arch"


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

static void
handle_rev_tag(struct bz_version_part *part, struct cork_buffer *dest)
{
    /* We already know this is a postrelease part, and that it's at the end of
     * the Arch version.  Check whether it consists only of digits. */
    if (BZ_VERSION_PART_IS_INTEGRAL(part)) {
        /* Is it rev1?  We ignore that. */
        if (part->int_value == 1) {
            return;
        }
        cork_buffer_append(dest, "-", 1);
    } else {
        /* If not, then we have to add back in the `+rev` that we skipped over
         * in the previous iteration. */
        cork_buffer_append(dest, ".rev.", 5);
    }
    cork_buffer_append_copy(dest, &part->string_value);
}

void
bz_version_to_arch(struct bz_version *version, struct cork_buffer *dest)
{
    size_t  i;
    size_t  count;
    struct bz_version_part  *part;
    bool  have_rev = false;
    bool  need_punct_before_digit = false;

    count = bz_version_part_count(version);
    assert(count > 0);

    for (i = 0; i < count; i++) {
        const char  *string_value;
        const char  *last_char;
        part = bz_version_get_part(version, i);
        string_value = part->string_value.buf;
        switch (part->kind) {
            case BZ_VERSION_RELEASE:
                if (have_rev) {
                    handle_rev_tag(part, dest);
                } else {
                    if (need_punct_before_digit ||
                        !BZ_VERSION_PART_IS_INTEGRAL(part)) {
                        cork_buffer_append(dest, ".", 1);
                    }
                    cork_buffer_append_copy(dest, &part->string_value);
                }
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

                /* Treat a +revXX tag specially, if it occurs at the end of the
                 * Buzzy version.  That kind of tag should become the Arch
                 * version release tag. */
                if (i == count-3 &&
                    strcmp(part->string_value.buf, "rev") == 0) {
                    have_rev = true;
                    continue;
                }

                if (isalpha(*string_value)) {
                    cork_buffer_append(dest, ".", 1);
                } else {
                    cork_buffer_append(dest, ".post", 5);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_FINAL:
                return;

            default:
                break;
        }

        assert(part->string_value.size > 0);
        last_char = strchr(string_value, '\0') - 1;
        need_punct_before_digit = isdigit(*last_char);
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

    clog_trace("Parse Arch version \"%s\"", arch_version);
    version = bz_version_new();

    %%{
        machine arch_version;

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
            bz_version_add_part(version, BZ_VERSION_POSTRELEASE, "rev", 3);
            cork_buffer_set(&buf, start, fpc - start);
            bz_version_add_part(version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }

        no_dot_release = digit+
                       >start_release %add_part;

        dot_release = '.' digit+
                    >start_release %add_part;

        release = no_dot_release | dot_release;

        pre_prerelease = "pre" (digit >start_prerelease digit*) %add_part;

        other_prerelease = ((alpha+) - pre_prerelease)
                         >start_prerelease %add_part;

        prerelease = pre_prerelease | other_prerelease;

        post_postrelease = ".post" (digit >start_postrelease digit*) %add_part;

        other_postrelease = ('.' alpha >start_postrelease alpha*)
                          %add_part;

        postrelease = post_postrelease | other_postrelease;

        skip_rev_1 = "-1";

        real_rev = (('-' (digit+)) - skip_rev_1)
                 >start_revision %add_revision;

        rev = skip_rev_1 | real_rev;

        part = release | prerelease | postrelease;

        main := no_dot_release part** rev?;

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
    cork_buffer_done(&buf);
    return version;
}


/*-----------------------------------------------------------------------
 * Native package database
 */

struct bz_version *
bz_arch_native_version_available(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *start;
    char  *end;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "pacman", "-Sddp", "--print-format", "%v",
               native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine arch_version_available;

        version = (alnum | '.' | '-')+;
        main := (version >{ start = fpc; } %{ end = fpc; }) '\n';

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) arch_version_available_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from pacman");
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_arch(start);
    cork_buffer_done(&out);
    return result;
}

struct bz_version *
bz_arch_native_version_installed(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *start;
    char  *end;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "pacman", "-Q", native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine arch_version_installed;

        package_name = (alnum | '-')+;
        version = (alnum | '.' | '-')+;
        main := package_name ' '
                (version >{ start = fpc; } %{ end = fpc; })
                '\n';

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) arch_version_installed_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from pacman");
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_arch(start);
    cork_buffer_done(&out);
    return result;
}


static int
bz_arch_native__install(const char *native_package_name,
                       struct bz_version *version)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "pacman", "-S", "--noconfirm", native_package_name,
         NULL);
}

static int
bz_arch_native__uninstall(const char *native_package_name)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "pacman", "-R", "--noconfirm", native_package_name,
         NULL);
}

struct bz_pdb *
bz_arch_native_pdb(void)
{
    return bz_native_pdb_new
        ("Arch", "arch",
         bz_arch_native_version_available,
         bz_arch_native_version_installed,
         bz_arch_native__install,
         bz_arch_native__uninstall,
         "%s", "lib%s", NULL);
}
