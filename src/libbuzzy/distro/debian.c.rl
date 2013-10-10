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
#include "buzzy/distro/debian.h"

#define CLOG_CHANNEL  "debian"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_debian_is_present(bool *dest)
{
    int  rc;
    struct stat  info;
    rc = stat("/etc/debian_version", &info);
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
 * Debian version strings
 */

void
bz_version_to_deb(struct bz_version *version, struct cork_buffer *dest)
{
    size_t  i;
    size_t  count;
    struct bz_version_part  *part;
    bool  have_release = false;
    bool  need_punct_before_digit = false;

    count = bz_version_part_count(version);
    assert(count > 0);

    for (i = 0; i < count; i++) {
        part = bz_version_get_part(version, i);
        switch (part->kind) {
            case BZ_VERSION_RELEASE:
                if (need_punct_before_digit ||
                    !BZ_VERSION_PART_IS_INTEGRAL(part)) {
                    cork_buffer_append(dest, ".", 1);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                need_punct_before_digit = true;
                break;

            case BZ_VERSION_PRERELEASE:
                assert(part->string_value.size > 0);
                cork_buffer_append(dest, "~", 1);
                cork_buffer_append_copy(dest, &part->string_value);
                need_punct_before_digit = true;
                break;

            case BZ_VERSION_POSTRELEASE:
                assert(part->string_value.size > 0);
                if (!have_release &&
                    strcmp(part->string_value.buf, "rev") == 0) {
                    have_release = true;
                    need_punct_before_digit = false;
                    cork_buffer_append(dest, "-", 1);
                } else {
                    cork_buffer_append(dest, "+", 1);
                    cork_buffer_append_copy(dest, &part->string_value);
                    need_punct_before_digit = true;
                }
                break;

            case BZ_VERSION_FINAL:
                return;

            default:
                break;
        }

        assert(part->string_value.size > 0);
    }
}


static struct bz_version *
bz_version_from_deb_ours(const char *debian_version)
{
    int  cs;
    const char  *p = debian_version;
    const char  *pe = strchr(debian_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    enum bz_version_part_kind  kind;
    const char  *start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    clog_trace("Parse our Debian version \"%s\"", debian_version);
    version = bz_version_new();

    %%{
        machine debian_version_ours;

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

        no_dot_release = digit+ >start_release %add_part;
        release = '.' alnum+ >start_release %add_part;
        prerelease = '~' alnum+ >start_prerelease %add_part;
        postrelease = '+' alnum+ >start_postrelease %add_part;
        rev = "-1";
        part = release | prerelease | postrelease;
        main := no_dot_release part** rev?;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) debian_version_ours_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    cork_buffer_done(&buf);
    return version;
}

static struct bz_version *
bz_version_from_deb_any(const char *debian_version)
{
    int  cs;
    const char  *p = debian_version;
    const char  *pe = strchr(debian_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    enum bz_version_part_kind  kind;
    const char  *start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    clog_trace("Parse any Debian version \"%s\"", debian_version);
    version = bz_version_new();

    %%{
        machine debian_version_any;

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
            ei_check(bz_version_add_part(version, kind, start, size));
        }

        action add_revision {
            ei_check(bz_version_add_part
                     (version, BZ_VERSION_POSTRELEASE, "rev", 3));
        }

        no_dot_release =     digit+ >start_release %add_part;
        dot_release    = '.' digit+ >start_release %add_part;

        alpha_prerelease = '~' alpha+ >start_prerelease %add_part;
        num_prerelease   = '~' digit+ >start_prerelease %add_part;
        prerelease       = alpha_prerelease | num_prerelease;

        alpha_postrelease = '+' alpha+ >start_postrelease %add_part;
        num_postrelease   = '+' digit+ >start_postrelease %add_part;
        postrelease       = alpha_postrelease | num_postrelease;

        part = dot_release | prerelease | postrelease;
        section = no_dot_release part**;

        rev = '-' %add_revision;

        main := section (rev section)?;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) debian_version_any_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        goto error;
    }

    bz_version_finalize(version);
    cork_buffer_done(&buf);
    return version;

error:
    cork_buffer_done(&buf);
    bz_version_free(version);
    return NULL;
}

struct bz_version *
bz_version_from_deb(const char *deb)
{
    struct bz_version  *version = bz_version_from_deb_ours(deb);
    if (version == NULL) {
        version = bz_version_from_deb_any(deb);
    }
    if (version == NULL) {
        bz_invalid_version("Invalid Debian version \"%s\"", deb);
    }
    return version;
}


/*-----------------------------------------------------------------------
 * Native package database
 */

struct bz_version *
bz_apt_native_version_available(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *start = NULL;
    char  *end = NULL;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "apt-cache", "show", "--no-all-versions",
               native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine debian_version_available;

        version_char = alnum | digit | '.' | '-' | '+' | '~';
        version = (version_char)+ >{ start = fpc; } %{ end = fpc; };
        version_line = "Version:" space+ version '\n';

        other_line = (any - '\n')* '\n';

        line = version_line | other_line;

        main := line*;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) debian_version_available_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from apt-cache");
        cork_buffer_done(&out);
        return NULL;
    }

    if (start == NULL || end == NULL) {
        bz_invalid_version("Unexpected output from apt-cache");
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_deb(start);
    cork_buffer_done(&out);
    return result;
}

struct bz_version *
bz_deb_native_version_installed(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *eof;
    char  *start = NULL;
    char  *end = NULL;
    bool  successful;
    bool  installed = false;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "dpkg-query", "-W", "-f", "${Status}\\n${Version}",
               native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;
    eof = pe;

    %%{
        machine debian_version_installed;

        installed = "installed" %{ installed = true; };
        status_line = (any - "\n")* installed? (any - "\n")*;
        version_char = alnum | digit | '.' | '-' | '+' | '~';
        version = (version_char)+ >{ start = fpc; } %{ end = fpc; };
        main := status_line "\n" version;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) debian_version_installed_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from dpkg-query");
        cork_buffer_done(&out);
        return NULL;
    }

    if (start == NULL || end == NULL) {
        bz_invalid_version("Unexpected output from dpkg-query");
        cork_buffer_done(&out);
        return NULL;
    }

    if (!installed) {
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_deb(start);
    cork_buffer_done(&out);
    return result;
}


static int
bz_apt_native__install(const char *native_package_name,
                       struct bz_version *version)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "apt-get", "install", "-y", native_package_name,
         NULL);
}

static int
bz_apt_native__uninstall(const char *native_package_name)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "apt-get", "remove", "-y", native_package_name,
         NULL);
}

struct bz_pdb *
bz_apt_native_pdb(void)
{
    return bz_native_pdb_new
        ("Debian", "debian",
         bz_apt_native_version_available,
         bz_deb_native_version_installed,
         bz_apt_native__install,
         bz_apt_native__uninstall,
         "%s-dev", "lib%s-dev", "%s", "lib%s", NULL);
}
