/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/native.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/homebrew.h"


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_homebrew_is_present(bool *dest)
{
    int  rc;
    struct stat  info;
    rc = stat("/usr/local/bin/brew", &info);
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
 * Native package database
 */

struct bz_version *
bz_homebrew_native_version_available(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *eof;
    char  *start = NULL;
    char  *end = NULL;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    assert(native_package_name != NULL);

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "brew", "info", native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;
    eof = pe;

    %%{
        machine homebrew_version_available;

        package_name = (alnum | '-' | '_')+;
        version = (alnum | '.' | '-')+;
        main := package_name ": stable "
                (version >{ start = fpc; } %{ end = fpc; })
                any*;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) homebrew_version_available_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version
            ("Unexpected output from brew for package %s", native_package_name);
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_string(start);
    cork_buffer_done(&out);
    return result;
}

struct bz_version *
bz_homebrew_native_version_installed(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *start = NULL;
    char  *end = NULL;
    bool  installed = false;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    assert(native_package_name != NULL);

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "brew", "info", native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine homebrew_version_installed;

        package_name = (alnum | '-' | '_')+;
        version = (alnum | '.' | '-')+;

        installed = (any - '\n')* "/Cellar/" package_name "/"
                    (version >{ start = fpc; } %{ end = fpc; })
                    " (" digit+ " file" (any - '\n')*
                    %{ installed = true; };
        not_installed = "Not installed" %{ installed = false; };

        main := package_name ": stable " version (any - '\n')*  '\n'
                any*  # url, keg only
                (installed | not_installed) '\n'
                any*;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) homebrew_version_installed_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version
            ("Unexpected output from brew for package %s", native_package_name);
        cork_buffer_done(&out);
        return NULL;
    }

    if (installed) {
        *end = '\0';
        result = bz_version_from_string(start);
    } else {
        result = NULL;
    }
    cork_buffer_done(&out);
    return result;
}


static int
bz_homebrew_native__install(const char *native_package_name,
                            struct bz_version *version)
{
    return bz_subprocess_run
        (false, NULL,
         "brew", "install", native_package_name,
         NULL);
}

static int
bz_homebrew_native__uninstall(const char *native_package_name)
{
    /* We don't pass the --needed flag to brew since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "brew", "remove", native_package_name,
         NULL);
}

struct bz_pdb *
bz_homebrew_native_pdb(void)
{
    return bz_native_pdb_new
        ("Homebrew", "homebrew",
         bz_homebrew_native_version_available,
         bz_homebrew_native_version_installed,
         bz_homebrew_native__install,
         bz_homebrew_native__uninstall,
         "%s", "lib%s", NULL);
}
