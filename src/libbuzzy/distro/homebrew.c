
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
    char  *start;
    char  *end;
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
    char  *start;
    char  *end;
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
	if ( (*p) == 10 )
		goto st14;
	goto st13;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 78: goto st22;
	}
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
	}
	goto st15;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr17;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr17;
	} else
		goto tr17;
	goto st15;
tr17:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 506 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto tr18;
		case 47: goto st16;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st17;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st17;
	} else
		goto st17;
	goto st15;
tr18:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 529 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto st0;
		case 40: goto st19;
		case 47: goto st16;
	}
	goto st15;
tr24:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ end = p; }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 544 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr21;
		case 47: goto st20;
	}
	goto st19;
tr21:
#line 147 "libbuzzy/distro/homebrew.c.rl"
	{ installed = true; }
	goto st35;
tr38:
#line 148 "libbuzzy/distro/homebrew.c.rl"
	{ installed = false; }
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 562 "libbuzzy/distro/homebrew.c"
	goto st35;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 10: goto tr21;
		case 47: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto tr23;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr23;
	} else
		goto tr23;
	goto st19;
tr23:
#line 145 "libbuzzy/distro/homebrew.c.rl"
	{ start = p; }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 589 "libbuzzy/distro/homebrew.c"
	switch( (*p) ) {
		case 10: goto tr21;
		case 32: goto tr24;
		case 47: goto st20;
	}
	if ( (*p) < 65 ) {
		if ( 45 <= (*p) && (*p) <= 57 )
			goto st21;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto st21;
	} else
		goto st21;
	goto st19;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 111: goto st23;
	}
	goto st15;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 116: goto st24;
	}
	goto st15;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 10: goto st0;
		case 32: goto st25;
		case 47: goto st16;
	}
	goto st15;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 105: goto st26;
	}
	goto st15;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 110: goto st27;
	}
	goto st15;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 115: goto st28;
	}
	goto st15;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 116: goto st29;
	}
	goto st15;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 97: goto st30;
	}
	goto st15;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 108: goto st31;
	}
	goto st15;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 108: goto st32;
	}
	goto st15;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 101: goto st33;
	}
	goto st15;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 10: goto st0;
		case 47: goto st16;
		case 100: goto st34;
	}
	goto st15;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	switch( (*p) ) {
		case 10: goto tr38;
		case 47: goto st16;
	}
	goto st15;
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
	_test_eof35: cs = 35; goto _test_eof; 
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
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 158 "libbuzzy/distro/homebrew.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) homebrew_version_installed_en_main;

    if (CORK_UNLIKELY(cs < 
#line 780 "libbuzzy/distro/homebrew.c"
35
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
