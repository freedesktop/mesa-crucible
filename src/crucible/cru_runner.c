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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <crucible/cru_log.h>

#include "cru_runner.h"
#include "cru_test.h"
#include "cru_test_def.h"

typedef struct cru_runch cru_runch_t;
typedef struct cru_runch_packet cru_runch_packet_t;

/// Communication channel among the runner's processes.
struct cru_runch {
    int pipe[2];
};

struct cru_runch_packet {
    enum {
        CRU_RUNCH_PACKET_TYPE_TEST_STARTED,
        CRU_RUNCH_PACKET_TYPE_TEST_DONE,
    } type;

    union {
        struct {
            uint64_t test_def_id;
        } test_started;

        struct {
            uint64_t test_def_id;
            cru_test_result_t result;
        } test_done;
    };
};

/// \brief PID of the runner's slave process.
///
/// \par Values
///     - if -1: In the runner's master process before forking.
///     - if 0: In the runner's master process after forking.
///     - if > 0: In the runner's slave process.
static pid_t slave_pid = -1;

/// Document and assert that this is the runner's master process.
#define ASSERT_IN_MASTER_PROCESS assert(slave_pid == -1 || slave_pid > 0)

/// Document and assert that this is the runner's slave process.
#define ASSERT_IN_SLAVE_PROCESS assert(slave_pid == 0)

bool cru_runner_do_cleanup_phase = true;
bool cru_runner_do_image_dumps = false;
bool cru_runner_use_spir_v = false;

// Only the master process touches these test counts.
static uint32_t num_tests = 0;
static uint32_t num_pass = 0;
static uint32_t num_fail = 0;
static uint32_t num_skip = 0;

static bool
cru_runch_init(cru_runch_t *ch)
{
    ASSERT_IN_MASTER_PROCESS;

    int err;

    err = pipe2(ch->pipe, O_CLOEXEC);
    if (err) {
        cru_loge("%s: failed to create pipe", __func__);
        return false;
    }

    return true;
}

static void
cru_runch_send(const cru_runch_t *ch, const cru_runch_packet_t *pk)
{
    ASSERT_IN_SLAVE_PROCESS;

    // POSIX ensures that writes less than PIPE_BUF are atomic.
    cru_static_assert(sizeof(*pk) < PIPE_BUF);

    write(ch->pipe[1], pk, sizeof(*pk));

}

static void
cru_runch_send_start_test(const cru_runch_t *ch,
                          uint64_t test_def_id)
{
    ASSERT_IN_SLAVE_PROCESS;

    cru_runch_send(ch,
        &(cru_runch_packet_t) {
            .type = CRU_RUNCH_PACKET_TYPE_TEST_STARTED,
            .test_started = {
                .test_def_id = test_def_id,
            },
        });
}

static void
cru_runch_send_end_test(const cru_runch_t *ch, uint64_t test_def_id,
                        cru_test_result_t result)
{
    ASSERT_IN_SLAVE_PROCESS;

    cru_runch_send(ch,
        &(cru_runch_packet_t) {
            .type = CRU_RUNCH_PACKET_TYPE_TEST_DONE,
            .test_done = {
                .test_def_id = test_def_id,
                .result = result,
            },
        });
}

/// Return true if there are no remaining packets to read.
static bool
cru_runch_read(const cru_runch_t *ch,
               cru_runch_packet_t *out_pk)
{
    ASSERT_IN_MASTER_PROCESS;

    ssize_t sz;

    // POSIX ensures that writes less than PIPE_BUF are atomic. And cru_runch
    // never writes more than PIPE_BUF. Therefore reads are atomic too.
    cru_static_assert(sizeof(*out_pk) < PIPE_BUF);

    sz = read(ch->pipe[0], out_pk, sizeof(*out_pk));

    return sz == sizeof(*out_pk);
}

/// Mark that the test should be run.
static inline void
mark_test_def(cru_test_def_t *def)
{
    ASSERT_IN_MASTER_PROCESS;

    if (!def->priv.run)
        ++num_tests;

    def->priv.run = true;
}

void
cru_runner_mark_all_nonexample_tests(void)
{
    ASSERT_IN_MASTER_PROCESS;

    cru_test_def_t *def;

    cru_foreach_test_def(def) {
        if (!cru_test_def_match(def, "example.*")) {
            mark_test_def(def);
        }
    }
}

void
cru_runner_mark_matching_tests(const cru_cstr_vec_t *testname_globs)
{
    ASSERT_IN_MASTER_PROCESS;

    cru_test_def_t *def;
    char **glob;

    cru_foreach_test_def(def) {
        cru_vec_foreach(glob, testname_globs) {
            if (cru_test_def_match(def, *glob)) {
                mark_test_def(def);
            }
        }
    }
}

/// Return false if there exist no remaining tests to run.
static bool
slave_run_one_test(const cru_runch_t *ch)
{
    ASSERT_IN_SLAVE_PROCESS;

    static uint64_t def_id = 0;

    const cru_test_def_t *def;
    cru_test_t *test;

    def = cru_test_def_from_id(def_id);
    if (!def) {
        // No more tests.
        return false;
    }

    if (!def->priv.run)
        goto done;

    cru_runch_send_start_test(ch, def_id);

    test = cru_test_create(def);
    if (!test) {
        cru_runch_send_end_test(ch, def_id, CRU_TEST_RESULT_FAIL);
        goto done;
    }

    if (cru_runner_do_image_dumps)
        cru_test_enable_dump(test);

    if (!cru_runner_do_cleanup_phase)
        cru_test_disable_cleanup(test);

    if (cru_runner_use_spir_v)
        cru_test_enable_spir_v(test);

    cru_test_start(test);
    cru_test_wait(test);

    cru_runch_send_end_test(ch, def_id, cru_test_get_result(test));

    cru_test_destroy(test);

done:
    ++def_id;
    return true;
}

/// Return false if there exist no remaining packets to read.
static bool
master_read_one_packet(const cru_runch_t *ch)
{
    ASSERT_IN_MASTER_PROCESS;

    const cru_test_def_t *def;
    cru_runch_packet_t pk;

    if (!cru_runch_read(ch, &pk))
        return false;

    switch (pk.type) {
    case CRU_RUNCH_PACKET_TYPE_TEST_STARTED:
        def = cru_test_def_from_id(pk.test_started.test_def_id);
        cru_log_tag("start", "%s", def->name);
        break;
    case CRU_RUNCH_PACKET_TYPE_TEST_DONE:
        switch (pk.test_done.result) {
        case CRU_TEST_RESULT_PASS: num_pass++; break;
        case CRU_TEST_RESULT_FAIL: num_fail++; break;
        case CRU_TEST_RESULT_SKIP: num_skip++; break;
        }

        def = cru_test_def_from_id(pk.test_done.test_def_id);
        cru_log_tag(cru_test_result_to_string(pk.test_done.result),
                    "%s", def->name);
        break;
    }

    return true;
}

/// Return true if and only all tests pass or skip.
bool
cru_runner_run_tests(void)
{
    ASSERT_IN_MASTER_PROCESS;

    cru_runch_t ch;

    cru_log_align_tags(true);
    cru_logi("running %u tests", num_tests);
    cru_logi("================================");

    if (!cru_runch_init(&ch))
        goto print_summary;

    slave_pid = fork();

    if (slave_pid == -1) {
        cru_loge("test runner failed to fork");
        goto print_summary;
    } else if (slave_pid == 0) {
        ASSERT_IN_SLAVE_PROCESS;

        close(ch.pipe[0]);
        while (slave_run_one_test(&ch)) {}

        // A big, and perhaps unneeded, hammer.
        fflush(stdout);
        fflush(stderr);

        exit(EXIT_SUCCESS);
    }

    ASSERT_IN_MASTER_PROCESS;

    close(ch.pipe[1]);
    while (master_read_one_packet(&ch)) {}

    if (waitpid(slave_pid, /*status*/ NULL, /*flags*/ 0) == -1) {
        cru_loge("runner failed to wait for slave process");
    }

print_summary:
    // A big, and perhaps unneeded, hammer.
    fflush(stdout);
    fflush(stderr);

    uint32_t num_ran = num_pass + num_fail + num_skip;
    uint32_t num_missing = num_tests - num_ran;


    cru_logi("================================");
    cru_logi("ran %u tests", num_ran);
    cru_logi("pass %u", num_pass);
    cru_logi("fail %u", num_fail);
    cru_logi("skip %u", num_skip);

    if (num_missing > 0)
        cru_logi("missing %u", num_missing);

    return num_pass + num_skip == num_tests;
}
