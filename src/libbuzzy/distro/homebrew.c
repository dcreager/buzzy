
#line 1 "libbuzzy/distro/homebrew.c.rl"
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

    
#line 86 "libbuzzy/distro/homebrew.c"
static const int homebrew_version_available_start = 1;

static const int homebrew_version_available_en_main = 1;


#line 92 "libbuzzy/distro/homebrew.c"
	{
	cs = homebrew_version_available_start;
	}

#line 97 "libbuzzy/distro/homebrew.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 45: goto st2;
		case 95: goto st2;
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
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 45: goto st2;
		case 58: goto st3;
		case 95: goto st2;
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
	if ( (*p) == 32 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 115 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 116 )
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 97 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 98 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 108 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 101 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 32 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr11;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr11;
		} else if ( (*p) >= 65 )
			goto tr11;
	} else
		goto tr11;
	goto st0;
tr11:
#line 88 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st12;
tr13:
#line 88 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 222 "libbuzzy/distro/homebrew.c"
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr13;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr13;
		} else if ( (*p) >= 65 )
			goto tr13;
	} else
		goto tr13;
	goto tr12;
tr12:
#line 88 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 243 "libbuzzy/distro/homebrew.c"
	goto st13;
	}
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
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 12: 
#line 88 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	break;
#line 267 "libbuzzy/distro/homebrew.c"
	}
	}

	_out: {}
	}

#line 94 "libbuzzy/distro/homebrew.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) homebrew_version_available_en_main;

    if (CORK_UNLIKELY(cs < 
#line 281 "libbuzzy/distro/homebrew.c"
12
#line 99 "libbuzzy/distro/homebrew.c.rl"
)) {
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

    
#line 324 "libbuzzy/distro/homebrew.c"
static const int homebrew_version_installed_start = 1;

static const int homebrew_version_installed_en_main = 1;


#line 330 "libbuzzy/distro/homebrew.c"
	{
	cs = homebrew_version_installed_start;
	}

#line 335 "libbuzzy/distro/homebrew.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 45: goto st2;
		case 95: goto st2;
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
st0:
cs = 0;
	goto _out;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
	switch( (*p) ) {
		case 45: goto st2;
		case 58: goto st3;
		case 95: goto st2;
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
	if ( (*p) == 32 )
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	if ( (*p) == 115 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	if ( (*p) == 116 )
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	if ( (*p) == 97 )
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	if ( (*p) == 98 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( (*p) == 108 )
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 101 )
		goto st10;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	if ( (*p) == 32 )
		goto st11;
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st12;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st12;
		} else if ( (*p) >= 65 )
			goto st12;
	} else
		goto st12;
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	if ( (*p) == 10 )
		goto st13;
	goto st12;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
	}
	goto st13;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 47: goto st14;
		case 67: goto st15;
		case 78: goto st16;
	}
	goto st13;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 101: goto st29;
	}
	goto st13;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 111: goto st17;
	}
	goto st13;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 116: goto st18;
	}
	goto st13;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 32: goto st19;
		case 47: goto st14;
		case 78: goto st16;
	}
	goto st13;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 105: goto st20;
	}
	goto st13;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 110: goto st21;
	}
	goto st13;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 115: goto st22;
	}
	goto st13;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 116: goto st23;
	}
	goto st13;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 97: goto st24;
	}
	goto st13;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 108: goto st25;
	}
	goto st13;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 108: goto st26;
	}
	goto st13;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 101: goto st27;
	}
	goto st13;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 100: goto st28;
	}
	goto st13;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 10: goto tr29;
		case 47: goto st14;
		case 78: goto st16;
	}
	goto st13;
tr29:
#line 148 "libbuzzy/distro/homebrew.c.rl"
	{ installed = false; }
	goto st129;
tr52:
#line 147 "libbuzzy/distro/homebrew.c.rl"
	{ installed = true; }
	goto st129;
tr69:
#line 147 "libbuzzy/distro/homebrew.c.rl"
	{ installed = true; }
#line 148 "libbuzzy/distro/homebrew.c.rl"
	{ installed = false; }
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
#line 632 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
	}
	goto st129;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	switch( (*p) ) {
		case 47: goto st130;
		case 67: goto st131;
		case 78: goto st132;
	}
	goto st129;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 101: goto st145;
	}
	goto st129;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 111: goto st133;
	}
	goto st129;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 116: goto st134;
	}
	goto st129;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	switch( (*p) ) {
		case 32: goto st135;
		case 47: goto st130;
		case 78: goto st132;
	}
	goto st129;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 105: goto st136;
	}
	goto st129;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 110: goto st137;
	}
	goto st129;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 115: goto st138;
	}
	goto st129;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 116: goto st139;
	}
	goto st129;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 97: goto st140;
	}
	goto st129;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 108: goto st141;
	}
	goto st129;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 108: goto st142;
	}
	goto st129;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 101: goto st143;
	}
	goto st129;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 100: goto st144;
	}
	goto st129;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	switch( (*p) ) {
		case 10: goto tr29;
		case 47: goto st130;
		case 78: goto st132;
	}
	goto st129;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 108: goto st146;
	}
	goto st129;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 108: goto st147;
	}
	goto st129;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 97: goto st148;
	}
	goto st129;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 114: goto st149;
	}
	goto st129;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	switch( (*p) ) {
		case 47: goto st150;
		case 78: goto st132;
	}
	goto st129;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st130;
		case 67: goto st228;
		case 78: goto st225;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	switch( (*p) ) {
		case 47: goto st130;
		case 67: goto tr168;
		case 78: goto tr169;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr167;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr167;
	} else
		goto tr167;
	goto st129;
tr167:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st153;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
#line 902 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
tr170:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st154;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
#line 925 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 40: goto st155;
		case 47: goto st130;
		case 78: goto st132;
	}
	goto st129;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st156;
	goto st129;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	switch( (*p) ) {
		case 32: goto st157;
		case 47: goto st130;
		case 78: goto st132;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st156;
	goto st129;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 102: goto st158;
	}
	goto st129;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 105: goto st159;
	}
	goto st129;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 108: goto st160;
	}
	goto st129;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	switch( (*p) ) {
		case 47: goto st130;
		case 78: goto st132;
		case 101: goto st161;
	}
	goto st129;
tr208:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st161;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
#line 1003 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
	}
	goto st161;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 67: goto st163;
		case 78: goto st164;
	}
	goto st161;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 101: goto st177;
	}
	goto st161;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 111: goto st165;
	}
	goto st161;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 116: goto st166;
	}
	goto st161;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto st167;
		case 47: goto st162;
		case 78: goto st164;
	}
	goto st161;
tr213:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st167;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
#line 1073 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 105: goto st168;
	}
	goto st161;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 110: goto st169;
	}
	goto st161;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 115: goto st170;
	}
	goto st161;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 116: goto st171;
	}
	goto st161;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 97: goto st172;
	}
	goto st161;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 108: goto st173;
	}
	goto st161;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 108: goto st174;
	}
	goto st161;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 101: goto st175;
	}
	goto st161;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 100: goto st176;
	}
	goto st161;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
	switch( (*p) ) {
		case 10: goto tr69;
		case 47: goto st162;
		case 78: goto st164;
	}
	goto st161;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 108: goto st178;
	}
	goto st161;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 108: goto st179;
	}
	goto st161;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 97: goto st180;
	}
	goto st161;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 78: goto st164;
		case 114: goto st181;
	}
	goto st161;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st182;
		case 78: goto st164;
	}
	goto st161;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st162;
		case 67: goto st198;
		case 78: goto st195;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st162;
		case 67: goto tr206;
		case 78: goto tr207;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr205;
	} else
		goto tr205;
	goto st161;
tr205:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st185;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
#line 1301 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
tr207:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st186;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
#line 1325 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 111: goto st187;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 116: goto st188;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr213;
		case 47: goto st162;
		case 78: goto st186;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
tr206:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st189;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
#line 1389 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 101: goto st190;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 108: goto st191;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 108: goto st192;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 97: goto st193;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st162;
		case 78: goto st186;
		case 114: goto st194;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 47: goto st182;
		case 78: goto st186;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st185;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st185;
	} else
		goto st185;
	goto st161;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 111: goto st196;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 116: goto st197;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto st167;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 101: goto st199;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 108: goto st200;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 108: goto st201;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 97: goto st202;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st184;
		case 78: goto st195;
		case 95: goto st183;
		case 114: goto st203;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st183;
		case 47: goto st204;
		case 78: goto st195;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st183;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st183;
	} else
		goto st183;
	goto st161;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
	switch( (*p) ) {
		case 10: goto tr52;
		case 46: goto tr205;
		case 47: goto st162;
		case 67: goto tr228;
		case 78: goto tr229;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr227;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr227;
	} else
		goto tr227;
	goto st161;
tr227:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st205;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
#line 1722 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
tr229:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st206;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
#line 1748 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 111: goto st207;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 116: goto st208;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr213;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
tr228:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st209;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
#line 1818 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 101: goto st210;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 108: goto st211;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 108: goto st212;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 97: goto st213;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st184;
		case 78: goto st206;
		case 95: goto st183;
		case 114: goto st214;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr208;
		case 46: goto st185;
		case 47: goto st204;
		case 78: goto st206;
		case 95: goto st183;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st205;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st205;
	} else
		goto st205;
	goto st161;
tr169:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st215;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
#line 1954 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 111: goto st216;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 116: goto st217;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
	switch( (*p) ) {
		case 32: goto tr241;
		case 47: goto st130;
		case 78: goto st215;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
tr241:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st218;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
#line 2015 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 40: goto st155;
		case 47: goto st130;
		case 78: goto st132;
		case 105: goto st136;
	}
	goto st129;
tr168:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st219;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
#line 2031 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 101: goto st220;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 108: goto st221;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 108: goto st222;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 97: goto st223;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st130;
		case 78: goto st215;
		case 114: goto st224;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	switch( (*p) ) {
		case 32: goto tr170;
		case 47: goto st150;
		case 78: goto st215;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st153;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st153;
	} else
		goto st153;
	goto st129;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 111: goto st226;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 116: goto st227;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
	switch( (*p) ) {
		case 32: goto st135;
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 101: goto st229;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 108: goto st230;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 108: goto st231;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 97: goto st232;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st152;
		case 78: goto st225;
		case 95: goto st151;
		case 114: goto st233;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	switch( (*p) ) {
		case 45: goto st151;
		case 47: goto st234;
		case 78: goto st225;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st151;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st151;
	} else
		goto st151;
	goto st129;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
	switch( (*p) ) {
		case 46: goto tr167;
		case 47: goto st130;
		case 67: goto tr256;
		case 78: goto tr257;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr255;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr255;
	} else
		goto tr255;
	goto st129;
tr255:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st235;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
#line 2348 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
tr257:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st236;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
#line 2373 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 111: goto st237;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 116: goto st238;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
	switch( (*p) ) {
		case 32: goto tr241;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
tr256:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st239;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
#line 2440 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 101: goto st240;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 108: goto st241;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 108: goto st242;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 97: goto st243;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st152;
		case 78: goto st236;
		case 95: goto st151;
		case 114: goto st244;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
	switch( (*p) ) {
		case 32: goto tr170;
		case 46: goto st153;
		case 47: goto st234;
		case 78: goto st236;
		case 95: goto st151;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st235;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st235;
	} else
		goto st235;
	goto st129;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 108: goto st30;
	}
	goto st13;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 108: goto st31;
	}
	goto st13;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 97: goto st32;
	}
	goto st13;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 114: goto st33;
	}
	goto st13;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 47: goto st34;
		case 78: goto st16;
	}
	goto st13;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st14;
		case 67: goto st112;
		case 78: goto st109;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	switch( (*p) ) {
		case 47: goto st14;
		case 67: goto tr40;
		case 78: goto tr41;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr39;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr39;
	} else
		goto tr39;
	goto st13;
tr39:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 2676 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
tr42:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 2699 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 40: goto st39;
		case 47: goto st14;
		case 78: goto st16;
	}
	goto st13;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st40;
	goto st13;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 32: goto st41;
		case 47: goto st14;
		case 78: goto st16;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st40;
	goto st13;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 102: goto st42;
	}
	goto st13;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 105: goto st43;
	}
	goto st13;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 108: goto st44;
	}
	goto st13;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	switch( (*p) ) {
		case 47: goto st14;
		case 78: goto st16;
		case 101: goto st45;
	}
	goto st13;
tr82:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 2777 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
	}
	goto st45;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 67: goto st47;
		case 78: goto st48;
	}
	goto st45;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 101: goto st61;
	}
	goto st45;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 111: goto st49;
	}
	goto st45;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 116: goto st50;
	}
	goto st45;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto st51;
		case 47: goto st46;
		case 78: goto st48;
	}
	goto st45;
tr87:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 2847 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 105: goto st52;
	}
	goto st45;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 110: goto st53;
	}
	goto st45;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 115: goto st54;
	}
	goto st45;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 116: goto st55;
	}
	goto st45;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 97: goto st56;
	}
	goto st45;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 108: goto st57;
	}
	goto st45;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 108: goto st58;
	}
	goto st45;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 101: goto st59;
	}
	goto st45;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 100: goto st60;
	}
	goto st45;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	switch( (*p) ) {
		case 10: goto tr69;
		case 47: goto st46;
		case 78: goto st48;
	}
	goto st45;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 108: goto st62;
	}
	goto st45;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 108: goto st63;
	}
	goto st45;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 97: goto st64;
	}
	goto st45;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 78: goto st48;
		case 114: goto st65;
	}
	goto st45;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st66;
		case 78: goto st48;
	}
	goto st45;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st46;
		case 67: goto st82;
		case 78: goto st79;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	switch( (*p) ) {
		case 10: goto tr52;
		case 47: goto st46;
		case 67: goto tr80;
		case 78: goto tr81;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr79;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr79;
	} else
		goto tr79;
	goto st45;
tr79:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 3075 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
tr81:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 3099 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 111: goto st71;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 116: goto st72;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr87;
		case 47: goto st46;
		case 78: goto st70;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
tr80:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 3163 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 101: goto st74;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 108: goto st75;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 108: goto st76;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 97: goto st77;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st46;
		case 78: goto st70;
		case 114: goto st78;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 47: goto st66;
		case 78: goto st70;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st69;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st69;
	} else
		goto st69;
	goto st45;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 111: goto st80;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 116: goto st81;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto st51;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 101: goto st83;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 108: goto st84;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 108: goto st85;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 97: goto st86;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st68;
		case 78: goto st79;
		case 95: goto st67;
		case 114: goto st87;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 10: goto tr52;
		case 45: goto st67;
		case 47: goto st88;
		case 78: goto st79;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st67;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st67;
	} else
		goto st67;
	goto st45;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	switch( (*p) ) {
		case 10: goto tr52;
		case 46: goto tr79;
		case 47: goto st46;
		case 67: goto tr102;
		case 78: goto tr103;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr101;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr101;
	} else
		goto tr101;
	goto st45;
tr101:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st89;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
#line 3496 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
tr103:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st90;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
#line 3522 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 111: goto st91;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 116: goto st92;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr87;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
tr102:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st93;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
#line 3592 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 101: goto st94;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 108: goto st95;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 108: goto st96;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 97: goto st97;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st68;
		case 78: goto st90;
		case 95: goto st67;
		case 114: goto st98;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	switch( (*p) ) {
		case 10: goto tr52;
		case 32: goto tr82;
		case 46: goto st69;
		case 47: goto st88;
		case 78: goto st90;
		case 95: goto st67;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st89;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st89;
	} else
		goto st89;
	goto st45;
tr41:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st99;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
#line 3728 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 111: goto st100;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 116: goto st101;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	switch( (*p) ) {
		case 32: goto tr115;
		case 47: goto st14;
		case 78: goto st99;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
tr115:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 3789 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 40: goto st39;
		case 47: goto st14;
		case 78: goto st16;
		case 105: goto st20;
	}
	goto st13;
tr40:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st103;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
#line 3805 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 101: goto st104;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 108: goto st105;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 108: goto st106;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 97: goto st107;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st14;
		case 78: goto st99;
		case 114: goto st108;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	switch( (*p) ) {
		case 32: goto tr42;
		case 47: goto st34;
		case 78: goto st99;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st37;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st37;
	} else
		goto st37;
	goto st13;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 111: goto st110;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 116: goto st111;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	switch( (*p) ) {
		case 32: goto st19;
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 101: goto st113;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 108: goto st114;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 108: goto st115;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 97: goto st116;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st36;
		case 78: goto st109;
		case 95: goto st35;
		case 114: goto st117;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	switch( (*p) ) {
		case 45: goto st35;
		case 47: goto st118;
		case 78: goto st109;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st35;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st35;
	} else
		goto st35;
	goto st13;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	switch( (*p) ) {
		case 46: goto tr39;
		case 47: goto st14;
		case 67: goto tr130;
		case 78: goto tr131;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr129;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr129;
	} else
		goto tr129;
	goto st13;
tr129:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st119;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
#line 4122 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
tr131:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st120;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
#line 4147 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 111: goto st121;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 116: goto st122;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	switch( (*p) ) {
		case 32: goto tr115;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
tr130:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
#line 4214 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 101: goto st124;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 108: goto st125;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 108: goto st126;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 97: goto st127;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 98 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st36;
		case 78: goto st120;
		case 95: goto st35;
		case 114: goto st128;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	switch( (*p) ) {
		case 32: goto tr42;
		case 46: goto st37;
		case 47: goto st118;
		case 78: goto st120;
		case 95: goto st35;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st119;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st119;
	} else
		goto st119;
	goto st13;
	}
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
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 
	_test_eof131: cs = 131; goto _test_eof; 
	_test_eof132: cs = 132; goto _test_eof; 
	_test_eof133: cs = 133; goto _test_eof; 
	_test_eof134: cs = 134; goto _test_eof; 
	_test_eof135: cs = 135; goto _test_eof; 
	_test_eof136: cs = 136; goto _test_eof; 
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
	_test_eof153: cs = 153; goto _test_eof; 
	_test_eof154: cs = 154; goto _test_eof; 
	_test_eof155: cs = 155; goto _test_eof; 
	_test_eof156: cs = 156; goto _test_eof; 
	_test_eof157: cs = 157; goto _test_eof; 
	_test_eof158: cs = 158; goto _test_eof; 
	_test_eof159: cs = 159; goto _test_eof; 
	_test_eof160: cs = 160; goto _test_eof; 
	_test_eof161: cs = 161; goto _test_eof; 
	_test_eof162: cs = 162; goto _test_eof; 
	_test_eof163: cs = 163; goto _test_eof; 
	_test_eof164: cs = 164; goto _test_eof; 
	_test_eof165: cs = 165; goto _test_eof; 
	_test_eof166: cs = 166; goto _test_eof; 
	_test_eof167: cs = 167; goto _test_eof; 
	_test_eof168: cs = 168; goto _test_eof; 
	_test_eof169: cs = 169; goto _test_eof; 
	_test_eof170: cs = 170; goto _test_eof; 
	_test_eof171: cs = 171; goto _test_eof; 
	_test_eof172: cs = 172; goto _test_eof; 
	_test_eof173: cs = 173; goto _test_eof; 
	_test_eof174: cs = 174; goto _test_eof; 
	_test_eof175: cs = 175; goto _test_eof; 
	_test_eof176: cs = 176; goto _test_eof; 
	_test_eof177: cs = 177; goto _test_eof; 
	_test_eof178: cs = 178; goto _test_eof; 
	_test_eof179: cs = 179; goto _test_eof; 
	_test_eof180: cs = 180; goto _test_eof; 
	_test_eof181: cs = 181; goto _test_eof; 
	_test_eof182: cs = 182; goto _test_eof; 
	_test_eof183: cs = 183; goto _test_eof; 
	_test_eof184: cs = 184; goto _test_eof; 
	_test_eof185: cs = 185; goto _test_eof; 
	_test_eof186: cs = 186; goto _test_eof; 
	_test_eof187: cs = 187; goto _test_eof; 
	_test_eof188: cs = 188; goto _test_eof; 
	_test_eof189: cs = 189; goto _test_eof; 
	_test_eof190: cs = 190; goto _test_eof; 
	_test_eof191: cs = 191; goto _test_eof; 
	_test_eof192: cs = 192; goto _test_eof; 
	_test_eof193: cs = 193; goto _test_eof; 
	_test_eof194: cs = 194; goto _test_eof; 
	_test_eof195: cs = 195; goto _test_eof; 
	_test_eof196: cs = 196; goto _test_eof; 
	_test_eof197: cs = 197; goto _test_eof; 
	_test_eof198: cs = 198; goto _test_eof; 
	_test_eof199: cs = 199; goto _test_eof; 
	_test_eof200: cs = 200; goto _test_eof; 
	_test_eof201: cs = 201; goto _test_eof; 
	_test_eof202: cs = 202; goto _test_eof; 
	_test_eof203: cs = 203; goto _test_eof; 
	_test_eof204: cs = 204; goto _test_eof; 
	_test_eof205: cs = 205; goto _test_eof; 
	_test_eof206: cs = 206; goto _test_eof; 
	_test_eof207: cs = 207; goto _test_eof; 
	_test_eof208: cs = 208; goto _test_eof; 
	_test_eof209: cs = 209; goto _test_eof; 
	_test_eof210: cs = 210; goto _test_eof; 
	_test_eof211: cs = 211; goto _test_eof; 
	_test_eof212: cs = 212; goto _test_eof; 
	_test_eof213: cs = 213; goto _test_eof; 
	_test_eof214: cs = 214; goto _test_eof; 
	_test_eof215: cs = 215; goto _test_eof; 
	_test_eof216: cs = 216; goto _test_eof; 
	_test_eof217: cs = 217; goto _test_eof; 
	_test_eof218: cs = 218; goto _test_eof; 
	_test_eof219: cs = 219; goto _test_eof; 
	_test_eof220: cs = 220; goto _test_eof; 
	_test_eof221: cs = 221; goto _test_eof; 
	_test_eof222: cs = 222; goto _test_eof; 
	_test_eof223: cs = 223; goto _test_eof; 
	_test_eof224: cs = 224; goto _test_eof; 
	_test_eof225: cs = 225; goto _test_eof; 
	_test_eof226: cs = 226; goto _test_eof; 
	_test_eof227: cs = 227; goto _test_eof; 
	_test_eof228: cs = 228; goto _test_eof; 
	_test_eof229: cs = 229; goto _test_eof; 
	_test_eof230: cs = 230; goto _test_eof; 
	_test_eof231: cs = 231; goto _test_eof; 
	_test_eof232: cs = 232; goto _test_eof; 
	_test_eof233: cs = 233; goto _test_eof; 
	_test_eof234: cs = 234; goto _test_eof; 
	_test_eof235: cs = 235; goto _test_eof; 
	_test_eof236: cs = 236; goto _test_eof; 
	_test_eof237: cs = 237; goto _test_eof; 
	_test_eof238: cs = 238; goto _test_eof; 
	_test_eof239: cs = 239; goto _test_eof; 
	_test_eof240: cs = 240; goto _test_eof; 
	_test_eof241: cs = 241; goto _test_eof; 
	_test_eof242: cs = 242; goto _test_eof; 
	_test_eof243: cs = 243; goto _test_eof; 
	_test_eof244: cs = 244; goto _test_eof; 
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
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 158 "libbuzzy/distro/homebrew.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) homebrew_version_installed_en_main;

    if (CORK_UNLIKELY(cs < 
#line 4592 "libbuzzy/distro/homebrew.c"
129
#line 163 "libbuzzy/distro/homebrew.c.rl"
)) {
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
