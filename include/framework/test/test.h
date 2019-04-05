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

#pragma once

#include "tapi/t.h"

typedef struct test test_t;
typedef struct test_create_info test_create_info_t;

struct test_create_info {
    const test_def_t *def;

    bool enable_dump;
    bool enable_cleanup_phase;
    bool enable_separate_cleanup_thread;
    bool enable_spir_v;
    bool enable_bootstrap;
    int device_id;
    uint32_t queue_family_index;

    uint32_t bootstrap_image_width;
    uint32_t bootstrap_image_height;
};

#ifdef DOXYGEN
test_t *test_create(const test_create_info_t *va_args info);
#else
#define test_create(...) \
    test_create_s(&(test_create_info_t) { 0, ##__VA_ARGS__ })
#endif

const char *test_result_to_string(test_result_t result);

/// Like test_create(), but with explicit info structure.
test_t *test_create_s(const test_create_info_t *info);
void test_destroy(test_t *test);
bool test_is_current(void);

void test_start(test_t *test);
void test_wait(test_t *test);
test_result_t test_get_result(test_t *test);
