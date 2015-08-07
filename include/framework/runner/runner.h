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

typedef enum runner_isolation_mode runner_isolation_mode_t;

enum runner_isolation_mode {
    /// The runner will isolate each test in a separate process.
    RUNNER_ISOLATION_MODE_PROCESS,

    /// The runner will isolate each test in a separate thread.
    RUNNER_ISOLATION_MODE_THREAD,
};

extern runner_isolation_mode_t runner_isolation_mode;
extern bool runner_do_forking;
extern bool runner_do_cleanup_phase;
extern bool runner_do_image_dumps;
extern bool runner_use_spir_v;
extern bool runner_use_separate_cleanup_threads;

void runner_enable_cleanup(bool b);
void runner_enable_image_dumps(bool b);
void runner_enable_spir_v(bool b);
void runner_enable_all_nonexample_tests(void);
void runner_enable_matching_tests(const cru_cstr_vec_t *testname_globs);
bool runner_run_tests(void);
