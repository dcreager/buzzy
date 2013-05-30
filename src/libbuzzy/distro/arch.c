
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

    
#line 178 "libbuzzy/distro/arch.c"
static const int arch_version_start = 1;

static const int arch_version_en_main = 1;


#line 184 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_start;
	}

#line 189 "libbuzzy/distro/arch.c"
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
#line 177 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st4;
tr9:
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 177 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 228 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 112: goto tr11;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr9;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr10;
	} else
		goto tr10;
	goto st0;
tr7:
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 201 "libbuzzy/distro/arch.c.rl"
	{
            start = p + 1;
            clog_trace("  Create new postrelease version part for revision");
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 260 "libbuzzy/distro/arch.c"
	if ( (*p) == 49 )
		goto st6;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st5;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st5;
	goto st0;
tr8:
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 292 "libbuzzy/distro/arch.c"
	if ( (*p) == 112 )
		goto tr6;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr4;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr4:
#line 177 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st7;
tr21:
#line 189 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st7;
tr13:
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 177 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st7;
tr17:
#line 183 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 346 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 112: goto tr11;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st7;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr10;
	} else
		goto tr10;
	goto st0;
tr5:
#line 189 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st8;
tr10:
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 183 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 387 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
tr11:
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 183 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 419 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 114: goto st10;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 101: goto st11;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr17;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
tr6:
#line 189 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 481 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 111: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 115: goto st14;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
		case 116: goto st15;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 45: goto tr7;
		case 46: goto tr8;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr21;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st8;
	} else
		goto st8;
	goto st0;
	}
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 4: 
	case 7: 
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
#line 195 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	break;
	case 5: 
#line 206 "libbuzzy/distro/arch.c.rl"
	{
            bz_version_add_part(version, BZ_VERSION_POSTRELEASE, "rev", 3);
            cork_buffer_set(&buf, start, p - start);
            bz_version_add_part(version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	break;
#line 594 "libbuzzy/distro/arch.c"
	}
	}

	_out: {}
	}

#line 248 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 608 "libbuzzy/distro/arch.c"
4
#line 253 "libbuzzy/distro/arch.c.rl"
)) {
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
               "pacman", "-Sdp", "--print-format", "%v",
               native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    
#line 653 "libbuzzy/distro/arch.c"
static const int arch_version_available_start = 1;

static const int arch_version_available_en_main = 1;


#line 659 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_available_start;
	}

#line 664 "libbuzzy/distro/arch.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr0;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr0;
		} else if ( (*p) >= 65 )
			goto tr0;
	} else
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 298 "libbuzzy/distro/arch.c.rl"
	{ start = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 694 "libbuzzy/distro/arch.c"
	if ( (*p) == 10 )
		goto tr2;
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st2;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st2;
		} else if ( (*p) >= 65 )
			goto st2;
	} else
		goto st2;
	goto st0;
tr2:
#line 298 "libbuzzy/distro/arch.c.rl"
	{ end = p; }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 717 "libbuzzy/distro/arch.c"
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 303 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_available_en_main;

    if (CORK_UNLIKELY(cs < 
#line 734 "libbuzzy/distro/arch.c"
3
#line 308 "libbuzzy/distro/arch.c.rl"
)) {
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

    
#line 773 "libbuzzy/distro/arch.c"
static const int arch_version_installed_start = 1;

static const int arch_version_installed_en_main = 1;


#line 779 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_installed_start;
	}

#line 784 "libbuzzy/distro/arch.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) == 45 )
		goto st2;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st2;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st2;
	} else
		goto st2;
	goto st0;
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 32: goto st3;
		case 45: goto st2;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st2;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st2;
	} else
		goto st2;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr3;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr3;
		} else if ( (*p) >= 65 )
			goto tr3;
	} else
		goto tr3;
	goto st0;
tr3:
#line 349 "libbuzzy/distro/arch.c.rl"
	{ start = p; }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 846 "libbuzzy/distro/arch.c"
	if ( (*p) == 10 )
		goto tr4;
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st4;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st4;
		} else if ( (*p) >= 65 )
			goto st4;
	} else
		goto st4;
	goto st0;
tr4:
#line 349 "libbuzzy/distro/arch.c.rl"
	{ end = p; }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 869 "libbuzzy/distro/arch.c"
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 355 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_installed_en_main;

    if (CORK_UNLIKELY(cs < 
#line 888 "libbuzzy/distro/arch.c"
5
#line 360 "libbuzzy/distro/arch.c.rl"
)) {
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
