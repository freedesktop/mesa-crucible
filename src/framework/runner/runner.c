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

/// \file
/// \brief Crucible Test Runner
///
/// The runner consists of two processes: master and slave. The master forks
/// the slave. The tests execute in the slave process. The master collects the
/// test results and prints their summary. The separation ensures that test
/// results and summary are printed even when a test crashes its process.

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/log.h"

#include "framework/runner/runner.h"
#include "framework/test/test.h"
#include "framework/test/test_def.h"

#include "master.h"
#include "runner.h"

static uint32_t runner_num_tests;
static bool runner_is_init = false;
runner_opts_t runner_opts = {0};

#define ASSERT_RUNNER_IS_INIT \
    do { \
        if (!runner_is_init) { \
            log_internal_error("%s: runner is not initialized", __func__); \
        } \
    } while (0)

bool
runner_init(runner_opts_t *opts)
{
    if (runner_is_init) {
        log_internal_error("cannot initialize runner twice");
        return false;
    }

    if (opts->no_fork
        && opts->isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        log_finishme("support no_fork with RUNNER_ISOLATION_MODE_THREAD");
        return false;
    }

    if (opts->jobs > 1
        && opts->isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        log_finishme("support jobs > 1 with RUNNER_ISOLATION_MODE_THREAD");
        return false;
    }

    if (opts->jobs > 1 && opts->no_fork) {
        log_finishme("support jobs > 1 with no_fork");
        return false;
    }

    runner_opts = *opts;
    runner_is_init = true;

    return true;
}

test_result_t
run_test_def(const test_def_t *def)
{
    ASSERT_RUNNER_IS_INIT;

    test_t *test;
    test_result_t result;

    assert(def->priv.enable);

    test = test_create(.def = def,
                       .enable_dump = !runner_opts.no_image_dumps,
                       .enable_cleanup_phase = !runner_opts.no_cleanup_phase,
                       .enable_spir_v = runner_opts.use_spir_v,
                       .enable_separate_cleanup_thread =
                            runner_opts.use_separate_cleanup_threads);
    if (!test)
        return TEST_RESULT_FAIL;

    test_start(test);
    test_wait(test);
    result = test_get_result(test);
    test_destroy(test);

    return result;
}

/// Return true if and only all tests pass or skip.
bool
runner_run_tests(void)
{
    ASSERT_RUNNER_IS_INIT;

    return master_run(runner_num_tests);
}

void
runner_enable_matching_tests(const cru_cstr_vec_t *testname_globs)
{
    ASSERT_RUNNER_IS_INIT;

    test_def_t *def;
    char **glob;

    bool include_all = testname_globs->len == 0;

    cru_foreach_test_def(def) {
        bool enable = false;

        if (include_all) {
            enable = test_def_match(def, "*");
        }

        cru_vec_foreach(glob, testname_globs) {
            enable = test_def_match(def, *glob);
        }

        if (enable) {
            def->priv.enable = true;
            ++runner_num_tests;
        }
    }
}
