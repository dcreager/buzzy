
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
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/error.h"
#include "buzzy/native.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
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

static struct cork_buffer  architecture = CORK_BUFFER_INIT();

static void
done_architecture(void)
{
    cork_buffer_done(&architecture);
}

CORK_INITIALIZER(init_architecture)
{
    cork_cleanup_at_exit(0, done_architecture);
}

const char *
bz_arch_current_architecture(void)
{
    if (architecture.size == 0) {
        char  *buf;
        rpi_check(bz_subprocess_get_output
                  (&architecture, NULL, NULL, "uname", "-m", NULL));
        /* Chomp the trailing newline */
        buf = architecture.buf;
        buf[--architecture.size] = '\0';
    }
    return architecture.buf;
}


/*-----------------------------------------------------------------------
 * Arch version strings
 */

#define cork_buffer_append_copy(dest, src) \
    (cork_buffer_append((dest), (src)->buf, (src)->size))

static bool
handle_rev_tag(struct bz_version_part *part, struct cork_buffer *dest)
{
    size_t  i;
    const char  *part_string = part->string_value.buf;

    /* We already know this is a postrelease part, and that it's at the end of
     * the Arch version.  Check whether it has the form "revXX", where XX
     * consists only of digits. */
    if (part->string_value.size <= 3) {
        return false;
    }
    if (memcmp(part_string, "rev", 3) != 0) {
        return false;
    }
    for (i = 3; i < part->string_value.size; i++) {
        if (!isdigit(part_string[i])) {
            return false;
        }
    }

    /* Is it rev1?  We ignore that. */
    if (part->string_value.size == 4 && part_string[3] == '1') {
        return true;
    }

    cork_buffer_append(dest, "-", 1);
    cork_buffer_append(dest, part_string + 3, part->string_value.size - 3);
    return true;
}

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

                /* Treat a +revXX tag specially, if it occurs at the end of the
                 * Buzzy version.  That kind of tag should become the Arch
                 * version release tag. */
                if (i == count-2 && handle_rev_tag(part, dest)) {
                    break;
                }

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

    
#line 214 "libbuzzy/distro/arch.c"
static const int arch_version_start = 1;

static const int arch_version_en_main = 1;


#line 220 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_start;
	}

#line 225 "libbuzzy/distro/arch.c"
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
#line 213 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_RELEASE;
            start = p;
            DEBUG("  Create new release version part\n");
        }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 250 "libbuzzy/distro/arch.c"
	switch( (*p) ) {
		case 45: goto tr9;
		case 46: goto tr10;
		case 112: goto tr13;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto st7;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr12;
	} else
		goto tr12;
	goto st0;
tr9:
#line 231 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 237 "libbuzzy/distro/arch.c.rl"
	{
            start = p + 1;
            DEBUG("  Create new postrelease version part for Arch revision\n");
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 282 "libbuzzy/distro/arch.c"
	if ( (*p) == 49 )
		goto st9;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st8;
	goto st0;
tr10:
#line 231 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 314 "libbuzzy/distro/arch.c"
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
#line 225 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            DEBUG("  Create new postrelease version part\n");
        }
	goto st4;
tr12:
#line 231 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 219 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            DEBUG("  Create new prerelease version part\n");
        }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 352 "libbuzzy/distro/arch.c"
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
#line 225 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            DEBUG("  Create new postrelease version part\n");
        }
	goto st10;
tr18:
#line 219 "libbuzzy/distro/arch.c.rl"
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
#line 382 "libbuzzy/distro/arch.c"
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
#line 225 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_POSTRELEASE;
            start = p;
            DEBUG("  Create new postrelease version part\n");
        }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 408 "libbuzzy/distro/arch.c"
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
#line 231 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
#line 219 "libbuzzy/distro/arch.c.rl"
	{
            kind = BZ_VERSION_PRERELEASE;
            start = p;
            DEBUG("  Create new prerelease version part\n");
        }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 491 "libbuzzy/distro/arch.c"
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
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 7: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
#line 231 "libbuzzy/distro/arch.c.rl"
	{
            size_t  size = p - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }
	break;
	case 8: 
#line 242 "libbuzzy/distro/arch.c.rl"
	{
            cork_buffer_set(&buf, "rev", 3);
            cork_buffer_append(&buf, start, p - start);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }
	break;
#line 581 "libbuzzy/distro/arch.c"
	}
	}

	_out: {}
	}

#line 285 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_en_main;

    if (CORK_UNLIKELY(cs < 
#line 595 "libbuzzy/distro/arch.c"
7
#line 290 "libbuzzy/distro/arch.c.rl"
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

    
#line 640 "libbuzzy/distro/arch.c"
static const int arch_version_available_start = 1;

static const int arch_version_available_en_main = 1;


#line 646 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_available_start;
	}

#line 651 "libbuzzy/distro/arch.c"
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
#line 335 "libbuzzy/distro/arch.c.rl"
	{ start = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 681 "libbuzzy/distro/arch.c"
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
#line 335 "libbuzzy/distro/arch.c.rl"
	{ end = p; }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 704 "libbuzzy/distro/arch.c"
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 340 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_available_en_main;

    if (CORK_UNLIKELY(cs < 
#line 721 "libbuzzy/distro/arch.c"
3
#line 345 "libbuzzy/distro/arch.c.rl"
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

    
#line 760 "libbuzzy/distro/arch.c"
static const int arch_version_installed_start = 1;

static const int arch_version_installed_en_main = 1;


#line 766 "libbuzzy/distro/arch.c"
	{
	cs = arch_version_installed_start;
	}

#line 771 "libbuzzy/distro/arch.c"
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
#line 386 "libbuzzy/distro/arch.c.rl"
	{ start = p; }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 833 "libbuzzy/distro/arch.c"
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
#line 386 "libbuzzy/distro/arch.c.rl"
	{ end = p; }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 856 "libbuzzy/distro/arch.c"
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 392 "libbuzzy/distro/arch.c.rl"


    /* A hack to suppress some unused variable warnings */
    (void) arch_version_installed_en_main;

    if (CORK_UNLIKELY(cs < 
#line 875 "libbuzzy/distro/arch.c"
5
#line 397 "libbuzzy/distro/arch.c.rl"
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
bz_arch_native_install(const char *native_package_name,
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

struct bz_pdb *
bz_arch_native_pdb(void)
{
    return bz_native_pdb_new
        ("Arch",
         bz_arch_native_version_available,
         bz_arch_native_version_installed,
         bz_arch_native_install,
         "%s", "lib%s", NULL);
}


/*-----------------------------------------------------------------------
 * Creating Arch packages
 */

struct bz_pacman_packager {
    struct bz_package_spec  *spec;
    struct cork_path  *package_path;
    struct cork_path  *staging_path;
    bool  verbose;
};

static void
bz_pacman_packager__free(void *user_data)
{
    struct bz_pacman_packager  *packager = user_data;
    cork_path_free(packager->package_path);
    cork_path_free(packager->staging_path);
    free(packager);
}

static void
bz_pacman_packager__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_pacman_packager  *packager = user_data;
    cork_buffer_append_printf
        (dest, "Package %s %s",
         bz_package_spec_name(packager->spec),
         bz_package_spec_version_string(packager->spec));
}

static int
bz_pacman_packager__is_needed(void *user_data, bool *is_needed)
{
    /* TODO: Only create the binary package if it doesn't already exist. */
    *is_needed = true;
    return 0;
}

static int
bz_pacman_packager__perform(void *user_data)
{
    struct bz_pacman_packager  *packager = user_data;
    struct cork_path  *pkg_path;
    struct cork_file  *pkg_dir;
    struct cork_exec  *exec;
    struct cork_env  *env;
    struct cork_path  *filename;
    struct cork_file  *pkgbuild = NULL;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    bool  staging_exists;

    const char  *package_name = bz_package_spec_name(packager->spec);
    const char  *version = bz_package_spec_version_string(packager->spec);
    const char  *architecture = bz_arch_current_architecture();
    const char  *license = bz_package_spec_license(packager->spec);

    if (license == NULL) {
        license = "unknown";
    }

    rii_check(bz_file_exists(packager->staging_path, &staging_exists));
    if (CORK_UNLIKELY(!staging_exists)) {
        cork_error_set
            (CORK_BUILTIN_ERROR, CORK_SYSTEM_ERROR,
             "Staging directory %s does not exist",
             cork_path_get(packager->staging_path));
        return -1;
    }

    /* Create a temporary directory */
    pkg_path = cork_path_user_cache_path();
    cork_path_append(pkg_path, "buzzy/arch/package");
    cork_path_append(pkg_path, package_name);
    cork_path_append(pkg_path, version);
    rip_check(pkg_dir = bz_create_directory(pkg_path));

    /* Create a PKGBUILD file for this package */
    cork_buffer_append_printf(&buf, "pkgname='%s'\n", package_name);
    cork_buffer_append_printf(&buf, "pkgver='%s'\n", version);
    cork_buffer_append_printf(&buf, "pkgrel='%s'\n", BZ_ARCH_RELEASE);
    cork_buffer_append_printf(&buf, "arch=('%s')\n", architecture);
    cork_buffer_append_printf(&buf, "license=('%s')\n", license);
    /* TODO: dependencies */
    cork_buffer_append_printf(&buf,
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '%s' \"${pkgdir}\"\n"
        "}\n",
        cork_path_get(packager->staging_path)
    );

    filename = cork_path_join(pkg_path, "PKGBUILD");
    ep_check(pkgbuild = bz_create_file(filename, &buf));
    cork_buffer_done(&buf);

    env = cork_env_clone_current();
    cork_env_add(env, "PKGDEST", cork_path_get(packager->package_path));
    cork_env_add(env, "PKGEXT", ".pkg.tar.xz");

    exec = cork_exec_new_with_params("makepkg", "-sf", NULL);
    cork_exec_set_cwd(exec, cork_path_get(pkg_path));
    cork_exec_set_env(exec, env);

    cork_file_free(pkg_dir);
    cork_file_free(pkgbuild);
    return bz_subprocess_run_exec(packager->verbose, NULL, exec);

error:
    cork_file_free(pkg_dir);
    if (pkgbuild != NULL) {
        cork_file_free(pkgbuild);
    }
    cork_buffer_done(&buf);
    return -1;
}

struct bz_action *
bz_pacman_create_package(struct bz_package_spec *spec,
                         struct cork_path *package_path,
                         struct cork_path *staging_path,
                         struct bz_action *stage_action,
                         bool verbose)
{
    struct bz_action  *action;
    struct bz_action  *prereq;
    struct bz_pacman_packager  *packager;

    packager = cork_new(struct bz_pacman_packager);
    packager->spec = spec;
    packager->package_path = package_path;
    packager->staging_path = staging_path;
    packager->verbose = verbose;

    action = bz_action_new
        (packager, bz_pacman_packager__free,
         bz_pacman_packager__message,
         bz_pacman_packager__is_needed,
         bz_pacman_packager__perform);

    ep_check(prereq = bz_install_dependency_string("pacman"));
    bz_action_add_pre(action, prereq);

    if (stage_action != NULL) {
        bz_action_add_pre(action, stage_action);
    }

    return action;

error:
    bz_action_free(action);
    return NULL;
}
