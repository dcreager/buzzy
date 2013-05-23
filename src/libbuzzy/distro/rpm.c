
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

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/version.h"
#include "buzzy/distro/rpm.h"

#define CLOG_CHANNEL  "rpm"


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
                if (seen_non_release_tag) {
                    cork_buffer_append(dest, ".1.", 3);
                } else {
                    cork_buffer_append(dest, ".", 1);
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
                    seen_non_release_tag = true;
                }
                break;

            default:
                break;
        }

        cork_buffer_append_copy(dest, &part->string_value);
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

    
#line 154 "libbuzzy/distro/rpm.c"
static const int rpm_version_start = 1;

static const int rpm_version_en_main = 1;


#line 160 "libbuzzy/distro/rpm.c"
	{
	cs = rpm_version_start;
	}

#line 165 "libbuzzy/distro/rpm.c"
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
#line 153 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st2;
tr4:
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 153 "libbuzzy/distro/rpm.c.rl"
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
#line 204 "libbuzzy/distro/rpm.c"
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
#line 171 "libbuzzy/distro/rpm.c.rl"
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
#line 232 "libbuzzy/distro/rpm.c"
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
#line 153 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st4;
tr18:
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 153 "libbuzzy/distro/rpm.c.rl"
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
#line 273 "libbuzzy/distro/rpm.c"
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
#line 171 "libbuzzy/distro/rpm.c.rl"
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
#line 301 "libbuzzy/distro/rpm.c"
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
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 153 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st10;
tr12:
#line 165 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            clog_trace("  Create new postrelease version part");
        }
	goto st10;
tr14:
#line 159 "libbuzzy/distro/rpm.c.rl"
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
#line 357 "libbuzzy/distro/rpm.c"
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
#line 171 "libbuzzy/distro/rpm.c.rl"
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
#line 384 "libbuzzy/distro/rpm.c"
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
#line 153 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st11;
tr24:
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 153 "libbuzzy/distro/rpm.c.rl"
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
#line 425 "libbuzzy/distro/rpm.c"
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
#line 153 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st12;
tr22:
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 153 "libbuzzy/distro/rpm.c.rl"
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
#line 466 "libbuzzy/distro/rpm.c"
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
#line 165 "libbuzzy/distro/rpm.c.rl"
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
#line 493 "libbuzzy/distro/rpm.c"
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
#line 153 "libbuzzy/distro/rpm.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            clog_trace("  Create new release version part");
        }
	goto st9;
tr5:
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 153 "libbuzzy/distro/rpm.c.rl"
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
#line 534 "libbuzzy/distro/rpm.c"
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
#line 171 "libbuzzy/distro/rpm.c.rl"
	{
            size_t  size = p - start;
            clog_trace("    String value: %.*s", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	break;
#line 578 "libbuzzy/distro/rpm.c"
	}
	}

	_out: {}
	}

#line 218 "libbuzzy/distro/rpm.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) rpm_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 592 "libbuzzy/distro/rpm.c"
10
#line 223 "libbuzzy/distro/rpm.c.rl"
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
