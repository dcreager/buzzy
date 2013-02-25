/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/version.h"


#if !defined(BZ_DEBUG_DEPENDENCIES)
#define BZ_DEBUG_DEPENDENCIES  0
#endif

#if BZ_DEBUG_DEPENDENCIES
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * Dependencies
 */

void
bz_dependency_free(struct bz_dependency *dep)
{
    cork_strfree(dep->package_name);
    if (dep->min_version != NULL) {
        bz_version_free(dep->min_version);
    }
    cork_buffer_done(&dep->string);
}

struct bz_dependency *
bz_dependency_new(const char *package_name, struct bz_version *min_version)
{
    struct bz_dependency  *dep = cork_new(struct bz_dependency);
    dep->package_name = cork_strdup(package_name);
    dep->min_version = min_version;
    cork_buffer_init(&dep->string);
    cork_buffer_append_string(&dep->string, package_name);
    if (min_version != NULL) {
        cork_buffer_append_printf
            (&dep->string, " >= %s", bz_version_to_string(min_version));
    }
    return dep;
}


static struct bz_dependency *
bz_dependency_from_string_parts(const char *string,
                                const char *name_start,
                                const char *name_end,
                                const char *version_start,
                                const char *version_end)
{
    struct cork_buffer  package_name_buf = CORK_BUFFER_INIT();
    struct cork_buffer  min_version_buf = CORK_BUFFER_INIT();
    struct bz_dependency  *result;
    struct bz_version  *min_version = NULL;

    if (CORK_UNLIKELY(name_start == name_end)) {
        goto error;
    }

    cork_buffer_set(&package_name_buf, name_start, name_end - name_start);

    if (version_end != NULL) {
        if (CORK_UNLIKELY(version_start == version_end)) {
            goto error;
        }
        cork_buffer_set
            (&min_version_buf, version_start, version_end - version_start);
        ep_check(min_version = bz_version_from_string(min_version_buf.buf));
    }

    ep_check(result = bz_dependency_new(package_name_buf.buf, min_version));
    cork_buffer_done(&package_name_buf);
    cork_buffer_done(&min_version_buf);
    return result;

error:
    cork_buffer_done(&package_name_buf);
    cork_buffer_done(&min_version_buf);
    bz_invalid_dependency("Invalid dependency \"%s\"", string);
    return NULL;
}


struct bz_dependency *
bz_dependency_from_string(const char *string)
{
    int  cs;
    const char  *p = string;
    const char  *pe = strchr(string, '\0');
    const char  *eof = pe;
    const char  *package_name_start = NULL;
    const char  *package_name_end = NULL;
    const char  *min_version_start = NULL;
    const char  *min_version_end = NULL;

    %%{
        machine buzzy_dependency;

        package_char = alnum | '-' | '_' | '/';
        package_name = package_char+
                       >{ package_name_start = fpc; }
                       %{ package_name_end = fpc; };

        version_char = alnum | '.' | '~' | '+';
        version = version_char+
                  >{ min_version_start = fpc; }
                  %{ min_version_end = fpc; };

        main := package_name
                (space* ">=" space* version)?;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) buzzy_dependency_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_dependency("Invalid dependency \"%s\"", string);
        return NULL;
    }

    return bz_dependency_from_string_parts
        (string, package_name_start, package_name_end,
         min_version_start, min_version_end);
}

const char *
bz_dependency_to_string(const struct bz_dependency *dep)
{
    return dep->string.buf;
}
