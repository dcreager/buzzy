
#line 1 "libbuzzy/distro/git.c.rl"
/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <string.h>

#include <libcork/core.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/git.h"


#if !defined(BZ_DEBUG_GIT)
#define BZ_DEBUG_GIT  0
#endif

#if BZ_DEBUG_GIT
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * git version strings
 */

struct bz_version *
bz_version_from_git_describe(const char *git_version)
{
    int  cs;
    const char  *p = git_version;
    const char  *pe = strchr(git_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    const char  *start;
    const char  *git_start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    DEBUG("---\nParse git version \"%s\"\n", git_version);
    version = bz_version_new();

    
#line 59 "libbuzzy/distro/git.c"
static const int git_version_start = 1;

static const int git_version_en_main = 1;


#line 65 "libbuzzy/distro/git.c"
	{
	cs = git_version_start;
	}

#line 70 "libbuzzy/distro/git.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	if ( (*p) == 45 )
		goto st1;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr2;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st1;
	} else
		goto st1;
	goto st0;
st0:
cs = 0;
	goto _out;
tr2:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st9;
tr20:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 73 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 125 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr18;
		case 46: goto tr19;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr20;
	goto st0;
tr18:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 73 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	goto st2;
tr21:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 80 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
	goto st2;
tr32:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	goto st2;
tr43:
#line 117 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 118 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	goto st2;
tr46:
#line 117 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 118 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 206 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto st3;
		case 103: goto tr6;
		case 112: goto tr7;
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
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 112 )
		goto tr10;
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr8;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr9;
	} else
		goto tr9;
	goto st0;
tr8:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st10;
tr14:
#line 64 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 255 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st10;
	goto st0;
tr19:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 73 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	goto st4;
tr22:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 80 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
	goto st4;
tr33:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	goto st4;
tr44:
#line 117 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 118 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	goto st4;
tr47:
#line 117 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 118 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 336 "libbuzzy/distro/git.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr11;
	goto st0;
tr11:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st11;
tr25:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 80 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st11;
tr35:
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 390 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr18;
		case 46: goto tr19;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st11;
	goto st0;
tr9:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 410 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
		case 112: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr25;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st12;
	} else
		goto st12;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
		case 112: goto st13;
		case 114: goto st14;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr25;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st12;
	} else
		goto st12;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
		case 112: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr25;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 100 ) {
			if ( 102 <= (*p) && (*p) <= 122 )
				goto st12;
		} else if ( (*p) >= 97 )
			goto st12;
	} else
		goto st12;
	goto st0;
tr10:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 477 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
		case 112: goto st13;
		case 114: goto st16;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr25;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st12;
	} else
		goto st12;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
		case 101: goto st5;
		case 112: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr25;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st12;
	} else
		goto st12;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 45 )
		goto st6;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr13;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr14;
	goto st0;
tr13:
#line 64 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 539 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr21;
		case 46: goto tr22;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st17;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st17;
	} else
		goto st17;
	goto st0;
tr4:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st18;
tr17:
#line 64 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 572 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st18;
	goto st0;
tr5:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 592 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 111: goto st21;
		case 112: goto st20;
		case 114: goto st23;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
		case 115: goto st22;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 115 ) {
			if ( 117 <= (*p) && (*p) <= 122 )
				goto st19;
		} else if ( (*p) >= 97 )
			goto st19;
	} else
		goto st19;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 100 ) {
			if ( 102 <= (*p) && (*p) <= 122 )
				goto st19;
		} else if ( (*p) >= 97 )
			goto st19;
	} else
		goto st19;
	goto st0;
tr6:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 700 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr41;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 102 ) {
			if ( 103 <= (*p) && (*p) <= 122 )
				goto st19;
		} else if ( (*p) >= 97 )
			goto tr42;
	} else
		goto st19;
	goto st0;
tr41:
#line 116 "libbuzzy/distro/git.c.rl"
	{ git_start = p; }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 726 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr43;
		case 46: goto tr44;
	}
	if ( (*p) > 57 ) {
		if ( 97 <= (*p) && (*p) <= 102 )
			goto st25;
	} else if ( (*p) >= 48 )
		goto st25;
	goto st0;
tr42:
#line 116 "libbuzzy/distro/git.c.rl"
	{ git_start = p; }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 745 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr46;
		case 46: goto tr47;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st25;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 102 ) {
			if ( 103 <= (*p) && (*p) <= 122 )
				goto st19;
		} else if ( (*p) >= 97 )
			goto st26;
	} else
		goto st19;
	goto st0;
tr7:
#line 58 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 775 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 111: goto st28;
		case 112: goto st20;
		case 114: goto st31;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
		case 115: goto st29;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 112: goto st20;
		case 116: goto st7;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 45 )
		goto st8;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr16;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr17;
	goto st0;
tr16:
#line 64 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 857 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st30;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st30;
	} else
		goto st30;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 45: goto tr32;
		case 46: goto tr33;
		case 101: goto st5;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st19;
	} else
		goto st19;
	goto st0;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 9: 
	case 11: 
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 73 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	break;
	case 10: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 80 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
	break;
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 27: 
	case 28: 
	case 29: 
	case 30: 
	case 31: 
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	break;
	case 25: 
#line 117 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 118 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	break;
	case 26: 
#line 117 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 118 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 87 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 69 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	break;
#line 1014 "libbuzzy/distro/git.c"
	}
	}

	_out: {}
	}

#line 143 "libbuzzy/distro/git.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) git_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 1028 "libbuzzy/distro/git.c"
9
#line 148 "libbuzzy/distro/git.c.rl"
)) {
        bz_invalid_version("Invalid git version \"%s\"", git_version);
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    cork_buffer_done(&buf);
    return version;
}


/*-----------------------------------------------------------------------
 * git version values
 */

struct bz_git_version {
    struct bz_version  *version;
};

static void
bz_git_version__free(void *user_data)
{
    struct bz_git_version  *git = user_data;
    if (git->version != NULL) {
        bz_version_free(git->version);
    }
    free(git);
}

static const char *
bz_git_version__get(void *user_data, struct bz_value *ctx)
{
    struct bz_git_version  *git = user_data;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct cork_buffer  dirty = CORK_BUFFER_INIT();

    if (git->version == NULL) {
        bool  successful;
        struct cork_path  *source_dir;
        struct cork_exec  *exec;

        /* Grab the base version string from "git describe" */
        ep_check(source_dir = bz_value_get_path(ctx, "source_dir", true));
        exec = cork_exec_new_with_params("git", "describe", NULL);
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        ei_check(bz_subprocess_get_output_exec(&out, NULL, &successful, exec));
        if (!successful) {
            goto error;
        }

        /* Chomp the trailing newline */
        ((char *) out.buf)[--out.size] = '\0';

        /* If the working tree is dirty, append "+dirty" to the version. */
        exec = cork_exec_new_with_params("git", "status", "--porcelain", NULL);
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        ei_check(bz_subprocess_get_output_exec
                 (&dirty, NULL, &successful, exec));
        if (!successful) {
            goto error;
        }
        if (dirty.size > 0) {
            cork_buffer_append_string(&out, "-dirty");
        }

        ep_check(git->version = bz_version_from_git_describe(out.buf));
    }

    cork_buffer_done(&out);
    cork_buffer_done(&dirty);
    return bz_version_to_string(git->version);

error:
    cork_buffer_done(&out);
    cork_buffer_done(&dirty);
    return NULL;
}

struct bz_value *
bz_git_version_value_new(void)
{
    struct bz_git_version  *git = cork_new(struct bz_git_version);
    git->version = NULL;
    return bz_scalar_value_new(git, bz_git_version__free, bz_git_version__get);
}


/*-----------------------------------------------------------------------
 * git repository actions
 */

static int
bz_git_perform_clone(const char *url, const char *commit,
                     struct cork_path *dest_dir)
{
    struct cork_path  *parent;

    /* Create the parent directory of our clone. */
    parent = cork_path_dirname(dest_dir);
    ei_check(bz_create_directory(cork_path_get(parent)));
    cork_path_free(parent);

    return bz_subprocess_run
        (false, NULL,
         "git", "clone", "--recursive",
         "--branch", commit,
         url,
         cork_path_get(dest_dir),
         NULL);

error:
    cork_path_free(parent);
    return -1;
}

static int
bz_git_perform_update(const char *url, const char *commit,
                      struct cork_path *dest_dir)
{
    /* Otherwise perform a fetch + reset.  Assume that we've already checked out
     * the right branch. */
    struct cork_buffer  remote_commit = CORK_BUFFER_INIT();
    cork_buffer_printf(&remote_commit, "origin/%s", commit);
    ei_check(bz_subprocess_run
             (false, NULL,
              "git",
              "--git-dir", cork_path_get(dest_dir),
              "--work-tree", cork_path_get(dest_dir),
              "fetch", "origin",
              NULL));
    ei_check(bz_subprocess_run
             (false, NULL,
              "git",
              "--git-dir", cork_path_get(dest_dir),
              "--work-tree", cork_path_get(dest_dir),
              "reset", "--hard", remote_commit.buf,
              NULL));
    cork_buffer_done(&remote_commit);
    return 0;

error:
    cork_buffer_done(&remote_commit);
    return -1;
}


int
bz_git_clone(const char *url, const char *commit, struct cork_path *dest_dir)
{
    bool  exists;

    /* If the target directory already exists, there's nothing to clone. */
    rii_check(bz_file_exists(cork_path_get(dest_dir), &exists));
    if (exists) {
        return 0;
    } else {
        bz_log_action("Clone %s (%s)", url, commit);
        return bz_git_perform_clone(url, commit, dest_dir);
    }
}

int
bz_git_update(const char *url, const char *commit, struct cork_path *dest_dir)
{
    bool  exists;

    /* If the destination directory doesn't exist yet, perform a clone instead
     * of an update. */
    rii_check(bz_file_exists(cork_path_get(dest_dir), &exists));
    if (exists) {
        bz_log_action("Update %s (%s)", url, commit);
        return bz_git_perform_update(url, commit, dest_dir);
    } else {
        bz_log_action("Clone %s (%s)", url, commit);
        return bz_git_perform_clone(url, commit, dest_dir);
    }

}
