
#line 1 "libbuzzy/dependency.c.rl"
/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdlib.h>

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

    
#line 118 "libbuzzy/dependency.c"
static const int buzzy_dependency_start = 1;

static const int buzzy_dependency_en_main = 1;


#line 124 "libbuzzy/dependency.c"
	{
	cs = buzzy_dependency_start;
	}

#line 129 "libbuzzy/dependency.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 45: goto tr0;
		case 95: goto tr0;
	}
	if ( (*p) < 65 ) {
		if ( 47 <= (*p) && (*p) <= 57 )
			goto tr0;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr0;
	} else
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 119 "libbuzzy/dependency.c.rl"
	{ package_name_start = p; }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 160 "libbuzzy/dependency.c"
	switch( (*p) ) {
		case 32: goto tr6;
		case 45: goto st5;
		case 62: goto tr8;
		case 95: goto st5;
	}
	if ( (*p) < 47 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr6;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st5;
		} else if ( (*p) >= 65 )
			goto st5;
	} else
		goto st5;
	goto st0;
tr6:
#line 120 "libbuzzy/dependency.c.rl"
	{ package_name_end = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 187 "libbuzzy/dependency.c"
	switch( (*p) ) {
		case 32: goto st2;
		case 62: goto st3;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st2;
	goto st0;
tr8:
#line 120 "libbuzzy/dependency.c.rl"
	{ package_name_end = p; }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 203 "libbuzzy/dependency.c"
	if ( (*p) == 61 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 32: goto st4;
		case 43: goto tr5;
		case 46: goto tr5;
		case 126: goto tr5;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st4;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr5;
		} else if ( (*p) >= 65 )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr5:
#line 124 "libbuzzy/dependency.c.rl"
	{ min_version_start = p; }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 237 "libbuzzy/dependency.c"
	switch( (*p) ) {
		case 43: goto st6;
		case 46: goto st6;
		case 126: goto st6;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st6;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st6;
	} else
		goto st6;
	goto st0;
	}
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 5: 
#line 120 "libbuzzy/dependency.c.rl"
	{ package_name_end = p; }
	break;
	case 6: 
#line 125 "libbuzzy/dependency.c.rl"
	{ min_version_end = p; }
	break;
#line 271 "libbuzzy/dependency.c"
	}
	}

	_out: {}
	}

#line 133 "libbuzzy/dependency.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) buzzy_dependency_en_main;

    if (CORK_UNLIKELY(cs < 
#line 285 "libbuzzy/dependency.c"
5
#line 138 "libbuzzy/dependency.c.rl"
)) {
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
