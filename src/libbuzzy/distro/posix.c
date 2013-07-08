/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/os.h"
#include "buzzy/value.h"
#include "buzzy/distro/posix.h"


/*-----------------------------------------------------------------------
 * Current architecture
 */

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
bz_posix_current_architecture(void)
{
    char  *buf;
    cork_buffer_clear(&architecture);
    rpi_check(bz_subprocess_get_output
              (&architecture, NULL, NULL, "uname", "-m", NULL));
    /* Chomp the trailing newline */
    buf = architecture.buf;
    buf[--architecture.size] = '\0';
    return architecture.buf;
}


static const char *
bz_posix_architecture_value__get(void *user_data, struct bz_value *ctx)
{
    struct cork_buffer  *buf = user_data;
    if (buf->size == 0) {
        cork_buffer_set_string(buf, bz_posix_current_architecture());
    }
    return buf->buf;
}

struct bz_value *
bz_posix_architecture_value_new(void)
{
    struct cork_buffer  *buf = cork_buffer_new();
    return bz_scalar_value_new
        (buf, (cork_free_f) cork_buffer_free, bz_posix_architecture_value__get);
}
