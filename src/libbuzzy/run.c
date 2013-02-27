/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/error.h"
#include "buzzy/run.h"


typedef int
(*subprocess_executor)(bz_subprocess_cmd *cmd,
                       struct cork_stream_consumer *out,
                       struct cork_stream_consumer *err,
                       int *exit_code);


/*-----------------------------------------------------------------------
 * Mocking subprocesses for test cases
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

static bool  mocks_enabled = false;
static struct cork_hash_table  mocks;

static enum cork_hash_table_map_result
free_mock(struct cork_hash_table_entry *entry, void *user_data)
{
    struct bz_subprocess_mock  *mock = entry->value;
    bz_subprocess_mock_free(mock);
    return CORK_HASH_TABLE_MAP_DELETE;
}

static void
free_mocks(void)
{
    if (mocks_enabled) {
        cork_hash_table_map(&mocks, free_mock, NULL);
        cork_hash_table_done(&mocks);
        mocks_enabled = false;
    }
}

static void
bz_subprocess_add_mock(const char *cmd, const char *out, const char *err,
                       int exit_code, bool allow_execute)
{
    struct bz_subprocess_mock  *mock;
    void  *v_old_mock = NULL;

    assert(mocks_enabled);
    mock = bz_subprocess_mock_new(cmd, out, err, exit_code, allow_execute);
    cork_hash_table_put
        (&mocks, (void *) mock->cmd, mock, NULL, NULL, &v_old_mock);
    if (v_old_mock != NULL) {
        struct bz_subprocess_mock  *old_mock = v_old_mock;
        bz_subprocess_mock_free(old_mock);
    }
}

void
bz_subprocess_mock(const char *cmd, const char *out, const char *err,
                   int exit_code)
{
    bz_subprocess_add_mock(cmd, out, err, exit_code, false);
}

void
bz_subprocess_mock_allow_execute(const char *cmd)
{
    bz_subprocess_add_mock(cmd, NULL, NULL, 0, true);
}

static int
bz_subprocess_execute(bz_subprocess_cmd *cmd,
                      struct cork_stream_consumer *out,
                      struct cork_stream_consumer *err,
                      int *exit_code)
{
    struct cork_subprocess  *subprocess;
    struct cork_subprocess_group  *group;

    /* Make sure that the subprocess command's parameter list is
     * NULL-terminated. */
    cork_array_append(cmd, NULL);

    subprocess = cork_subprocess_new_exec
        (cork_array_at(cmd, 0), (char **) &cork_array_at(cmd, 0),
         out, err, exit_code);
    group = cork_subprocess_group_new();
    cork_subprocess_group_add(group, subprocess);
    rii_check(cork_subprocess_group_start(group));
    rii_check(cork_subprocess_group_wait(group));
    cork_subprocess_group_free(group);
    return 0;
}

static int
bz_subprocess_mock_execute(bz_subprocess_cmd *cmd,
                           struct cork_stream_consumer *out,
                           struct cork_stream_consumer *err,
                           int *exit_code)
{
    size_t  i;
    struct bz_subprocess_mock  *mock;
    struct cork_buffer  mock_key = CORK_BUFFER_INIT();

    assert(mocks_enabled);

    /* Construct the mock key for this subprocess. */
    for (i = 0; i < cork_array_size(cmd); i++) {
        const char  *param = cork_array_at(cmd, i);
        if (i > 0) {
            cork_buffer_append(&mock_key, " ", 1);
        }
        cork_buffer_append_string(&mock_key, param);
    }

    /* Look for a mock entry for this command. */
    mock = cork_hash_table_get(&mocks, mock_key.buf);
    if (CORK_UNLIKELY(mock == NULL)) {
        bz_subprocess_error
            ("No mock for command \"%s\"", (char *) mock_key.buf);
        cork_buffer_done(&mock_key);
        return -1;
    }
    cork_buffer_done(&mock_key);

    /* "Run" the mocked command. */
    if (mock->allow_execute) {
        return bz_subprocess_execute(cmd, out, err, exit_code);
    }

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

/* Start by using the "real" executor */
static subprocess_executor  executor = bz_subprocess_execute;

void
bz_subprocess_start_mocks(void)
{
    /* Free any existing mocks first. */
    free_mocks();

    cork_string_hash_table_init(&mocks, 0);
    mocks_enabled = true;
    executor = bz_subprocess_mock_execute;
}


/*-----------------------------------------------------------------------
 * Executing subprocesses
 */

static int
drop_consumer__data(struct cork_stream_consumer *consumer, const void *buf,
                    size_t size, bool is_first_chunk)
{
    return 0;
}

static int
drop_consumer__eof(struct cork_stream_consumer *consumer)
{
    return 0;
}

static void
drop_consumer__free(struct cork_stream_consumer *consumer)
{
    /* nothing to do */
}

static struct cork_stream_consumer  drop_consumer = {
    drop_consumer__data,
    drop_consumer__eof,
    drop_consumer__free
};

int
bz_subprocess_a_get_output(struct cork_buffer *out_buf,
                           struct cork_buffer *err_buf,
                           bool *successful, bz_subprocess_cmd *cmd)
{
    int  rc;
    int  exit_code;
    struct cork_stream_consumer  *out;
    struct cork_stream_consumer  *err;

    assert(cork_array_size(cmd) > 0);

    out = (out_buf == NULL)? NULL: cork_buffer_to_stream_consumer(out_buf);
    err = (err_buf == NULL)? NULL: cork_buffer_to_stream_consumer(err_buf);
    rc = executor(cmd, out, err, &exit_code);
    if (out != NULL) {
        cork_stream_consumer_free(out);
    }
    if (err != NULL) {
        cork_stream_consumer_free(err);
    }
    rii_check(rc);

    if (successful == NULL) {
        if (CORK_UNLIKELY(exit_code != 0)) {
            bz_subprocess_error("%s failed", cork_array_at(cmd, 0));
            return -1;
        }
    } else {
        *successful = (exit_code == 0);
    }

    return 0;
}

int
bz_subprocess_v_get_output(struct cork_buffer *out_buf,
                           struct cork_buffer *err_buf,
                           bool *successful, va_list args)
{
    int  rc;
    const char  *param;
    bz_subprocess_cmd  cmd;

    cork_array_init(&cmd);
    while ((param = va_arg(args, const char *)) != NULL) {
        cork_array_append(&cmd, param);
    }
    rc = bz_subprocess_a_get_output(out_buf, err_buf, successful, &cmd);
    cork_array_done(&cmd);
    return rc;
}

int
bz_subprocess_get_output(struct cork_buffer *out_buf,
                         struct cork_buffer *err_buf,
                         bool *successful, ...)
{
    int  rc;
    va_list  args;
    va_start(args, successful);
    rc = bz_subprocess_v_get_output(out_buf, err_buf, successful, args);
    va_end(args);
    return rc;

}

int
bz_subprocess_a_run(bool verbose, bool *successful, bz_subprocess_cmd *cmd)
{
    int  rc;
    int  exit_code;
    struct cork_stream_consumer  *out;
    struct cork_stream_consumer  *err;

    assert(cork_array_size(cmd) > 0);

    out = verbose? NULL: &drop_consumer;
    err = verbose? NULL: &drop_consumer;
    rc = executor(cmd, out, err, &exit_code);
    if (out != NULL) {
        cork_stream_consumer_free(out);
    }
    if (err != NULL) {
        cork_stream_consumer_free(err);
    }
    rii_check(rc);

    if (successful == NULL) {
        if (CORK_UNLIKELY(exit_code != 0)) {
            bz_subprocess_error("%s failed", cork_array_at(cmd, 0));
            return -1;
        }
    } else {
        *successful = (exit_code == 0);
    }

    return 0;
}

int
bz_subprocess_v_run(bool verbose, bool *successful, va_list args)
{
    int  rc;
    const char  *param;
    bz_subprocess_cmd  cmd;

    cork_array_init(&cmd);
    while ((param = va_arg(args, const char *)) != NULL) {
        cork_array_append(&cmd, param);
    }
    rc = bz_subprocess_a_run(verbose, successful, &cmd);
    cork_array_done(&cmd);
    return rc;
}

int
bz_subprocess_run(bool verbose, bool *successful, ...)
{
    int  rc;
    va_list  args;
    va_start(args, successful);
    rc = bz_subprocess_v_run(verbose, successful, args);
    va_end(args);
    return rc;

}
