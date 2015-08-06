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

#include "framework/test/cru_test.h"
#include "util/cru_log.h"

static pthread_mutex_t cru_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool align_tags = false;

void
cru_log_tag(const char *tag, const char *format, ...)
{
    va_list va;

    va_start(va, format);
    cru_log_tag_v(tag, format, va);
    va_end(va);
}

void
cru_loge(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    cru_loge_v(format, va);
    va_end(va);
}

void
cru_logw(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    cru_logw_v(format, va);
    va_end(va);
}

void
cru_logi(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    cru_logi_v(format, va);
    va_end(va);
}

void
cru_logd(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    cru_logd_v(format, va);
    va_end(va);
}

void
cru_log_tag_v(const char *tag, const char *format, va_list va)
{
    pthread_mutex_lock(&cru_log_mutex);

    if (align_tags) {
        // Align to 7 because that's wide enough for "warning".
        printf("crucible: %-7s: ", tag);
    } else {
        printf("crucible: %s: ", tag);
    }

    if (cru_test_is_current()) {
        printf("%s: ", t_name);
    }

    vprintf(format, va);
    printf("\n");

    pthread_mutex_unlock(&cru_log_mutex);
}

void
cru_loge_v(const char *format, va_list va)
{
    cru_log_tag_v("error", format, va);
}

void
cru_logw_v(const char *format, va_list va)
{
    cru_log_tag_v("warning", format, va);
}

void
cru_logi_v(const char *format, va_list va)
{
    cru_log_tag_v("info", format, va);
}

void
cru_logd_v(const char *format, va_list va)
{
    cru_log_tag_v("debug", format, va);
}

void
__cru_finishme(const char *file, int line, const char *format, ...)
{
    va_list va;

    pthread_mutex_lock(&cru_log_mutex);

    va_start(va, format);
    printf("FINISHME: %s:%d: ", file, line);
    vprintf(format, va);
    printf("\n");
    va_end(va);

    pthread_mutex_unlock(&cru_log_mutex);
}

void
cru_log_internal_error_loc(const char *file, int line,
                           const char *format, ...)
{
    va_list va;

    va_start(va, format);
    cru_log_internal_error_loc_v(file, line, format, va);
    va_end(va);
}

void
cru_log_internal_error_loc_v(const char *file, int line,
                             const char *format, va_list va)
{
    pthread_mutex_lock(&cru_log_mutex);

    printf("internal error: %s:%d: ", file, line);
    vprintf(format, va);
    printf("\n");

    abort();
}

void
cru_log_align_tags(bool enable)
{
    align_tags = enable;
}
