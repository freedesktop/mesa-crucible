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
#include <stdint.h>

#include "util/cru_vec.h"

typedef enum runner_isolation_mode runner_isolation_mode_t;
typedef struct runner_opts runner_opts_t;

enum runner_isolation_mode {
    /// The runner will isolate each test in a separate process.
    RUNNER_ISOLATION_MODE_PROCESS,

    /// The runner will isolate each test in a separate thread.
    RUNNER_ISOLATION_MODE_THREAD,
};

struct runner_opts {
    /// Number of tests to run simultaneously. Similar to GNU Make's -j
    /// option.
    uint32_t jobs;

    runner_isolation_mode_t isolation_mode;
    bool no_fork;
    bool no_cleanup_phase;
    bool no_image_dumps;
    bool use_spir_v;
    bool use_separate_cleanup_threads;

    /// The runner will write JUnit XML to this path, if not NULL.
    const char *junit_xml_filepath;
};

bool runner_init(runner_opts_t *opts);
void runner_enable_matching_tests(const cru_cstr_vec_t *testname_globs);
bool runner_run_tests(void);
