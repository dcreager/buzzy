/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_RUN_H
#define BUZZY_RUN_H

#include <stdarg.h>

#include <libcork/core.h>
#include <libcork/ds.h>


/*-----------------------------------------------------------------------
 * Executing subprocesses
 */

typedef cork_array(const char *)  bz_subprocess_cmd;


/* Execute the subprocess, wait for it to finish, and capture its stdout and
 * stderr streams into the given buffers.  (The buffers can be NULL, in which
 * case that output stream is ignored.) */

int
bz_subprocess_a_get_output(struct cork_buffer *out, struct cork_buffer *err,
                           bz_subprocess_cmd *cmd);

int
bz_subprocess_v_get_output(struct cork_buffer *out, struct cork_buffer *err,
                           va_list args);

CORK_ATTR_SENTINEL
int
bz_subprocess_get_output(struct cork_buffer *out, struct cork_buffer *err, ...);


/* Execute the subprocess and wait for it to finish.
 *
 * If verbose is true, then the subprocess's stdout and stderr will be the same
 * as the calling process, so the end user will see the subprocess's output.  If
 * verbose is false, then the subprocess's output will be ignored.
 *
 * If successful is non-NULL, we'll fill it in with whether the subprocess's
 * exit code was 0 (indicating success). */

int
bz_subprocess_v_run(bool verbose, bool *successful, va_list args);

int
bz_subprocess_a_run(bool verbose, bool *successful, bz_subprocess_cmd *cmd);

CORK_ATTR_SENTINEL
int
bz_subprocess_run(bool verbose, bool *successful, ...);


/*-----------------------------------------------------------------------
 * Mocking subprocesses for test cases
 */

void
bz_subprocess_start_mocks(void);

/* To make it easier to specify the mocks, you provide a full command line,
 * which is the result of concatenating (separated by spaces) the program and
 * params parameters from one of the above subprocess-calling functions. */
void
bz_subprocess_mock(const char *cmd, const char *out, const char *err,
                   int exit_code);

/* Allow a particular command to actually be executed. */
void
bz_subprocess_mock_allow_execute(const char *cmd);


#endif /* BUZZY_RUN_H */
