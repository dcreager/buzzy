/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013-2014, RedJack, LLC.
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
    subprocess = cork_subprocess_new_exec(exec, out, err, exit_code);
    ei_check(cork_subprocess_start(subprocess));
    ei_check(cork_subprocess_wait(subprocess));
    cork_subprocess_free(subprocess);
    return 0;

error:
    cork_subprocess_free(subprocess);
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
bz_real__create_dir(struct cork_path *path, cork_file_mode mode)
{
    struct cork_file  *file = cork_file_new_from_path(path);
    ei_check(cork_file_mkdir
             (file, mode, CORK_FILE_RECURSIVE | CORK_FILE_PERMISSIVE));
    return file;

error:
    cork_file_free(file);
    return NULL;
}

int
bz_create_directory(const char *path_string, cork_file_mode mode)
{
    struct cork_path  *path = cork_path_new(path_string);
    struct cork_file  *dir;
    clog_debug("Create directory %s", path_string);
    rip_check(dir = bz_mocked_create_dir(path, mode));
    cork_file_free(dir);
    return 0;
}


struct cork_file *
bz_real__create_file(struct cork_path *path, struct cork_buffer *src,
                     cork_file_mode mode)
{
    int  fd;
    ssize_t  bytes_written;

    ei_check_posix(fd = creat(cork_path_get(path), mode));

    bytes_written = write(fd, src->buf, src->size);
    if (CORK_UNLIKELY(bytes_written == -1)) {
        cork_system_error_set();
        close(fd);
        goto error;
    } else if (CORK_UNLIKELY(bytes_written != src->size)) {
        cork_error_set_printf
            (ENOSPC, "Cannot write %zu bytes to %s",
             src->size, cork_path_get(path));
        close(fd);
        goto error;
    }

    ei_check_posix(close(fd));
    return cork_file_new_from_path(path);

error:
    cork_path_free(path);
    return NULL;
}

int
bz_create_file(const char *path_string, struct cork_buffer *src,
               cork_file_mode mode)
{
    struct cork_path  *path = cork_path_new(path_string);
    struct cork_file  *file;
    clog_debug("Create file %s", path_string);
    rip_check(file = bz_mocked_create_file(path, src, mode));
    cork_file_free(file);
    return 0;
}


#define COPY_BUF_SIZE  65536
static char  COPY_BUF[COPY_BUF_SIZE];

struct cork_file *
bz_real__copy_file(struct cork_path *dest, struct cork_path *src, int mode)
{
    int  in_fd;
    int  out_fd;
    ssize_t  bytes_read;
    ssize_t  bytes_written;

    ei_check_posix(in_fd = open(cork_path_get(src), O_RDONLY));
    ei_check_posix(out_fd = creat(cork_path_get(dest), mode));

    while ((bytes_read = read(in_fd, COPY_BUF, COPY_BUF_SIZE)) > 0) {
        bytes_written = write(out_fd, COPY_BUF, bytes_read);
        if (CORK_UNLIKELY(bytes_written == -1)) {
            cork_system_error_set();
            close(in_fd);
            close(out_fd);
            goto error;
        } else if (CORK_UNLIKELY(bytes_written != bytes_read)) {
            cork_error_set_printf
                (ENOSPC, "Cannot write %zu bytes to %s",
                 bytes_read, cork_path_get(dest));
            close(in_fd);
            close(out_fd);
            goto error;
        }
    }

    if (bytes_read < 0) {
        cork_system_error_set();
        close(in_fd);
        close(out_fd);
        goto error;
    }

    ei_check_posix(close(in_fd));
    ei_check_posix(close(out_fd));
    cork_path_free(src);
    return cork_file_new_from_path(dest);

error:
    cork_path_free(dest);
    cork_path_free(src);
    return NULL;
}

int
bz_copy_file(const char *dest_string, const char *src_string, int mode)
{
    struct cork_path  *dest = cork_path_new(dest_string);
    struct cork_path  *src = cork_path_new(src_string);
    struct cork_file  *file;
    clog_debug("Copy file %s to %s", src_string, dest_string);
    rip_check(file = bz_mocked_copy_file(dest, src, mode));
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


int
bz_real__load_file(struct cork_path *path, struct cork_buffer *dest)
{
    struct cork_stream_consumer  *consumer =
        cork_buffer_to_stream_consumer(dest);
    ei_check(cork_consume_file_from_path
             (consumer, cork_path_get(path), O_RDONLY));
    cork_stream_consumer_free(consumer);
    return 0;

error:
    cork_stream_consumer_free(consumer);
    return -1;
}

int
bz_load_file(const char *path_string, struct cork_buffer *dest)
{
    int  rc;
    struct cork_path  *path = cork_path_new(path_string);
    clog_debug("Read contents of %s", path_string);
    rc = bz_mocked_load_file(path, dest);
    cork_path_free(path);
    return rc;
}


int
bz_real__walk_directory(const char *path, struct cork_dir_walker *walker)
{
    return cork_walk_directory(path, walker);
}

int
bz_walk_directory(const char *path_string, struct cork_dir_walker *walker)
{
    clog_debug("Walk contents of %s", path_string);
    return bz_mocked_walk_directory(path_string, walker);
}
