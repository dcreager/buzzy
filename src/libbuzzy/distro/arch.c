
#line 1 "libbuzzy/distro/arch.c.rl"
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

    
#line 143 "libbuzzy/distro/arch.c"
static const int arch_version_start = 1;

static const int arch_version_en_main = 1;


#line 149 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_start;
	}

#line 154 "libbuzzy/distro/arch.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 142 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            DEBUG("  Create new release version part\n");
        }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 179 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
		case 112: goto tr13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st8;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr12;
	} else
		goto tr12;
	goto st0;
tr9:
#line 160 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 166 "libbuzzy/distro/arch.c.rl"
	{
            start = p + 1;
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 210 "libbuzzy/distro/arch.c"
	if ( (*p) == 49 )
		goto st3;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st9;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st9;
	goto st0;
tr10:
#line 160 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 242 "libbuzzy/distro/arch.c"
	if ( (*p) == 112 )
		goto tr5;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr0;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr4;
	} else
		goto tr4;
	goto st0;
tr4:
#line 154 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            DEBUG("  Create new postrelease version part\n");
        }
	goto st5;
tr12:
#line 160 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 148 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            DEBUG("  Create new prerelease version part\n");
        }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 280 "libbuzzy/distro/arch.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
tr16:
#line 154 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            DEBUG("  Create new postrelease version part\n");
        }
	goto st10;
tr18:
#line 148 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            DEBUG("  Create new prerelease version part\n");
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 310 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
tr5:
#line 154 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            DEBUG("  Create new postrelease version part\n");
        }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 336 "libbuzzy/distro/arch.c"
	if ( (*p) == 111 )
		goto st11;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
		case 115: goto st12;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
		case 116: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr16;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr16;
	} else
		goto tr16;
	goto st0;
tr13:
#line 160 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 148 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            DEBUG("  Create new prerelease version part\n");
        }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 419 "libbuzzy/distro/arch.c"
	if ( (*p) == 114 )
		goto st14;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
		case 101: goto st15;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr18;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr18;
	} else
		goto tr18;
	goto st0;
	}
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 8: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
#line 160 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	break;
	case 9: 
#line 170 "libbuzzy/distro/arch.c.rl"
	{
            cork_buffer_set(&buf, "rev", 3);
            cork_buffer_append(&buf, start, p - start);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	break;
#line 509 "libbuzzy/distro/arch.c"
	}
	}

	_out: {}
	}

#line 211 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 523 "libbuzzy/distro/arch.c"
8
#line 216 "libbuzzy/distro/arch.c.rl"
)) {
        bz_invalid_version("Invalid Arch version \"%s\"", arch_version);
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    return version;
}
