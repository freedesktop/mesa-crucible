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

#include <stdbool.h>

#include "util/cru_vec.h"

enum test_isolation {
    /// The runner will start each test in a separate process.
    CRU_TEST_ISOLATION_PROCESS,

    /// The runner will start each test in a separate thread.
    CRU_TEST_ISOLATION_THREAD,
};

extern enum test_isolation cru_runner_test_isolation;
extern bool cru_runner_do_forking;
extern bool cru_runner_do_cleanup_phase;
extern bool cru_runner_do_image_dumps;
extern bool cru_runner_use_spir_v;
extern bool cru_runner_use_separate_cleanup_threads;

void cru_runner_enable_cleanup(bool b);
void cru_runner_enable_image_dumps(bool b);
void cru_runner_enable_spir_v(bool b);
void cru_runner_enable_all_nonexample_tests(void);
void cru_runner_enable_matching_tests(const cru_cstr_vec_t *testname_globs);
bool cru_runner_run_tests(void);
