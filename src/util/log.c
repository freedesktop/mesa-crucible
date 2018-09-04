// Copyright 2015 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice (including the next
// paragraph) shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "framework/runner/runner.h"
#include "framework/test/test.h"
#include "util/log.h"

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool log_has_aligned_tags = false;
static bool log_should_print_pids = false;

void
log_tag(const char *tag, pid_t pid, const char *format, ...)
{
    va_list va;

    va_start(va, format);
    log_tag_v(tag, pid, format, va);
    va_end(va);
}

void
log_abort(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    log_abort_v(format, va);
    va_end(va);
}

void
loge(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    loge_v(format, va);
    va_end(va);
}

void
logw(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    logw_v(format, va);
    va_end(va);
}

void
logi(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    logi_v(format, va);
    va_end(va);
}

void
logd(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    logd_v(format, va);
    va_end(va);
}

void
log_tag_v(const char *tag, pid_t pid, const char *format, va_list va)
{
    pthread_mutex_lock(&log_mutex);

    // Tags are aligned to 7 because that's wide enough for "warning".
    // PID fields are aligned to 6 because that's enough for "master" and
    // any 16-bit unsigned value.
    if (log_should_print_pids) {
        if (pid == 0) {
            if (log_has_aligned_tags) {
                printf("crucible [master]: %-7s: ", tag);
            } else {
                printf("crucible [master]: %s: ", tag);
            }
        } else {
            int ipid = pid; // printf likes standard data types better
            if (log_has_aligned_tags) {
                printf("crucible [%-6d]: %-7s: ", ipid, tag);
            } else {
                printf("crucible [%-6d]: %s: ", ipid, tag);
            }
        }
    } else {
        if (log_has_aligned_tags) {
            printf("crucible: %-7s: ", tag);
        } else {
            printf("crucible: %s: ", tag);
        }
    }

    if (test_is_current()) {
        printf("%s: ", t_name);
    }

    vprintf(format, va);
    printf("\n");

    // Don't buffer the log messages. If a GPU hang occurs, buffering makes it
    // difficult to determine which test hung the GPU.
    fflush(stdout);

    pthread_mutex_unlock(&log_mutex);
}

void
log_abort_v(const char *format, va_list va)
{
    log_tag_v("abort", 0, format, va);
    abort();
}

void
loge_v(const char *format, va_list va)
{
    log_tag_v("error", 0, format, va);
}

void
logw_v(const char *format, va_list va)
{
    log_tag_v("warning", 0, format, va);
}

void
logi_v(const char *format, va_list va)
{
    log_tag_v("info", 0, format, va);
}

void
logd_v(const char *format, va_list va)
{
    log_tag_v("debug", 0, format, va);
}

void
__log_finishme(const char *file, int line, const char *format, ...)
{
    va_list va;

    pthread_mutex_lock(&log_mutex);

    va_start(va, format);
    printf("FINISHME: %s:%d: ", file, line);
    vprintf(format, va);
    printf("\n");
    va_end(va);

    pthread_mutex_unlock(&log_mutex);
}

void
log_internal_error_loc(const char *file, int line,
                           const char *format, ...)
{
    va_list va;

    va_start(va, format);
    log_internal_error_loc_v(file, line, format, va);
    va_end(va);
}

void
log_internal_error_loc_v(const char *file, int line,
                             const char *format, va_list va)
{
    pthread_mutex_lock(&log_mutex);

    printf("internal error: %s:%d: ", file, line);
    vprintf(format, va);
    printf("\n");

    abort();
}

void
log_print_pids(bool enable)
{
    log_should_print_pids = enable;
}

void
log_align_tags(bool enable)
{
    log_has_aligned_tags = enable;
}
