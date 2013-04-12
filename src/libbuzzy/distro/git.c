
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
#include "buzzy/os.h"
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

    
#line 57 "libbuzzy/distro/git.c"
static const int git_version_start = 1;

static const int git_version_en_main = 1;


#line 63 "libbuzzy/distro/git.c"
	{
	cs = git_version_start;
	}

#line 68 "libbuzzy/distro/git.c"
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
#line 56 "libbuzzy/distro/git.c.rl"
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
#line 104 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr17;
		case 46: goto tr18;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st9;
	goto st0;
tr17:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 71 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	goto st2;
tr20:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 78 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
	goto st2;
tr31:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 85 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	goto st2;
tr44:
#line 113 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 114 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 85 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 172 "libbuzzy/distro/git.c"
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
#line 56 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st10;
tr11:
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 221 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr20;
		case 46: goto tr21;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st10;
	goto st0;
tr18:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 71 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	goto st4;
tr21:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 78 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
	goto st4;
tr32:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 85 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	goto st4;
tr45:
#line 113 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 114 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 85 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 289 "libbuzzy/distro/git.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr2;
	goto st0;
tr9:
#line 56 "libbuzzy/distro/git.c.rl"
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
#line 305 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr23;
		case 46: goto tr21;
		case 112: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr24;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st11;
	} else
		goto st11;
	goto st0;
tr23:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 330 "libbuzzy/distro/git.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr11;
	goto st0;
tr13:
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st12;
tr24:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 356 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr20;
		case 46: goto tr21;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st12;
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
		case 45: goto tr23;
		case 46: goto tr21;
		case 112: goto st13;
		case 114: goto st14;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr24;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st11;
	} else
		goto st11;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 45: goto tr23;
		case 46: goto tr21;
		case 112: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr24;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 100 ) {
			if ( 102 <= (*p) && (*p) <= 122 )
				goto st11;
		} else if ( (*p) >= 97 )
			goto st11;
	} else
		goto st11;
	goto st0;
tr10:
#line 56 "libbuzzy/distro/git.c.rl"
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
#line 422 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr23;
		case 46: goto tr21;
		case 112: goto st13;
		case 114: goto st16;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr24;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st11;
	} else
		goto st11;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 45: goto tr23;
		case 46: goto tr21;
		case 101: goto st6;
		case 112: goto st13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr24;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st11;
	} else
		goto st11;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 45 )
		goto st5;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr13;
	goto st0;
tr4:
#line 56 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st17;
tr14:
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 485 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr31;
		case 46: goto tr32;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st17;
	goto st0;
tr5:
#line 56 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            cork_buffer_clear(&buf);
            DEBUG("  Create new version part\n");
        }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 505 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
tr34:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 530 "libbuzzy/distro/git.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr14;
	goto st0;
tr16:
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st19;
tr35:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 556 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr31;
		case 46: goto tr32;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st19;
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
		case 45: goto tr34;
		case 46: goto tr32;
		case 111: goto st21;
		case 112: goto st20;
		case 114: goto st23;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
		case 115: goto st22;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 115 ) {
			if ( 117 <= (*p) && (*p) <= 122 )
				goto st18;
		} else if ( (*p) >= 97 )
			goto st18;
	} else
		goto st18;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 100 ) {
			if ( 102 <= (*p) && (*p) <= 122 )
				goto st18;
		} else if ( (*p) >= 97 )
			goto st18;
	} else
		goto st18;
	goto st0;
tr6:
#line 56 "libbuzzy/distro/git.c.rl"
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
#line 663 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr42;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 102 ) {
			if ( 103 <= (*p) && (*p) <= 122 )
				goto st18;
		} else if ( (*p) >= 97 )
			goto tr43;
	} else
		goto st18;
	goto st0;
tr47:
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st25;
tr42:
#line 112 "libbuzzy/distro/git.c.rl"
	{ git_start = p; }
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 62 "libbuzzy/distro/git.c.rl"
	{
            start = p;
            DEBUG("  Looking for a trailer\n");
        }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 709 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr44;
		case 46: goto tr45;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st25;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 102 ) {
			if ( 103 <= (*p) && (*p) <= 122 )
				goto st19;
		} else if ( (*p) >= 97 )
			goto st25;
	} else
		goto st19;
	goto st0;
tr43:
#line 112 "libbuzzy/distro/git.c.rl"
	{ git_start = p; }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 734 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr45;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr47;
	} else if ( (*p) > 90 ) {
		if ( (*p) > 102 ) {
			if ( 103 <= (*p) && (*p) <= 122 )
				goto st18;
		} else if ( (*p) >= 97 )
			goto st26;
	} else
		goto st18;
	goto st0;
tr7:
#line 56 "libbuzzy/distro/git.c.rl"
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
#line 764 "libbuzzy/distro/git.c"
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 111: goto st28;
		case 112: goto st20;
		case 114: goto st30;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
		case 115: goto st29;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 112: goto st20;
		case 116: goto st8;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 45 )
		goto st7;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr16;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 45: goto tr34;
		case 46: goto tr32;
		case 101: goto st6;
		case 112: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st18;
	} else
		goto st18;
	goto st0;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
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
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 9: 
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 71 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add release part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_RELEASE, buf.buf, buf.size);
        }
	break;
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 78 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add prerelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_PRERELEASE, buf.buf, buf.size);
        }
	break;
	case 17: 
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
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
#line 85 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	break;
	case 25: 
	case 26: 
#line 113 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_set(&buf, "git", 3); }
#line 114 "libbuzzy/distro/git.c.rl"
	{ cork_buffer_append(&buf, git_start, p - git_start); }
#line 85 "libbuzzy/distro/git.c.rl"
	{
            DEBUG("  Add postrelease part\n");
            DEBUG("    String value: %.*s\n", (int) buf.size, (char *) buf.buf);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
#line 67 "libbuzzy/distro/git.c.rl"
	{
            cork_buffer_append(&buf, start, p - start);
        }
	break;
#line 957 "libbuzzy/distro/git.c"
	}
	}

	_out: {}
	}

#line 141 "libbuzzy/distro/git.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) git_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 971 "libbuzzy/distro/git.c"
9
#line 146 "libbuzzy/distro/git.c.rl"
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
bz_git_version__provide(void *user_data, struct bz_env *env)
{
    struct bz_git_version  *git = user_data;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct cork_buffer  dirty = CORK_BUFFER_INIT();

    if (git->version == NULL) {
        bool  successful;
        struct cork_path  *source_dir;
        struct cork_exec  *exec;

        /* Grab the base version string from "git describe" */
        ep_check(source_dir = bz_env_get_path(env, "source_dir", true));
        exec = cork_exec_new_with_params("git", "describe", NULL);
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        cork_path_free(source_dir);
        ei_check(bz_subprocess_get_output_exec(&out, NULL, &successful, exec));
        if (!successful) {
            goto error;
        }

        /* Chomp the trailing newline */
        ((char *) out.buf)[--out.size] = '\0';

        /* If the working tree is dirty, append "+dirty" to the version. */
        ep_check(source_dir = bz_env_get_path(env, "source_dir", true));
        exec = cork_exec_new_with_params("git", "status", "--porcelain", NULL);
        cork_exec_set_cwd(exec, cork_path_get(source_dir));
        cork_path_free(source_dir);
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

struct bz_value_provider *
bz_git_version_value_new(void)
{
    struct bz_git_version  *git = cork_new(struct bz_git_version);
    git->version = NULL;
    return bz_value_provider_new
        (git, bz_git_version__free, bz_git_version__provide);
}
