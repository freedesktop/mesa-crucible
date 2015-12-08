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
/// \brief The runner's master process

#include <limits.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "framework/test/test.h"
#include "framework/test/test_def.h"
#include "util/log.h"

#include "runner.h"
#include "master.h"
#include "slave.h"

typedef struct slave slave_t;
typedef struct slave_pipe slave_pipe_t;

struct slave_pipe {
    union {
        int fd[2];

        struct {
            int read_fd;
            int write_fd;
        };
    };

    /// A reference to the pipe's containing slave. Used by epoll handlers.
    slave_t *slave;
};

/// \brief A slave process's proxy in the master process.
///
/// The struct is valid if and only if slave::pid != 0.
struct slave {
    /// Cases:
    ///   * if 0: the proxy is not connected to a process,
    ///           and the struct is invalid
    ///   * if > 0: the master forked the slave and has not yet reaped it
    pid_t pid;

    struct {
        uint32_t len;
        const test_def_t *data[256];
    } tests;

    slave_pipe_t dispatch_pipe;
    slave_pipe_t result_pipe;

    /// Each slave process's stdout and stderr are connected to a pipe in the
    /// master process. This prevents concurrently running slaves from
    /// corrupting the master's stdout and stderr with interleaved output.
    slave_pipe_t stdout_pipe;
    slave_pipe_t stderr_pipe;

    /// Number of tests dispatched to the slave over its lifetime.
    uint32_t lifetime_test_count;

    bool recvd_sentinel;
    bool is_dead;
};

static struct master {
    atomic_bool sigint_flag;
    atomic_bool goto_next_phase;

    int epoll_fd;
    int signal_fd;

    /// Count of currently dispatched tests.
    uint32_t cur_dispatched_tests;

    /// Maximum allowed count of currently dispatched tests.
    uint32_t max_dispatched_tests;

    uint32_t num_tests;
    uint32_t num_pass;
    uint32_t num_fail;
    uint32_t num_skip;
    uint32_t num_lost;

    uint32_t num_slaves;
    slave_t slaves[64];

} master = {
    .epoll_fd = -1,
    .signal_fd = -1,
};

static void master_print_header(void);
static void master_enter_dispatch_phase(void);
static void master_enter_cleanup_phase(void);
static void master_print_summary(void);

static void master_dispatch_loop_no_fork(void);
static void master_dispatch_loop_with_fork(void);

static void master_dispatch_test(const test_def_t *def);
static slave_t * master_get_open_slave(void);
static slave_t * master_get_new_slave(void);
static slave_t * master_find_unborn_slave(void);
static void master_cleanup_dead_slave(slave_t *slave);

static void master_collect_result(int timeout_ms);

static void master_report_result(const test_def_t *def, test_result_t result);
static bool master_send_packet(slave_t *slave, const dispatch_packet_t *pk);

static void master_kill_all_slaves(void);

static void master_init_epoll(void);
static void master_finish_epoll(void);
static bool master_epoll_add_slave_pipe(slave_pipe_t *pipe, int rw);

static void master_handle_epoll_event(const struct epoll_event *event);
static void master_handle_pipe_event(const struct epoll_event *event);
static void master_handle_signal_event(const struct epoll_event *event);
static void master_handle_sigchld(void);
static void master_handle_sigint(int sig);
static void master_yield_to_sigint(void);

static bool slave_is_open(const slave_t *slave);
static int32_t slave_find_test(slave_t *slave, const test_def_t *def);
static bool slave_insert_test(slave_t *slave, const test_def_t *def);
static void slave_rm_test(slave_t *slave, const test_def_t *def);

static bool slave_start_test(slave_t *slave, const test_def_t *def);
static void slave_send_sentinel(slave_t *slave);
static void slave_drain_result_pipe(slave_t *slave);

static bool slave_pipe_init(slave_t *slave, slave_pipe_t *pipe);
static void slave_pipe_finish(slave_pipe_t *pipe);
static bool slave_pipe_become_reader(slave_pipe_t *pipe);
static bool slave_pipe_become_writer(slave_pipe_t *pipe);
static void slave_pipe_drain_to_fd(slave_pipe_t *pipe, int fd);

static slave_t *find_slave_by_pid(pid_t pid);

#define master_for_each_slave_slot(s)                                       \
    for ((s) = master.slaves;                                               \
         (s) < master.slaves + ARRAY_LENGTH(master.slaves);                 \
         (s) = (slave_t *) (s) + 1)

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

bool
master_run(uint32_t num_tests)
{
    master.num_tests = num_tests;
    master.max_dispatched_tests = CLAMP(runner_opts.jobs,
                                        1, ARRAY_LENGTH(master.slaves));

    master_init_epoll();
    set_sigint_handler(master_handle_sigint);

    master_print_header();
    master_enter_dispatch_phase();
    master_enter_cleanup_phase();
    master_print_summary();

    set_sigint_handler(SIG_DFL);
    master_finish_epoll();

    return master.num_pass + master.num_skip == master.num_tests;
}

static void
master_print_header(void)
{
    log_align_tags(true);
    logi("running %u tests", master.num_tests);
    logi("================================");

}

static void
master_print_summary(void)
{
    // A big, and perhaps unneeded, hammer.
    fflush(stdout);
    fflush(stderr);

    logi("================================");
    logi("ran %u tests", master.num_tests);
    logi("pass %u", master.num_pass);
    logi("fail %u", master.num_fail);
    logi("skip %u", master.num_skip);
    logi("lost %u", master.num_lost);
}

static void
master_enter_dispatch_phase(void)
{
    if (runner_opts.no_fork) {
        master_dispatch_loop_no_fork();
    } else {
        master_dispatch_loop_with_fork();
    }
}

static void
master_enter_cleanup_phase(void)
{
    slave_t *slave;

    if (runner_opts.no_fork)
        return;

    // Tell each slave that it will receive no more tests.
    master_for_each_slave_slot(slave) {
        if (!slave->pid)
            continue;

        slave_send_sentinel(slave);
        if (master.goto_next_phase)
            return;
    }

    while (master.num_slaves > 0) {
        master_collect_result(-1);
        if (master.goto_next_phase)
            return;
    }
}

/// Run all tests in the master process.
static void
master_dispatch_loop_no_fork(void)
{
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

/// Dispatch tests to slave processes.
static void
master_dispatch_loop_with_fork(void)
{
    const test_def_t *def;

    cru_foreach_test_def(def) {
        if (!def->priv.enable)
            continue;

        if (def->skip) {
            master_report_result(def, TEST_RESULT_SKIP);
            continue;
        }

        master_dispatch_test(def);
        if (master.goto_next_phase)
            return;

        master_collect_result(0);
        if (master.goto_next_phase)
            return;
    }
}

static void
master_dispatch_test(const test_def_t *def)
{
    slave_t *slave = NULL;

    assert(master.cur_dispatched_tests <= master.max_dispatched_tests);

    master_yield_to_sigint();
    if (master.goto_next_phase)
        return;

    while (master.cur_dispatched_tests == master.max_dispatched_tests) {
        master_collect_result(0);
        if (master.goto_next_phase)
            return;
    }

    while (!slave) {
        master_yield_to_sigint();
        if (master.goto_next_phase)
            return;

        slave = master_get_open_slave();
        if (master.goto_next_phase)
            return;
    }

    slave_start_test(slave, def);
}

static slave_t *
master_get_open_slave(void)
{
    for (;;) {
        slave_t *slave = NULL;

        if (master.goto_next_phase)
            return NULL;

        master_for_each_slave_slot(slave) {
            if (slave_is_open(slave)) {
                return slave;
            }
        }

        switch (runner_opts.isolation_mode) {
        case RUNNER_ISOLATION_MODE_PROCESS:
            if (master.num_slaves < master.max_dispatched_tests) {
                return master_get_new_slave();
            }
            break;
        case RUNNER_ISOLATION_MODE_THREAD:
            if (master.num_slaves == 0) {
                return master_get_new_slave();
            }
            break;
        }

        // All slaves are busy. Wait for a test to finish, then try again.
        master_collect_result(-1);
    }
}

static slave_t *
master_get_new_slave(void)
{
    slave_t *slave;

    if (master.goto_next_phase)
        return NULL;

    slave = master_find_unborn_slave();
    if (!slave)
        return NULL;

    assert(!slave->pid);
    *slave = (slave_t) {0};

    if (!slave_pipe_init(slave, &slave->dispatch_pipe))
        goto fail;
    if (!slave_pipe_init(slave, &slave->result_pipe))
        goto fail;
    if (!slave_pipe_init(slave, &slave->stdout_pipe))
        goto fail;
    if (!slave_pipe_init(slave, &slave->stderr_pipe))
        goto fail;

    // Flush standard out and error before forking.  Otherwise, both the
    // child and parent processes will have the same queue and, when that
    // gets flushed, we'll end up with duplicate data in the output.
    fflush(stdout);
    fflush(stderr);

    slave->pid = fork();

    if (slave->pid == -1) {
        slave->pid = 0;
        loge("test runner failed to fork slave process");
        goto fail;
    }

    if (slave->pid == 0) {
        // Before the slave duplicates stdout and stderr, write only to the
        // debug log. This avoids corrupting the master's stdout and stderr
        // with interleaved output during concurrent test runs.
        if (!(dup2(slave->stdout_pipe.write_fd, STDOUT_FILENO) != -1 &&
              dup2(slave->stderr_pipe.write_fd, STDERR_FILENO) != -1)) {
            logd("runner failed to dup slave's stdout and stderr");
            exit(EXIT_FAILURE);
        }

        slave_pipe_finish(&slave->stdout_pipe);
        slave_pipe_finish(&slave->stderr_pipe);

        set_sigint_handler(SIG_DFL);
        master_finish_epoll();

        if (!slave_pipe_become_reader(&slave->dispatch_pipe))
            exit(EXIT_FAILURE);
        if (!slave_pipe_become_writer(&slave->result_pipe))
            exit(EXIT_FAILURE);

        slave_run(slave->dispatch_pipe.read_fd, slave->result_pipe.write_fd);

        exit(EXIT_SUCCESS);
    }

    if (!slave_pipe_become_writer(&slave->dispatch_pipe))
        goto fail;
    if (!slave_pipe_become_reader(&slave->result_pipe))
        goto fail;
    if (!slave_pipe_become_reader(&slave->stdout_pipe))
        goto fail;
    if (!slave_pipe_become_reader(&slave->stderr_pipe))
        goto fail;

    if (fcntl(slave->result_pipe.read_fd, F_SETFL, O_NONBLOCK) == -1)
        goto fail;
    if (fcntl(slave->stdout_pipe.read_fd, F_SETFL, O_NONBLOCK) == -1)
        goto fail;
    if (fcntl(slave->stderr_pipe.read_fd, F_SETFL, O_NONBLOCK) == -1)
        goto fail;

    if (!master_epoll_add_slave_pipe(&slave->result_pipe, 0))
        goto fail;
    if (!master_epoll_add_slave_pipe(&slave->stdout_pipe, 0))
        goto fail;
    if (!master_epoll_add_slave_pipe(&slave->stderr_pipe, 0))
        goto fail;

    ++master.num_slaves;

    return slave;

fail:
    loge("runner failed to initialize slave process");

    // If we can't create slaves, we should proceed to the result summary.
    master.goto_next_phase = true;

    return NULL;
}

static void
master_cleanup_dead_slave(slave_t *slave)
{
    int err;

    assert(slave->pid);
    assert(slave->is_dead);

    slave_drain_result_pipe(slave);
    slave_pipe_drain_to_fd(&slave->stdout_pipe, STDOUT_FILENO);
    slave_pipe_drain_to_fd(&slave->stderr_pipe, STDERR_FILENO);

    // Any remaining tests owned by the slave are lost.
    for (uint32_t i = 0; i < slave->tests.len; ++i) {
        const test_def_t *def = slave->tests.data[i];
        master_report_result(def, TEST_RESULT_LOST);
    }

    assert(master.cur_dispatched_tests >= slave->tests.len);
    master.cur_dispatched_tests -= slave->tests.len;
    slave->tests.len = 0;

    err = epoll_ctl(master.epoll_fd, EPOLL_CTL_DEL,
                    slave->result_pipe.read_fd, NULL);
    if (err == -1) {
        loge("runner failed to remove slave process's pipe from epoll "
             "fd; abort!");
        abort();
    }

    slave_pipe_finish(&slave->dispatch_pipe);
    slave_pipe_finish(&slave->result_pipe);
    slave_pipe_finish(&slave->stdout_pipe);
    slave_pipe_finish(&slave->stderr_pipe);

    slave->pid = 0;
    --master.num_slaves;
}

static slave_t *
master_find_unborn_slave(void)
{
    slave_t *slave;

    master_for_each_slave_slot(slave) {
        if (!slave->pid) {
            return slave;
        }
    }

    return NULL;
}

static void
master_collect_result(int timeout_ms)
{
    struct epoll_event event;

    master_yield_to_sigint();
    if (master.goto_next_phase)
        return;

    if (epoll_wait(master.epoll_fd, &event, 1, timeout_ms) <= 0)
        return;

    master_handle_epoll_event(&event);
}

static void
master_report_result(const test_def_t *def, test_result_t result)
{
    log_tag(test_result_to_string(result), "%s", def->name);
    fflush(stdout);

    switch (result) {
    case TEST_RESULT_PASS: master.num_pass++; break;
    case TEST_RESULT_FAIL: master.num_fail++; break;
    case TEST_RESULT_SKIP: master.num_skip++; break;
    case TEST_RESULT_LOST: master.num_lost++; break;
    }
}

static bool
master_send_packet(slave_t *slave, const dispatch_packet_t *pk)
{
    bool result = false;
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

    static_assert(sizeof(*pk) <= PIPE_BUF, "dispatch packets will not be read "
                  "and written atomically");
    if (write(slave->dispatch_pipe.write_fd, pk, sizeof(*pk)) != sizeof(*pk))
        goto cleanup;

    result = true;

cleanup:
    err = sigaction(SIGPIPE, &old_sa, NULL);
    if (err == -1) {
        loge("test runner failed to re-enable SIGPIPE");
        abort();
    }

    return result;
}

static void
master_kill_all_slaves(void)
{
    int err;
    slave_t *slave;

    master_for_each_slave_slot(slave) {
        if (!slave->pid)
            continue;

        err = kill(slave->pid, SIGINT);
        if (err) {
            loge("runner failed to kill child process %d", slave->pid);
            abort();
        }
    }
}

static void
master_init_epoll(void)
{
    sigset_t sigset;
    int err;

    assert(master.signal_fd == -1);
    assert(master.epoll_fd == -1);

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);

    master.signal_fd = signalfd(-1, &sigset, SFD_CLOEXEC);
    if (master.signal_fd == -1)
        goto fail;

    err = sigprocmask(SIG_BLOCK, &sigset, NULL);
    if (err == -1)
        goto fail;

    master.epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (master.epoll_fd == -1)
        goto fail;

    err = epoll_ctl(master.epoll_fd, EPOLL_CTL_ADD, master.signal_fd,
                    &(struct epoll_event) {
                        .events = EPOLLIN,
                        .data = {
                            .ptr = &master.signal_fd,
                        },
                    });
    if (err == -1)
        goto fail;

    return;

fail:
    loge("runner failed to setup epoll fd");
    master.goto_next_phase = true;
}

static void
master_finish_epoll(void)
{
    sigset_t sigset;

    assert(master.signal_fd >= 0);
    assert(master.epoll_fd >= 0);

    close(master.signal_fd);
    close(master.epoll_fd);

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

static bool
master_epoll_add_slave_pipe(slave_pipe_t *pipe, int rw)
{
    int err;

    assert(rw == 0 || rw == 1);

    err = epoll_ctl(master.epoll_fd, EPOLL_CTL_ADD, pipe->fd[rw],
                    &(struct epoll_event) {
                        .events = EPOLLIN,
                        .data = { .ptr = pipe },
                    });
    if (err == -1) {
        loge("runner failed to add a slave pipe to epoll fd");
        return false;
    }

    return true;
}

static void
master_handle_epoll_event(const struct epoll_event *event)
{
    if (event->data.ptr == &master.signal_fd) {
        master_handle_signal_event(event);
    } else {
        master_handle_pipe_event(event);
    }
}

static void
master_handle_pipe_event(const struct epoll_event *event)
{
    slave_pipe_t *pipe = event->data.ptr;

    assert(event->data.ptr != &master.signal_fd);

    switch ((void*) pipe - (void*) pipe->slave) {
    case offsetof(slave_t, result_pipe):
        slave_drain_result_pipe(pipe->slave);
        break;
    case offsetof(slave_t, stdout_pipe):
        slave_pipe_drain_to_fd(pipe, STDOUT_FILENO);
        break;
    case offsetof(slave_t, stderr_pipe):
        slave_pipe_drain_to_fd(pipe, STDERR_FILENO);
        break;
    default:
        log_internal_error("invalid slave pipe in epoll event");
        break;
    }
}

static void
master_handle_signal_event(const struct epoll_event *event)
{
    struct signalfd_siginfo siginfo;
    int n;

    assert(event->data.ptr == &master.signal_fd);

    n = read(master.signal_fd, &siginfo, sizeof(siginfo));
    if (n != sizeof(siginfo))
        log_abort("runner failed to read from signal fd");

    switch (siginfo.ssi_signo) {
    case SIGCHLD:
        master_handle_sigchld();
        break;
    default:
        log_abort("runner caught unexpected signal %d", siginfo.ssi_signo);
        break;
    }
}

static void
master_handle_sigchld(void)
{
    pid_t pid;

    while ((pid = waitpid(-1, /*status*/ NULL, WNOHANG)) > 0) {
        slave_t *slave;

        slave = find_slave_by_pid(pid);
        if (!slave) {
            loge("runner caught unexpected pid");
            master.goto_next_phase = true;
            return;
        }

        slave->is_dead = true;
        master_cleanup_dead_slave(slave);
    }
}

static void
master_handle_sigint(int sig)
{
    assert(sig == SIGINT);
    atomic_store(&master.sigint_flag, true);
}

/// Take actions triggered by any previously received SIGINT.
static void
master_yield_to_sigint(void)
{
    if (!atomic_exchange(&master.sigint_flag, false))
        return;

    master_kill_all_slaves();

    // A second SIGINT, if received before the runner resumes dispatching
    // tests, halts the testrun. Give the user a short window in which to send
    // the second SIGINT.
    nanosleep(&(struct timespec) { .tv_nsec = 500000000 }, NULL);

    if (!atomic_exchange(&master.sigint_flag, false))
        return;

    // The runner received the second SIGINT. Halt the testrun.
    master.goto_next_phase = true;
}

/// Is the slave accepting new tests?
static bool
slave_is_open(const slave_t *slave)
{
    if (!slave->pid)
        return false;

    if (slave->is_dead)
        return false;

    switch (runner_opts.isolation_mode) {
    case RUNNER_ISOLATION_MODE_PROCESS:
        // The master sends each slave exactly one test.
        return slave->lifetime_test_count == 0;
    case RUNNER_ISOLATION_MODE_THREAD:
        return slave->tests.len < ARRAY_LENGTH(slave->tests.data);
    }

    return false;
}

static int32_t
slave_find_test(slave_t *slave, const test_def_t *def)
{
    for (uint32_t i = 0; i < slave->tests.len; ++i) {
        if (slave->tests.data[i] == def) {
            return i;
        }
    }

    return -1;
}

static bool
slave_insert_test(slave_t *slave, const test_def_t *def)
{
    if (slave->is_dead)
        return false;

    if (slave->tests.len >= ARRAY_LENGTH(slave->tests.data))
        return false;

    slave->tests.data[slave->tests.len++] = def;
    ++master.cur_dispatched_tests;

    return true;
}

static void
slave_rm_test(slave_t *slave, const test_def_t *def)
{
    int32_t i;

    i = slave_find_test(slave, def);
    if (i < 0) {
        loge("slave cannot remove test it doesn't own");
        return;
    }

    assert(slave->tests.len >= 1);
    assert(master.cur_dispatched_tests >= 1);

    --slave->tests.len;
    --master.cur_dispatched_tests;

    memmove(slave->tests.data + i, slave->tests.data + i + 1,
            slave->tests.len);
}

static bool
slave_start_test(slave_t *slave, const test_def_t *def)
{
    const dispatch_packet_t pk = { .test_def = def };

    if (!def)
        return false;

    if (!slave->pid)
        return false;

    if (master.cur_dispatched_tests >= master.max_dispatched_tests)
        return false;

    if (!slave_insert_test(slave, def))
        return false;

    log_tag("start", "%s", def->name);

    if (!master_send_packet(slave, &pk)) {
        slave_rm_test(slave, def);
        return false;
    }

    ++slave->lifetime_test_count;

    switch (runner_opts.isolation_mode) {
    case RUNNER_ISOLATION_MODE_PROCESS:
        // The master sends each slave exactly one test.
        slave_send_sentinel(slave);
        break;
    case RUNNER_ISOLATION_MODE_THREAD:
        // The master may send the slave multiple tests. The master will tell
        // the slave to expect no more tests by later sending it a NULL test.
        break;
    }

    return true;
}

static void
slave_send_sentinel(slave_t *slave)
{
    assert(slave->pid);

    if (slave->recvd_sentinel || slave->is_dead)
        return;

    master_send_packet(slave, &(dispatch_packet_t) { .test_def = NULL });
    slave->recvd_sentinel = true;
}

static void
slave_drain_result_pipe(slave_t *slave)
{
    for (;;) {
        result_packet_t pk;

        static_assert(sizeof(pk) <= PIPE_BUF, "result packets will not be "
                      "read and written atomically");

        // To avoid deadlock between master and slave, this read must be
        // non-blocking.
        if (read(slave->result_pipe.read_fd, &pk, sizeof(pk)) != sizeof(pk))
            return;

        slave_rm_test(slave, pk.test_def);
        master_report_result(pk.test_def, pk.result);
    }
}

static slave_t *
find_slave_by_pid(pid_t pid)
{
    slave_t *slave;

    master_for_each_slave_slot(slave) {
        if (slave->pid == pid) {
            return slave;
        }
    }

    return NULL;
}

static bool
slave_pipe_init(slave_t *slave, slave_pipe_t *pipe)
{
    int err;

    err = pipe2(pipe->fd, O_CLOEXEC);
    if (err) {
        loge("failed to create pipe");
        return false;
    }

    pipe->slave = slave;

    return true;
}

static void
slave_pipe_finish(slave_pipe_t *pipe)
{
    for (int i = 0; i < 2; ++i) {
        if (pipe->fd[i] != -1) {
            close(pipe->fd[i]);
        }
    }
}

static bool
slave_pipe_become_reader(slave_pipe_t *pipe)
{
    int err;

    assert(pipe->read_fd != -1);
    assert(pipe->write_fd != -1);

    err = close(pipe->write_fd);
    if (err == -1) {
        loge("runner failed to close pipe's write fd");
        return false;
    }

    pipe->write_fd = -1;

    return true;
}

static bool
slave_pipe_become_writer(slave_pipe_t *pipe)
{
    int err;

    assert(pipe->read_fd != -1);
    assert(pipe->write_fd != -1);

    err = close(pipe->read_fd);
    if (err == -1) {
        loge("runner failed to close pipe's read fd");
        return false;
    }

    pipe->read_fd = -1;

    return true;
}

static void
slave_pipe_drain_to_fd(slave_pipe_t *pipe, int fd)
{
    char buf[4096];

    for (;;) {
        ssize_t nread = 0;
        ssize_t nwrite = 0;

        if (master.goto_next_phase)
            return;

        nread = read(pipe->read_fd, buf, sizeof(buf));
        if (nread <= 0)
            return;

        while (nread > 0) {
            if (master.goto_next_phase)
                return;

            nwrite = write(fd, buf, nread);
            if (nwrite < 0) {
                // Even on write errors, we must continue to drain the slave's
                // pipe. Otherwise the slave may block on a full pipe.
                continue;
            }

            nread -= nwrite;
        }
    }
}
