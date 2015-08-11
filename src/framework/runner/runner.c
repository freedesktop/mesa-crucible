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
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>

#include "util/log.h"

#include "framework/runner/runner.h"
#include "framework/test/test.h"
#include "framework/test/test_def.h"

#include "master.h"
#include "runner.h"

static uint32_t runner_num_tests;

runner_opts_t runner_opts = {
    .jobs = 1,
    .isolation_mode = RUNNER_ISOLATION_MODE_THREAD,
    .no_fork = false,
    .no_cleanup_phase = false,
    .no_image_dumps = true,
    .use_spir_v = false,
    .use_separate_cleanup_threads = true,
};

bool
cru_pipe_init(cru_pipe_t *p)
{
    int err;

    err = pipe2(p->fd, O_CLOEXEC);
    if (err) {
        loge("failed to create pipe");
        return false;
    }

    return true;
}

void
cru_pipe_finish(cru_pipe_t *p)
{
    for (int i = 0; i < 2; ++i) {
        if (p->fd[i] != -1) {
            close(p->fd[i]);
        }
    }
}

bool
cru_pipe_become_reader(cru_pipe_t *p)
{
    int err;

    assert(p->read_fd != -1);
    assert(p->write_fd != -1);

    err = close(p->write_fd);
    if (err == -1) {
        loge("runner failed to close pipe's write fd");
        return false;
    }

    p->write_fd = -1;

    return true;
}

bool
cru_pipe_become_writer(cru_pipe_t *p)
{
    int err;

    assert(p->read_fd != -1);
    assert(p->write_fd != -1);

    err = close(p->read_fd);
    if (err == -1) {
        loge("runner failed to close pipe's read fd");
        return false;
    }

    p->read_fd = -1;

    return true;
}

bool
cru_pipe_atomic_write_n(const cru_pipe_t *p, const void *data, size_t n)
{
    // POSIX.1-2001 says that writes of less than PIPE_BUF bytes are atomic.
    // We assume that, if all writes to a pipe are atomic, then all reads will
    // be atomic also.
    assert(n < PIPE_BUF);

    return write(p->write_fd, data, n) == n;
}

bool
cru_pipe_atomic_read_n(const cru_pipe_t *p, void *data, size_t n)
{
    assert(n < PIPE_BUF);

    return read(p->read_fd, data, n) == n;
}

test_result_t
run_test_def(const test_def_t *def)
{
    test_t *test;
    test_result_t result;

    assert(def->priv.enable);

    test = test_create(def);
    if (!test)
        return TEST_RESULT_FAIL;

    if (!runner_opts.no_image_dumps)
        test_enable_dump(test);

    if (runner_opts.no_cleanup_phase)
        test_disable_cleanup(test);

    if (runner_opts.use_spir_v)
        test_enable_spir_v(test);

    if (!runner_opts.use_separate_cleanup_threads)
        test_disable_separate_cleanup_thread(test);

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
    if (runner_opts.no_fork &&
            runner_opts.isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        log_finishme("support no_fork with RUNNER_ISOLATION_MODE_THREAD");
        return false;
    }

    if (runner_opts.jobs > 1 &&
            runner_opts.isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        log_finishme("support jobs > 1 with RUNNER_ISOLATION_MODE_THREAD");
        return false;
    }

    if (runner_opts.jobs > 1 && runner_opts.no_fork) {
        log_finishme("support jobs > 1 with no_fork");
        return false;
    }

    return master_run(runner_num_tests);
}

static inline void
enable_test_def(test_def_t *def)
{
    if (!def->priv.enable)
        ++runner_num_tests;

    def->priv.enable = true;
}

void
runner_enable_all_nonexample_tests(void)
{
    test_def_t *def;

    cru_foreach_test_def(def) {
        if (!test_def_match(def, "example.*")) {
            enable_test_def(def);
        }
    }
}

void
runner_enable_matching_tests(const cru_cstr_vec_t *testname_globs)
{
    test_def_t *def;
    char **glob;

    cru_foreach_test_def(def) {
        cru_vec_foreach(glob, testname_globs) {
            if (test_def_match(def, *glob)) {
                enable_test_def(def);
            }
        }
    }
}
