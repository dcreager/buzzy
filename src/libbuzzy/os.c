/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <clogger.h>
#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>
#include <libcork/helpers/posix.h>

#include "buzzy/error.h"
#include "buzzy/mock.h"
#include "buzzy/os.h"

#define CLOG_CHANNEL  "os"


/*-----------------------------------------------------------------------
 * Executing subprocesses
 */

int
bz_real__exec(struct cork_exec *exec, struct cork_stream_consumer *out,
              struct cork_stream_consumer *err, int *exit_code)
{
    struct cork_subprocess  *subprocess;
    struct cork_subprocess_group  *group;
    subprocess = cork_subprocess_new_exec(exec, out, err, exit_code);
    group = cork_subprocess_group_new();
    cork_subprocess_group_add(group, subprocess);
    ei_check(cork_subprocess_group_start(group));
    ei_check(cork_subprocess_group_wait(group));
    cork_subprocess_group_free(group);
    return 0;

error:
    cork_subprocess_group_free(group);
    return -1;
}

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
bz_subprocess_get_output_exec(struct cork_buffer *out_buf,
                              struct cork_buffer *err_buf,
                              bool *successful, struct cork_exec *exec)
{
    int  rc;
    int  exit_code;
    const char  *program = cork_exec_program(exec);
    struct cork_stream_consumer  *out;
    struct cork_stream_consumer  *err;

    out = (out_buf == NULL)? &drop_consumer:
        cork_buffer_to_stream_consumer(out_buf);
    err = (err_buf == NULL)? &drop_consumer:
        cork_buffer_to_stream_consumer(err_buf);
    rc = bz_mocked_exec(exec, out, err, &exit_code);
    if (out != NULL) {
        cork_stream_consumer_free(out);
    }
    if (err != NULL) {
        cork_stream_consumer_free(err);
    }
    rii_check(rc);

    if (successful == NULL) {
        if (CORK_UNLIKELY(exit_code != 0)) {
            bz_subprocess_error("%s failed", program);
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
    struct cork_exec  *exec;
    const char  *program;
    const char  *param;
    program = va_arg(args, const char *);
    assert(program != NULL);
    exec = cork_exec_new(program);
    cork_exec_add_param(exec, program);

    while ((param = va_arg(args, const char *)) != NULL) {
        cork_exec_add_param(exec, param);
    }
    return bz_subprocess_get_output_exec(out_buf, err_buf, successful, exec);
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
bz_subprocess_run_exec(bool verbose, bool *successful, struct cork_exec *exec)
{
    int  rc;
    int  exit_code;
    const char  *program = cork_exec_program(exec);
    struct cork_buffer  out_buf = CORK_BUFFER_INIT();
    struct cork_buffer  err_buf = CORK_BUFFER_INIT();
    struct cork_stream_consumer  *out = NULL;
    struct cork_stream_consumer  *err = NULL;

    cork_buffer_set(&out_buf, "", 0);
    cork_buffer_set(&err_buf, "", 0);
    if (!verbose) {
        out = cork_buffer_to_stream_consumer(&out_buf);
        err = cork_buffer_to_stream_consumer(&err_buf);
    }
    rc = bz_mocked_exec(exec, out, err, &exit_code);
    if (out != NULL) {
        cork_stream_consumer_free(out);
    }
    if (err != NULL) {
        cork_stream_consumer_free(err);
    }
    rii_check(rc);

    if (successful == NULL) {
        if (CORK_UNLIKELY(exit_code != 0)) {
            if (verbose) {
                bz_subprocess_error("%s failed", program);
            } else {
                bz_subprocess_error
                    ("%s failed\n\nstdout:\n%s\nstderr:\n%s",
                     program, (char *) out_buf.buf, (char *) err_buf.buf);
            }
            goto error;
        }
    } else {
        *successful = (exit_code == 0);
    }

    cork_buffer_done(&out_buf);
    cork_buffer_done(&err_buf);
    return 0;

error:
    cork_buffer_done(&out_buf);
    cork_buffer_done(&err_buf);
    return -1;
}

int
bz_subprocess_v_run(bool verbose, bool *successful, va_list args)
{
    struct cork_exec  *exec;
    const char  *program;
    const char  *param;
    program = va_arg(args, const char *);
    assert(program != NULL);
    exec = cork_exec_new(program);
    cork_exec_add_param(exec, program);

    while ((param = va_arg(args, const char *)) != NULL) {
        cork_exec_add_param(exec, param);
    }
    return bz_subprocess_run_exec(verbose, successful, exec);
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


/*-----------------------------------------------------------------------
 * Creating files and directories
 */

struct cork_file *
bz_real__create_dir(struct cork_path *path)
{
    struct cork_file  *file = cork_file_new_from_path(path);
    ei_check(cork_file_mkdir
             (file, 0750, CORK_FILE_RECURSIVE | CORK_FILE_PERMISSIVE));
    return file;

error:
    cork_file_free(file);
    return NULL;
}

int
bz_create_directory(const char *path_string)
{
    struct cork_path  *path = cork_path_new(path_string);
    struct cork_file  *dir;
    clog_debug("Create directory %s", path_string);
    rip_check(dir = bz_mocked_create_dir(path));
    cork_file_free(dir);
    return 0;
}


struct cork_file *
bz_real__create_file(struct cork_path *path, struct cork_buffer *src)
{
    int  fd;
    ssize_t  bytes_written;

    ei_check_posix(fd = creat(cork_path_get(path), 0640));

    bytes_written = write(fd, src->buf, src->size);
    if (CORK_UNLIKELY(bytes_written == -1)) {
        close(fd);
        cork_system_error_set();
        goto error;
    } else if (CORK_UNLIKELY(bytes_written != src->size)) {
        close(fd);
        cork_error_set(CORK_BUILTIN_ERROR, CORK_SYSTEM_ERROR,
                       "Cannot write %zu bytes to %s",
                       src->size, cork_path_get(path));
        goto error;
    }

    ei_check_posix(close(fd));
    return cork_file_new_from_path(path);

error:
    cork_path_free(path);
    return NULL;
}

int
bz_create_file(const char *path_string, struct cork_buffer *src)
{
    struct cork_path  *path = cork_path_new(path_string);
    struct cork_file  *file;
    clog_debug("Create file %s", path_string);
    rip_check(file = bz_mocked_create_file(path, src));
    cork_file_free(file);
    return 0;
}


int
bz_real__file_exists(struct cork_path *path, bool *exists)
{
    int  rc;
    struct cork_file  *file = cork_file_new(cork_path_get(path));
    rc = cork_file_exists(file, exists);
    cork_file_free(file);
    return rc;
}

int
bz_file_exists(const char *path_string, bool *exists)
{
    int  rc;
    struct cork_path  *path = cork_path_new(path_string);
    clog_debug("Check whether %s exists", path_string);
    rc = bz_mocked_file_exists(path, exists);
    cork_path_free(path);
    return rc;
}
