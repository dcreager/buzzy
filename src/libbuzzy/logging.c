/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <stdarg.h>

#include <libcork/core.h>
#include <libcork/helpers/errors.h>

#include "buzzy/logging.h"
#include "buzzy/mock.h"


void
bz_real__print_action(struct cork_buffer *buf)
{
    printf("%s\n", (char *) buf->buf);
}


static size_t  action_count = 0;

void
bz_log_action(const char *fmt, ...)
{
    struct cork_buffer  message = CORK_BUFFER_INIT();
    va_list  args;
    cork_buffer_printf(&message, "[%zu] ", ++action_count);
    va_start(args, fmt);
    cork_buffer_append_vprintf(&message, fmt, args);
    va_end(args);
    bz_mocked_print_action(&message);
    cork_buffer_done(&message);
}

void
bz_reset_action_count(void)
{
    action_count = 0;
}
