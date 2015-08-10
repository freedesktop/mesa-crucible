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
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "util/log.h"

#include "framework/runner/runner.h"
#include "framework/test/test.h"
#include "framework/test/test_def.h"

#ifdef NDEBUG
#   define ASSERT_MASTER_NOT_RUNNING ((void) 0)
#   define ASSERT_IN_MASTER_PROCESS ((void) 0)
#   define ASSERT_IN_SLAVE_PROCESS ((void) 0)
#else
#   define ASSERT_MASTER_NOT_RUNNING assert(master.pid == 0)

#   define ASSERT_IN_MASTER_PROCESS assert(getpid() == master.pid)

#   define ASSERT_IN_SLAVE_PROCESS \
        do { \
            assert(master.pid > 0); \
            assert(getppid() == master.pid); \
        } while (0)
#endif

typedef union cru_pipe cru_pipe_t;
typedef struct cru_slave cru_slave_t;
typedef struct cru_dispatch_packet cru_dispatch_packet_t;
typedef struct cru_result_packet cru_result_packet_t;

union cru_pipe {
    int fd[2];

    struct {
        int read_fd;
        int write_fd;
    };
};

/// Channel between the master process and a slave process.
struct cru_slave {
    /// \brief PID of the slave process.
    ///
    /// If -1, in the runner's master process before forking.
    /// If 0, in the runner's master process after forking.
    /// If > 0, in the runner's slave process.
    pid_t pid;

    uint32_t num_active_tests;

    cru_pipe_t dispatch_pipe;
    cru_pipe_t result_pipe;
};

struct cru_dispatch_packet {
    const test_def_t *test_def;
};

struct cru_result_packet {
    const test_def_t *test_def;
    test_result_t result;
};


struct cru_master {
    cru_slave_t slaves[1];

    uint32_t num_tests;
    uint32_t num_pass;
    uint32_t num_fail;
    uint32_t num_skip;

    atomic_bool sigint_flag;

#ifndef NDEBUG
    /// This is set only for the duration of runner_run_tests().
    pid_t pid;
#endif

};

runner_opts_t runner_opts = {
    .isolation_mode = RUNNER_ISOLATION_MODE_THREAD,
    .no_fork = false,
    .no_cleanup_phase = false,
    .no_image_dumps = true,
    .use_spir_v = false,
    .use_separate_cleanup_threads = true,
};

static struct cru_master master = {
    .slaves = {
        { .pid = -1 },
    },

#ifndef NDEBUG
    .pid = 0,
#endif
};

static void
set_sigint_handler(sighandler_t handler)
{
    const struct sigaction sa = {
        .sa_handler = handler,
    };

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        loge("test runner failed to set SIGINT handler");
        abort();
    }
}

static bool
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

static void
cru_pipe_finish(cru_pipe_t *p)
{
    for (int i = 0; i < 2; ++i) {
        if (p->fd[i] != -1) {
            close(p->fd[i]);
        }
    }
}

static bool
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

static bool
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

static bool
cru_pipe_atomic_write_n(const cru_pipe_t *p, const void *data, size_t n)
{
    // POSIX.1-2001 says that writes of less than PIPE_BUF bytes are atomic.
    // We assume that, if all writes to a pipe are atomic, then all reads will
    // be atomic also.
    assert(n < PIPE_BUF);

    return write(p->write_fd, data, n) == n;
}

static bool
cru_pipe_atomic_read_n(const cru_pipe_t *p, void *data, size_t n)
{
    assert(n < PIPE_BUF);

    return read(p->read_fd, data, n) == n;
}

#define cru_pipe_atomic_write(p, data) \
    cru_pipe_atomic_write_n((p), (data), sizeof(*(data)))

#define cru_pipe_atomic_read(p, data) \
    cru_pipe_atomic_read_n((p), (data), sizeof(*(data)))

/// Return false if the pipe is empty or has errors.
static void
master_report_result(const test_def_t *def, test_result_t result)
{
    ASSERT_IN_MASTER_PROCESS;

    log_tag(test_result_to_string(result),
                "%s", def->name);
    fflush(stdout);

    switch (result) {
    case TEST_RESULT_PASS: master.num_pass++; break;
    case TEST_RESULT_FAIL: master.num_fail++; break;
    case TEST_RESULT_SKIP: master.num_skip++; break;
    }
}

/// Return false on failure.
static bool
master_send_dispatch(cru_slave_t *slave, const test_def_t *def)
{
    ASSERT_IN_MASTER_PROCESS;

    bool result = false;
    const cru_dispatch_packet_t pk = { .test_def = def };
    const struct sigaction ignore_sa = { .sa_handler = SIG_IGN };
    struct sigaction old_sa;
    int err;

    // If the slave process died, then writing to its dispatch pipe will emit
    // SIGPIPE. Ignore it, because the master should never die.
    err = sigaction(SIGPIPE, &ignore_sa, &old_sa);
    if (err == -1) {
        loge("test runner failed to disable SIGPIPE");
        abort();
    }

    if (!cru_pipe_atomic_write(&slave->dispatch_pipe, &pk))
        goto cleanup;

    result = true;
    slave->num_active_tests += 1;

cleanup:
    err = sigaction(SIGPIPE, &old_sa, NULL);
    if (err == -1) {
        loge("test runner failed to re-enable SIGPIPE");
        abort();
    }

    return result;
}

/// Return NULL if the pipe is empty or has errors.
static const test_def_t *
slave_recv_dispatch(const cru_slave_t *slave)
{
    ASSERT_IN_SLAVE_PROCESS;

    cru_dispatch_packet_t pk;

    if (!cru_pipe_atomic_read(&slave->dispatch_pipe, &pk))
        return false;

    return pk.test_def;
}

/// Return false if the pipe is empty or has errors.
static bool
master_recv_result(cru_slave_t *slave)
{
    ASSERT_IN_MASTER_PROCESS;

    cru_result_packet_t pk;

    if (!cru_pipe_atomic_read(&slave->result_pipe, &pk))
        return false;

    slave->num_active_tests -= 1;
    master_report_result(pk.test_def, pk.result);

    return true;
}

/// Return false on failure.
static bool
slave_send_result(const cru_slave_t *slave,
                  const test_def_t *def, test_result_t result)
{
    ASSERT_IN_SLAVE_PROCESS;

    const cru_result_packet_t pk = {
        .test_def = def,
        .result = result,
    };

    return cru_pipe_atomic_write(&slave->result_pipe, &pk);
}

static test_result_t
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

static void
slave_loop(const cru_slave_t *slave)
{
    ASSERT_IN_SLAVE_PROCESS;

    const test_def_t *def;

    while (true) {
        test_result_t result;

        def = slave_recv_dispatch(slave);
        if (!def)
            return;

        result = run_test_def(def);
        slave_send_result(slave, def, result);
    }
}

static void
master_drain_result_pipe(cru_slave_t *slave)
{
    ASSERT_IN_MASTER_PROCESS;

    while (slave->num_active_tests > 0) {
        if (!master_recv_result(slave)) {
            break;
        }
    }
}

static bool
master_init_slave(cru_slave_t *slave)
{
    ASSERT_IN_MASTER_PROCESS;

    assert(slave->pid == -1);

    slave->num_active_tests = 0;

    if (!cru_pipe_init(&slave->dispatch_pipe))
        return false;

    if (!cru_pipe_init(&slave->result_pipe))
        return false;

    slave->pid = fork();

    if (slave->pid == -1) {
        loge("test runner failed to fork slave process");
        return false;
    }

    if (slave->pid == 0) {
        set_sigint_handler(SIG_DFL);

        if (!cru_pipe_become_reader(&slave->dispatch_pipe))
            exit(EXIT_FAILURE);

        if (!cru_pipe_become_writer(&slave->result_pipe))
            exit(EXIT_FAILURE);

        slave_loop(slave);

        exit(EXIT_SUCCESS);
    }

    if (!cru_pipe_become_writer(&slave->dispatch_pipe))
        return false;

    if (!cru_pipe_become_reader(&slave->result_pipe))
        return false;

    return true;
}

static void
master_cleanup_slave(cru_slave_t *slave)
{
    ASSERT_IN_MASTER_PROCESS;

    if (slave->pid == -1)
        return;

    master_drain_result_pipe(slave);

    slave->num_active_tests = 0;
    cru_pipe_finish(&slave->dispatch_pipe);
    cru_pipe_finish(&slave->result_pipe);

    if (waitpid(slave->pid, /*status*/ NULL, /*flags*/ 0) == -1) {
        loge("runner failed to wait for slave process");
    }

    slave->pid = -1;
}

static void
master_kill_slaves(void)
{
    ASSERT_IN_MASTER_PROCESS;

    int err;

    for (uint32_t i = 0; i < ARRAY_LENGTH(master.slaves); ++i) {
        cru_slave_t *slave = &master.slaves[i];

        if (slave->pid == -1)
            continue;

        err = kill(slave->pid, SIGINT);
        if (err) {
            loge("runner failed to kill child process %d", slave->pid);
            abort();
        }

        master_cleanup_slave(slave);
    }
}

static void
master_handle_sigint(int sig)
{
    ASSERT_IN_MASTER_PROCESS;

    assert(sig == SIGINT);
    atomic_store(&master.sigint_flag, true);
}

/// Dispatch tests to slave processes.
static void
master_loop_with_forking(void)
{
    ASSERT_IN_MASTER_PROCESS;

    const test_def_t *def;
    cru_slave_t *slave = &master.slaves[0];

    // Dispatch each test to the current slave process. Interleave the
    // dispatching and result collection.
    cru_foreach_test_def(def) {
        if (slave->pid == -1 && !master_init_slave(slave)) {
            return;
        }

        // SIGINT kills all running tests. A second SIGINT, if received before
        // the runner resumes running tests, kills the testrun and prints the
        // result summary.
        if (atomic_exchange(&master.sigint_flag, false)) {
            master_kill_slaves();

            // Give the user a short window in which to send SIGINT again
            // before tests resume running.
            nanosleep(&(struct timespec) { .tv_nsec = 500000000 }, NULL);

            if (atomic_exchange(&master.sigint_flag, false)) {
                return;
            }
        }

        if (!def->priv.enable)
            continue;

        if (def->skip) {
            master_report_result(def, TEST_RESULT_SKIP);
            continue;
        }

        log_tag("start", "%s", def->name);

        if (!master_send_dispatch(slave, def)) {
            // The slave is probably be dead. Reap the zombie and spawn a new
            // one.  Re-dispatch this test to the new slave.
            master_drain_result_pipe(slave);
            master_cleanup_slave(slave);

            if (!master_init_slave(slave)) {
                // Can't recover.
                return;
            }

            if (!master_send_dispatch(slave, def)) {
                // We twice failed to dispatch this test. Give up.
                return;
            }
        }


        switch (runner_opts.isolation_mode) {
        case RUNNER_ISOLATION_MODE_PROCESS:
            master_send_dispatch(slave, NULL);
            master_cleanup_slave(slave);
            break;
        case RUNNER_ISOLATION_MODE_THREAD:
            master_drain_result_pipe(slave);
            break;
        }
    }

    if (slave->pid != -1) {
        master_send_dispatch(slave, NULL);
        master_cleanup_slave(slave);
    }
}

/// Run all tests in the master process.
static void
master_loop_no_forking(void)
{
    ASSERT_IN_MASTER_PROCESS;

    const test_def_t *def;

    cru_foreach_test_def(def) {
        test_result_t result;

        if (!def->priv.enable)
            continue;

        if (def->skip) {
            master_report_result(def, TEST_RESULT_SKIP);
            continue;
        }

        log_tag("start", "%s", def->name);
        result = run_test_def(def);
        master_report_result(def, result);
    }
}

static void
master_loop(void)
{
    ASSERT_IN_MASTER_PROCESS;

    if (runner_opts.no_fork) {
        master_loop_no_forking();
    } else {
        set_sigint_handler(master_handle_sigint);
        master_loop_with_forking();
        set_sigint_handler(SIG_DFL);
    }
}


/// Return true if and only all tests pass or skip.
bool
runner_run_tests(void)
{
    ASSERT_MASTER_NOT_RUNNING;

    if (runner_opts.no_fork
        && runner_opts.isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        loge("invalid options for test runner");
        return false;
    }

#ifndef NDEBUG
    master.pid = getpid();
#endif

    log_align_tags(true);
    logi("running %u tests", master.num_tests);
    logi("================================");

    master_loop();

    uint32_t num_ran = master.num_pass + master.num_fail + master.num_skip;
    uint32_t num_lost = master.num_tests - num_ran;

    // A big, and perhaps unneeded, hammer.
    fflush(stdout);
    fflush(stderr);

    logi("================================");
    logi("ran %u tests", master.num_tests);
    logi("pass %u", master.num_pass);
    logi("fail %u", master.num_fail);
    logi("skip %u", master.num_skip);
    logi("lost %u", num_lost);

#ifndef NDEBUG
    master.pid = 0;
#endif

    return master.num_pass + master.num_skip == master.num_tests;
}

static inline void
enable_test_def(test_def_t *def)
{
    ASSERT_MASTER_NOT_RUNNING;

    if (!def->priv.enable)
        ++master.num_tests;

    def->priv.enable = true;
}

void
runner_enable_all_nonexample_tests(void)
{
    ASSERT_MASTER_NOT_RUNNING;

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
    ASSERT_MASTER_NOT_RUNNING;

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
