
#line 1 "libbuzzy/distro/rpm.c.rl"
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

    
#line 185 "libbuzzy/distro/rpm.c"
static const int rpm_version_start = 1;

static const int rpm_version_en_main = 1;


#line 191 "libbuzzy/distro/rpm.c"
	{
	cs = rpm_version_start;
	}

#line 196 "libbuzzy/distro/rpm.c"
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
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st2;
tr4:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 235 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr2;
		case 45: goto tr3;
		case 46: goto tr2;
		case 95: goto tr2;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr4;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr2:
#line 202 "libbuzzy/distro/rpm.c.rl"
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
#line 263 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto st3;
		case 46: goto st3;
		case 95: goto st3;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr7;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr8;
	} else
		goto tr8;
	goto st0;
tr7:
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st4;
tr18:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 184 "libbuzzy/distro/rpm.c.rl"
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
#line 304 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr2;
		case 45: goto tr3;
		case 46: goto tr2;
		case 95: goto tr2;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st4;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr3:
#line 202 "libbuzzy/distro/rpm.c.rl"
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
#line 332 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 48: goto st6;
		case 49: goto tr11;
	}
	if ( 50 <= (*p) && (*p) <= 57 )
		goto tr12;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 46 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr14;
	goto st0;
tr21:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st10;
tr12:
#line 196 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st10;
tr14:
#line 190 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            clog_trace("  Create new prerelease version part");
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 388 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr20;
		case 46: goto tr20;
		case 95: goto tr20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr21;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr22;
	} else
		goto tr22;
	goto st0;
tr20:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 415 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto st8;
		case 46: goto st8;
		case 95: goto st8;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr16;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr17;
	} else
		goto tr17;
	goto st0;
tr16:
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st11;
tr24:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 184 "libbuzzy/distro/rpm.c.rl"
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
#line 456 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr20;
		case 46: goto tr20;
		case 95: goto tr20;
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
tr17:
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st12;
tr22:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 184 "libbuzzy/distro/rpm.c.rl"
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
#line 497 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr20;
		case 46: goto tr20;
		case 95: goto tr20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr24;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st12;
	} else
		goto st12;
	goto st0;
tr11:
#line 196 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 524 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr20;
		case 46: goto tr20;
		case 95: goto tr20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr21;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr22;
	} else
		goto tr22;
	goto st0;
tr8:
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st9;
tr5:
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 184 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 565 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 43: goto tr2;
		case 45: goto tr3;
		case 46: goto tr2;
		case 95: goto tr2;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr18;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st9;
	} else
		goto st9;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 10: 
	case 11: 
	case 12: 
#line 202 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	break;
#line 609 "libbuzzy/distro/rpm.c"
	}
	}

	_out: {}
	}

#line 249 "libbuzzy/distro/rpm.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) rpm_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 623 "libbuzzy/distro/rpm.c"
10
#line 254 "libbuzzy/distro/rpm.c.rl"
)) {
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

    
#line 670 "libbuzzy/distro/rpm.c"
static const int rpm_version_available_start = 49;

static const int rpm_version_available_en_main = 49;


#line 676 "libbuzzy/distro/rpm.c"
	{
	cs = rpm_version_available_start;
	}

#line 681 "libbuzzy/distro/rpm.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr13:
#line 305 "libbuzzy/distro/rpm.c.rl"
	{ r_end = p; }
	goto st49;
tr35:
#line 302 "libbuzzy/distro/rpm.c.rl"
	{ v_end = p; }
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 699 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto st49;
		case 82: goto st1;
		case 86: goto st43;
	}
	goto st0;
st0:
	if ( ++p == pe )
		goto _test_eof0;
case 0:
	if ( (*p) == 10 )
		goto st49;
	goto st0;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 10: goto st49;
		case 101: goto st2;
	}
	goto st0;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 10: goto st49;
		case 108: goto st3;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 10: goto st49;
		case 101: goto st4;
	}
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 10: goto st49;
		case 97: goto st5;
	}
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 10: goto st49;
		case 115: goto st6;
	}
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 10: goto st49;
		case 101: goto st7;
	}
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 10: goto st50;
		case 32: goto st7;
		case 58: goto st8;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st7;
	goto st0;
tr21:
#line 305 "libbuzzy/distro/rpm.c.rl"
	{ r_end = p; }
	goto st50;
tr43:
#line 302 "libbuzzy/distro/rpm.c.rl"
	{ v_end = p; }
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 791 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto st50;
		case 32: goto st7;
		case 58: goto st8;
		case 82: goto st1;
		case 86: goto st43;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st7;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 10: goto st51;
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
		case 10: goto st51;
		case 32: goto st9;
		case 46: goto tr12;
		case 95: goto tr12;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st9;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr12;
		} else if ( (*p) >= 65 )
			goto tr12;
	} else
		goto tr12;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	switch( (*p) ) {
		case 10: goto st51;
		case 32: goto st9;
		case 46: goto tr12;
		case 82: goto tr58;
		case 86: goto tr59;
		case 95: goto tr12;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st9;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr12;
		} else if ( (*p) >= 65 )
			goto tr12;
	} else
		goto tr12;
	goto st0;
tr12:
#line 305 "libbuzzy/distro/rpm.c.rl"
	{ r_start = p; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 867 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
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
tr58:
#line 305 "libbuzzy/distro/rpm.c.rl"
	{ r_start = p; }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 890 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 101: goto st12;
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
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 108: goto st13;
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
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 101: goto st14;
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
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 97: goto st15;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st10;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st10;
	} else
		goto st10;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 115: goto st16;
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
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 101: goto st17;
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
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 10: goto tr21;
		case 32: goto st7;
		case 46: goto st10;
		case 58: goto st8;
		case 95: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st7;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
tr59:
#line 305 "libbuzzy/distro/rpm.c.rl"
	{ r_start = p; }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 1032 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 101: goto st19;
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
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 114: goto st20;
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
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 115: goto st21;
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
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 105: goto st22;
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
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 111: goto st23;
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
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 10: goto tr13;
		case 46: goto st10;
		case 95: goto st10;
		case 110: goto st24;
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
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 10: goto tr29;
		case 32: goto st25;
		case 46: goto st10;
		case 58: goto st26;
		case 95: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st25;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	switch( (*p) ) {
		case 10: goto st52;
		case 32: goto st25;
		case 58: goto st26;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st25;
	goto st0;
tr29:
#line 305 "libbuzzy/distro/rpm.c.rl"
	{ r_end = p; }
	goto st52;
tr50:
#line 302 "libbuzzy/distro/rpm.c.rl"
	{ v_end = p; }
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1190 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto st52;
		case 32: goto st25;
		case 58: goto st26;
		case 82: goto st1;
		case 86: goto st43;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st25;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 10: goto st53;
		case 32: goto st27;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st27;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	switch( (*p) ) {
		case 10: goto st53;
		case 32: goto st27;
		case 46: goto tr34;
		case 95: goto tr34;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st27;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr34;
		} else if ( (*p) >= 65 )
			goto tr34;
	} else
		goto tr34;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 10: goto st53;
		case 32: goto st27;
		case 46: goto tr34;
		case 82: goto tr60;
		case 86: goto tr61;
		case 95: goto tr34;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st27;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr34;
		} else if ( (*p) >= 65 )
			goto tr34;
	} else
		goto tr34;
	goto st0;
tr34:
#line 302 "libbuzzy/distro/rpm.c.rl"
	{ v_start = p; }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 1266 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
tr60:
#line 302 "libbuzzy/distro/rpm.c.rl"
	{ v_start = p; }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 1289 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 101: goto st30;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 108: goto st31;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 101: goto st32;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 97: goto st33;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 115: goto st34;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 101: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	switch( (*p) ) {
		case 10: goto tr43;
		case 32: goto st7;
		case 46: goto st28;
		case 58: goto st8;
		case 95: goto st28;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st7;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st28;
		} else if ( (*p) >= 65 )
			goto st28;
	} else
		goto st28;
	goto st0;
tr61:
#line 302 "libbuzzy/distro/rpm.c.rl"
	{ v_start = p; }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 1431 "libbuzzy/distro/rpm.c"
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 101: goto st37;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 114: goto st38;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 115: goto st39;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 105: goto st40;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 111: goto st41;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	switch( (*p) ) {
		case 10: goto tr35;
		case 46: goto st28;
		case 95: goto st28;
		case 110: goto st42;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st28;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st28;
	} else
		goto st28;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 10: goto tr50;
		case 32: goto st25;
		case 46: goto st28;
		case 58: goto st26;
		case 95: goto st28;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st25;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st28;
		} else if ( (*p) >= 65 )
			goto st28;
	} else
		goto st28;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	switch( (*p) ) {
		case 10: goto st49;
		case 101: goto st44;
	}
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	switch( (*p) ) {
		case 10: goto st49;
		case 114: goto st45;
	}
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	switch( (*p) ) {
		case 10: goto st49;
		case 115: goto st46;
	}
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	switch( (*p) ) {
		case 10: goto st49;
		case 105: goto st47;
	}
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	switch( (*p) ) {
		case 10: goto st49;
		case 111: goto st48;
	}
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 10: goto st49;
		case 110: goto st25;
	}
	goto st0;
	}
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof0: cs = 0; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 

	_test_eof: {}
	}

#line 317 "libbuzzy/distro/rpm.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) rpm_version_available_en_main;

    if (CORK_UNLIKELY(cs < 
#line 1685 "libbuzzy/distro/rpm.c"
49
#line 322 "libbuzzy/distro/rpm.c.rl"
)) {
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
