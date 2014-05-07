
#line 1 "libbuzzy/distro/debian.c.rl"
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
    bool  epoch_was_last = false;
    bool  have_release = false;
    bool  need_punct_before_digit = false;

    count = bz_version_part_count(version);
    assert(count > 0);

    for (i = 0; i < count; i++) {
        const char  *string_value;
        const char  *last_char;
        part = bz_version_get_part(version, i);
        string_value = part->string_value.buf;
        switch (part->kind) {
            case BZ_VERSION_EPOCH:
                cork_buffer_append_copy(dest, &part->string_value);
                cork_buffer_append(dest, ":", 1);
                break;

            case BZ_VERSION_RELEASE:
                if (!epoch_was_last &&
                    need_punct_before_digit !=
                    !BZ_VERSION_PART_IS_INTEGRAL(part)) {
                    cork_buffer_append(dest, ".", 1);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_PRERELEASE:
                assert(part->string_value.size > 0);
                cork_buffer_append(dest, "~", 1);
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_POSTRELEASE:
                assert(part->string_value.size > 0);
                if (!have_release &&
                    strcmp(part->string_value.buf, "rev") == 0) {
                    have_release = true;
                    cork_buffer_append(dest, "-", 1);
                } else {
                    cork_buffer_append(dest, "+", 1);
                    cork_buffer_append_copy(dest, &part->string_value);
                }
                break;

            case BZ_VERSION_FINAL:
                return;

            default:
                break;
        }

        epoch_was_last = (part->kind == BZ_VERSION_EPOCH);
        assert(part->string_value.size > 0);
        last_char = strchr(string_value, '\0') - 1;
        need_punct_before_digit = isdigit(*last_char);
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

    
#line 147 "libbuzzy/distro/debian.c"
static const int debian_version_ours_start = 1;

static const int debian_version_ours_en_main = 1;


#line 153 "libbuzzy/distro/debian.c"
	{
	cs = debian_version_ours_start;
	}

#line 158 "libbuzzy/distro/debian.c"
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
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st7;
tr13:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 197 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr10;
		case 45: goto tr11;
		case 46: goto tr12;
		case 58: goto st6;
		case 126: goto tr16;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr15;
	} else
		goto tr15;
	goto st0;
tr10:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 226 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr2;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr3;
	} else
		goto tr3;
	goto st0;
tr5:
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st8;
tr2:
#line 158 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st8;
tr7:
#line 152 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st8;
tr18:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 278 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr10;
		case 45: goto tr11;
		case 46: goto tr12;
		case 126: goto tr16;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st8;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr15;
	} else
		goto tr15;
	goto st0;
tr11:
#line 170 "libbuzzy/distro/debian.c.rl"
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
#line 306 "libbuzzy/distro/debian.c"
	if ( (*p) == 49 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	goto st0;
tr12:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 327 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr5;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr6;
	} else
		goto tr6;
	goto st0;
tr6:
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st10;
tr3:
#line 158 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st10;
tr8:
#line 152 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st10;
tr15:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 379 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr10;
		case 45: goto tr11;
		case 46: goto tr12;
		case 126: goto tr16;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr18;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
tr16:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 407 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr7;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr8;
	} else
		goto tr8;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr9;
	goto st0;
tr9:
#line 164 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start - 1;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, BZ_VERSION_EPOCH, start, size);
        }
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st11;
tr20:
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 146 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 456 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr10;
		case 45: goto tr11;
		case 46: goto tr12;
		case 126: goto tr16;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr20;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr15;
	} else
		goto tr15;
	goto st0;
	}
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 7: 
	case 8: 
	case 10: 
	case 11: 
#line 170 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	break;
#line 499 "libbuzzy/distro/debian.c"
	}
	}

	_out: {}
	}

#line 202 "libbuzzy/distro/debian.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) debian_version_ours_en_main;

    if (CORK_UNLIKELY(cs < 
#line 513 "libbuzzy/distro/debian.c"
7
#line 207 "libbuzzy/distro/debian.c.rl"
)) {
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

    
#line 543 "libbuzzy/distro/debian.c"
static const int debian_version_any_start = 1;

static const int debian_version_any_en_main = 1;


#line 549 "libbuzzy/distro/debian.c"
	{
	cs = debian_version_any_start;
	}

#line 554 "libbuzzy/distro/debian.c"
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
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st10;
tr20:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 593 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr17;
		case 45: goto tr18;
		case 46: goto tr19;
		case 58: goto st9;
		case 126: goto tr23;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr20;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr22;
	} else
		goto tr22;
	goto st0;
tr17:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 622 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr2;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr3;
	} else
		goto tr3;
	goto st0;
tr12:
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st11;
tr2:
#line 248 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st11;
tr14:
#line 242 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st11;
tr34:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 674 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr17;
		case 45: goto tr18;
		case 46: goto tr19;
		case 126: goto tr23;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st11;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr22;
	} else
		goto tr22;
	goto st0;
tr18:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 702 "libbuzzy/distro/debian.c"
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
#line 266 "libbuzzy/distro/debian.c.rl"
	{
            ei_check(bz_version_add_part
                     (version, BZ_VERSION_POSTRELEASE, "rev", 3));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st12;
tr27:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 743 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr25;
		case 46: goto tr26;
		case 126: goto tr29;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr27;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr28;
	} else
		goto tr28;
	goto st0;
tr25:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 770 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr6;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr7;
	} else
		goto tr7;
	goto st0;
tr8:
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st13;
tr6:
#line 248 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st13;
tr10:
#line 242 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st13;
tr31:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 822 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr25;
		case 46: goto tr26;
		case 126: goto tr29;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st13;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr28;
	} else
		goto tr28;
	goto st0;
tr26:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 849 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr8;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr9;
	} else
		goto tr9;
	goto st0;
tr9:
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st14;
tr7:
#line 248 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st14;
tr11:
#line 242 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st14;
tr28:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 901 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr25;
		case 46: goto tr26;
		case 126: goto tr29;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr31;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st14;
	} else
		goto st14;
	goto st0;
tr29:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 928 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr10;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr11;
	} else
		goto tr11;
	goto st0;
tr5:
#line 266 "libbuzzy/distro/debian.c.rl"
	{
            ei_check(bz_version_add_part
                     (version, BZ_VERSION_POSTRELEASE, "rev", 3));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st15;
tr33:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 969 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr25;
		case 46: goto tr26;
		case 126: goto tr29;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr31;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr33;
	} else
		goto tr33;
	goto st0;
tr19:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 996 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr12;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr13;
	} else
		goto tr13;
	goto st0;
tr13:
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st16;
tr3:
#line 248 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st16;
tr15:
#line 242 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st16;
tr22:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 1048 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr17;
		case 45: goto tr18;
		case 46: goto tr19;
		case 126: goto tr23;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr34;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st16;
	} else
		goto st16;
	goto st0;
tr23:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 1076 "libbuzzy/distro/debian.c"
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr14;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr15;
	} else
		goto tr15;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr16;
	goto st0;
tr16:
#line 254 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start - 1;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, BZ_VERSION_EPOCH, start, size);
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st17;
tr36:
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
#line 236 "libbuzzy/distro/debian.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 1125 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr17;
		case 45: goto tr18;
		case 46: goto tr19;
		case 126: goto tr23;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr36;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr22;
	} else
		goto tr22;
	goto st0;
	}
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
#line 260 "libbuzzy/distro/debian.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            ei_check(bz_version_add_part(version, kind, start, size));
        }
	break;
#line 1178 "libbuzzy/distro/debian.c"
	}
	}

	_out: {}
	}

#line 300 "libbuzzy/distro/debian.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) debian_version_any_en_main;

    if (CORK_UNLIKELY(cs < 
#line 1192 "libbuzzy/distro/debian.c"
10
#line 305 "libbuzzy/distro/debian.c.rl"
)) {
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

    
#line 1252 "libbuzzy/distro/debian.c"
static const int debian_version_available_start = 19;

static const int debian_version_available_en_main = 19;


#line 1258 "libbuzzy/distro/debian.c"
	{
	cs = debian_version_available_start;
	}

#line 1263 "libbuzzy/distro/debian.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr12:
#line 365 "libbuzzy/distro/debian.c.rl"
	{ end = p; }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 1277 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 10: goto st19;
		case 86: goto st1;
	}
	goto st0;
st0:
	if ( ++p == pe )
		goto _test_eof0;
case 0:
	if ( (*p) == 10 )
		goto st19;
	goto st0;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 10: goto st19;
		case 101: goto st2;
	}
	goto st0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 10: goto st19;
		case 114: goto st3;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 10: goto st19;
		case 115: goto st4;
	}
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 10: goto st19;
		case 105: goto st5;
	}
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 10: goto st19;
		case 111: goto st6;
	}
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 10: goto st19;
		case 110: goto st7;
	}
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto st19;
		case 58: goto st8;
	}
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 10: goto st20;
		case 32: goto st9;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 10: goto st20;
		case 32: goto st9;
		case 43: goto tr11;
		case 126: goto tr11;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr11;
		} else if ( (*p) >= 9 )
			goto st9;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr11;
		} else if ( (*p) >= 65 )
			goto tr11;
	} else
		goto tr11;
	goto st0;
tr21:
#line 365 "libbuzzy/distro/debian.c.rl"
	{ end = p; }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 1397 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 10: goto st20;
		case 32: goto st9;
		case 43: goto tr11;
		case 86: goto tr23;
		case 126: goto tr11;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto tr11;
		} else if ( (*p) >= 9 )
			goto st9;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr11;
		} else if ( (*p) >= 65 )
			goto tr11;
	} else
		goto tr11;
	goto st0;
tr11:
#line 365 "libbuzzy/distro/debian.c.rl"
	{ start = p; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 1428 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
tr23:
#line 365 "libbuzzy/distro/debian.c.rl"
	{ start = p; }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 1454 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 101: goto st12;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 114: goto st13;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 115: goto st14;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 105: goto st15;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 111: goto st16;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 110: goto st17;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 10: goto tr12;
		case 43: goto st10;
		case 58: goto st18;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st10;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 10: goto tr21;
		case 32: goto st9;
		case 43: goto st10;
		case 126: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st10;
		} else if ( (*p) >= 9 )
			goto st9;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
	}
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof0: cs = 0; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 

	_test_eof: {}
	}

#line 377 "libbuzzy/distro/debian.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) debian_version_available_en_main;

    if (CORK_UNLIKELY(cs < 
#line 1663 "libbuzzy/distro/debian.c"
19
#line 382 "libbuzzy/distro/debian.c.rl"
)) {
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

    
#line 1712 "libbuzzy/distro/debian.c"
static const int debian_version_installed_start = 1;

static const int debian_version_installed_en_main = 1;


#line 1718 "libbuzzy/distro/debian.c"
	{
	cs = debian_version_installed_start;
	}

#line 1723 "libbuzzy/distro/debian.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr12:
#line 430 "libbuzzy/distro/debian.c.rl"
	{ installed = true; }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 1737 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
	}
	goto st1;
tr13:
#line 430 "libbuzzy/distro/debian.c.rl"
	{ installed = true; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 1751 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto tr15;
		case 126: goto tr15;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr15;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr15;
		} else if ( (*p) >= 65 )
			goto tr15;
	} else
		goto tr15;
	goto st0;
st0:
cs = 0;
	goto _out;
tr15:
#line 433 "libbuzzy/distro/debian.c.rl"
	{ start = p; }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 1779 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 43: goto st13;
		case 126: goto st13;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st13;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st13;
		} else if ( (*p) >= 65 )
			goto st13;
	} else
		goto st13;
	goto st0;
tr14:
#line 430 "libbuzzy/distro/debian.c.rl"
	{ installed = true; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 1804 "libbuzzy/distro/debian.c"
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 105: goto st3;
	}
	goto st1;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 110: goto st4;
	}
	goto st1;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 115: goto st5;
	}
	goto st1;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 116: goto st6;
	}
	goto st1;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 97: goto st7;
	}
	goto st1;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 108: goto st8;
	}
	goto st1;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 108: goto st9;
	}
	goto st1;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 101: goto st10;
	}
	goto st1;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 10: goto st12;
		case 32: goto st2;
		case 100: goto st11;
	}
	goto st1;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 10: goto tr13;
		case 32: goto tr14;
	}
	goto tr12;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 13: 
#line 433 "libbuzzy/distro/debian.c.rl"
	{ end = p; }
	break;
#line 1923 "libbuzzy/distro/debian.c"
	}
	}

	_out: {}
	}

#line 439 "libbuzzy/distro/debian.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) debian_version_installed_en_main;

    if (CORK_UNLIKELY(cs < 
#line 1937 "libbuzzy/distro/debian.c"
12
#line 444 "libbuzzy/distro/debian.c.rl"
)) {
        bz_invalid_version("Unexpected output from dpkg-query");
        cork_buffer_done(&out);
        return NULL;
    }

    if (installed && (start == NULL || end == NULL)) {
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
