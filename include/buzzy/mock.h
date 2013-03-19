/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_MOCK_H
#define BUZZY_MOCK_H

#include <libcork/core.h>

#include "buzzy/action.h"
#include "buzzy/os.h"


/*-----------------------------------------------------------------------
 * Mocking for test cases
 */

void
bz_start_mocks(void);

/* To make it easier to specify the mocks, you provide a full command line,
 * which is the result of concatenating (separated by spaces) the program and
 * params parameters from one of the above subprocess-calling functions. */
void
bz_mock_subprocess(const char *cmd, const char *out, const char *err,
                   int exit_code);

/* Allow a particular command to actually be executed. */
void
bz_mock_subprocess_allow_execute(const char *cmd);

void
bz_mock_file_exists(const char *path, bool exists);


/* Querying which mocked functions were executed */

void
bz_mocked_actions_clear(void);

const char *
bz_mocked_actions_run(void);

const char *
bz_mocked_commands_run(void);


/*-----------------------------------------------------------------------
 * Mockable functions
 */

struct bz_mock {
    int
    (*exec)(struct cork_exec *exec, struct cork_stream_consumer *out,
            struct cork_stream_consumer *err, int *exit_code);

    struct cork_file *
    (*create_dir)(struct cork_path *path);

    struct cork_file *
    (*create_file)(struct cork_path *path, struct cork_buffer *src);

    int
    (*file_exists)(struct cork_path *path, bool *exists);

    void
    (*print_action)(struct cork_buffer *buf);
};

extern struct bz_mock  *bz_mocks;

#define bz_mocked_exec(e, out, err, ec) \
    (bz_mocks->exec((e), (out), (err), (ec)))
#define bz_mocked_create_dir(p) \
    (bz_mocks->create_dir((p)))
#define bz_mocked_create_file(p, s) \
    (bz_mocks->create_file((p), (s)))
#define bz_mocked_file_exists(p, e) \
    (bz_mocks->file_exists((p), (e)))
#define bz_mocked_print_action(b) \
    (bz_mocks->print_action((b)))


/*-----------------------------------------------------------------------
 * Real implementations
 */

int
bz_real__exec(struct cork_exec *exec, struct cork_stream_consumer *out,
              struct cork_stream_consumer *err, int *exit_code);

struct cork_file *
bz_real__create_dir(struct cork_path *path);

struct cork_file *
bz_real__create_file(struct cork_path *path, struct cork_buffer *src);

int
bz_real__file_exists(struct cork_path *path, bool *exists);

void
bz_real__print_action(struct cork_buffer *buf);


#endif /* BUZZY_MOCK_H */
