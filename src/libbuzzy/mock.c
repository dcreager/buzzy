/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/mock.h"


/*-----------------------------------------------------------------------
 * Real implementations
 */

static struct bz_mock  real_implementations = {
    bz_real__exec,
    bz_real__create_dir,
    bz_real__create_file,
    bz_real__file_exists,
    bz_real__print_action,
    bz_real__walk_directory
};


/*-----------------------------------------------------------------------
 * Mocked state
 */

static bool  mocks_enabled = false;
static struct cork_hash_table  *mocks;
static struct cork_buffer  actions_run;
static struct cork_buffer  commands_run;

struct bz_subprocess_mock;

static void
free_mocks(void)
{
    if (mocks_enabled) {
        cork_hash_table_free(mocks);
        cork_buffer_done(&actions_run);
        cork_buffer_done(&commands_run);
        mocks_enabled = false;
    }
}


/*-----------------------------------------------------------------------
 * Mocking subprocesses
 */

struct bz_subprocess_mock {
    const char  *cmd;
    const char  *out;
    const char  *err;
    int  exit_code;
    bool  allow_execute;
};

static struct bz_subprocess_mock *
bz_subprocess_mock_new(const char *cmd, const char *out, const char *err,
                       int exit_code, bool allow_execute)
{
    struct bz_subprocess_mock  *mock = cork_new(struct bz_subprocess_mock);
    mock->cmd = cork_strdup(cmd);
    mock->out = (out == NULL)? NULL: cork_strdup(out);
    mock->err = (err == NULL)? NULL: cork_strdup(err);
    mock->exit_code = exit_code;
    mock->allow_execute = allow_execute;
    return mock;
}

static void
bz_subprocess_mock_free(struct bz_subprocess_mock *mock)
{
    cork_strfree(mock->cmd);
    if (mock->out != NULL) {
        cork_strfree(mock->out);
    }
    if (mock->err != NULL) {
        cork_strfree(mock->err);
    }
    free(mock);
}

static void
bz_subprocess_add_mock(const char *cmd, const char *out, const char *err,
                       int exit_code, bool allow_execute)
{
    struct bz_subprocess_mock  *mock;
    assert(mocks_enabled);
    mock = bz_subprocess_mock_new(cmd, out, err, exit_code, allow_execute);
    cork_hash_table_put(mocks, (void *) mock->cmd, mock, NULL, NULL, NULL);
}

void
bz_mock_subprocess(const char *cmd, const char *out, const char *err,
                   int exit_code)
{
    bz_subprocess_add_mock(cmd, out, err, exit_code, false);
}

void
bz_mock_subprocess_allow_execute(const char *cmd)
{
    bz_subprocess_add_mock(cmd, NULL, NULL, 0, true);
}

void
bz_mock_file_exists(const char *path, bool exists)
{
    struct cork_buffer  cmd = CORK_BUFFER_INIT();
    cork_buffer_printf(&cmd, "[ -f %s ]", path);
    bz_subprocess_add_mock(cmd.buf, NULL, NULL, exists? 0: 1, false);
    cork_buffer_done(&cmd);
}


static int
bz_mocked__exec(struct cork_exec *exec, struct cork_stream_consumer *out,
                struct cork_stream_consumer *err, int *exit_code)
{
    size_t  i;
    struct bz_subprocess_mock  *mock;
    struct cork_buffer  mock_key = CORK_BUFFER_INIT();

    assert(mocks_enabled);

    /* Construct the mock key for this subprocess. */
    for (i = 0; i < cork_exec_param_count(exec); i++) {
        const char  *param = cork_exec_param(exec, i);
        if (i > 0) {
            cork_buffer_append(&mock_key, " ", 1);
        }
        cork_buffer_append_string(&mock_key, param);
    }

    /* Look for a mock entry for this command. */
    mock = cork_hash_table_get(mocks, mock_key.buf);
    if (CORK_UNLIKELY(mock == NULL)) {
        bz_subprocess_error
            ("No mock for command \"%s\"", (char *) mock_key.buf);
        cork_buffer_done(&mock_key);
        return -1;
    }
    cork_buffer_append(&commands_run, "$ ", 2);
    cork_buffer_append(&commands_run, mock_key.buf, mock_key.size);
    cork_buffer_append(&commands_run, "\n", 1);
    cork_buffer_done(&mock_key);

    /* "Run" the mocked command. */
    if (mock->allow_execute) {
        return bz_real__exec(exec, out, err, exit_code);
    }

    cork_exec_free(exec);

    if (out != NULL) {
        if (mock->out == NULL) {
            rii_check(cork_stream_consumer_data(out, NULL, 0, true));
        } else {
            rii_check(cork_stream_consumer_data
                      (out, mock->out, strlen(mock->out), true));
        }
        rii_check(cork_stream_consumer_eof(out));
    }

    if (err != NULL) {
        if (mock->err == NULL) {
            rii_check(cork_stream_consumer_data(err, NULL, 0, true));
        } else {
            rii_check(cork_stream_consumer_data
                      (err, mock->err, strlen(mock->err), true));
        }
        rii_check(cork_stream_consumer_eof(err));
    }

    if (exit_code != NULL) {
        *exit_code = mock->exit_code;
    }

    return 0;
}


/*-----------------------------------------------------------------------
 * Mocking files and directories
 */

static struct cork_file *
bz_mocked__create_dir(struct cork_path *path)
{
    cork_buffer_append_printf
        (&commands_run, "$ mkdir -p %s\n", cork_path_get(path));
    return cork_file_new_from_path(path);
}

static struct cork_file *
bz_mocked__create_file(struct cork_path *path, struct cork_buffer *src)
{
    assert(mocks_enabled);
    cork_buffer_append_printf
        (&commands_run, "$ cat > %s <<EOF\n", cork_path_get(path));
    cork_buffer_append(&commands_run, src->buf, src->size);
    cork_buffer_append(&commands_run, "EOF\n", 4);
    return cork_file_new_from_path(path);
}

static int
bz_mocked__file_exists(struct cork_path *path, bool *exists)
{
    struct bz_subprocess_mock  *mock;
    struct cork_buffer  cmd = CORK_BUFFER_INIT();

    cork_buffer_printf(&cmd, "[ -f %s ]", cork_path_get(path));
    mock = cork_hash_table_get(mocks, cmd.buf);
    if (CORK_UNLIKELY(mock == NULL)) {
        bz_subprocess_error("No mock for file \"%s\"", cork_path_get(path));
        cork_buffer_done(&cmd);
        return -1;
    }

    cork_buffer_append_printf(&commands_run, "$ %s\n", (char *) cmd.buf);
    cork_buffer_done(&cmd);
    *exists = (mock->exit_code == 0);
    return 0;
}

static int
bz_mocked__walk_directory(const char *path, struct cork_dir_walker *walker)
{
    /* TODO: For now in test cases, we assume that there aren't any files in the
     * directory that we want to walk.  We'll need to add in support for
     * non-empty mocked directories at some point. */
    return 0;
}


/*-----------------------------------------------------------------------
 * Mocking actions
 */

static void
bz_mocked__print_action(const char *message)
{
    cork_buffer_append_string(&actions_run, message);
    cork_buffer_append(&actions_run, "\n", 1);
}


/*-----------------------------------------------------------------------
 * Mocked implementations
 */

static struct bz_mock  mocked_implementations = {
    bz_mocked__exec,
    bz_mocked__create_dir,
    bz_mocked__create_file,
    bz_mocked__file_exists,
    bz_mocked__print_action,
    bz_mocked__walk_directory
};


/*-----------------------------------------------------------------------
 * Wrappers
 */

/* Start by using the "real" executor */
struct bz_mock  *bz_mocks = &real_implementations;

void
bz_start_mocks(void)
{
    if (!mocks_enabled) {
        cork_cleanup_at_exit(0, free_mocks);
    }

    /* Free any existing mocks first. */
    free_mocks();

    bz_reset_action_count();
    mocks = cork_string_hash_table_new(0, 0);
    cork_hash_table_set_free_value
        (mocks, (cork_free_f) bz_subprocess_mock_free);
    cork_buffer_init(&actions_run);
    cork_buffer_append(&actions_run, "", 0);
    cork_buffer_init(&commands_run);
    cork_buffer_append(&commands_run, "", 0);
    mocks_enabled = true;
    bz_mocks = &mocked_implementations;
}

void
bz_mocked_actions_clear(void)
{
    bz_reset_action_count();
    cork_buffer_clear(&actions_run);
}

const char *
bz_mocked_actions_run(void)
{
    assert(mocks_enabled);
    return actions_run.buf;
}

const char *
bz_mocked_commands_run(void)
{
    assert(mocks_enabled);
    return commands_run.buf;
}
