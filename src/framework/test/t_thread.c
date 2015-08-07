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

#include "test.h"
#include "t_phases.h"
#include "t_thread.h"

static noreturn void *
test_thread_start(void *arg)
{
    ASSERT_NOT_IN_TEST_THREAD;

    test_thread_arg_t targ = *(test_thread_arg_t *) arg;
    free(arg);

    test_t *t = targ.test;
    cru_cleanup_stack_t *cleanup = NULL;

    cleanup = cru_cleanup_create();
    if (!cleanup) {
        // Without a cleanup stack, this is not a well-formed test thread.
        // That prevents us from calling any "t_*" functions, even t_end(). So
        // give up and die.
        loge("%s: failed to create cleanup stack for test thread",
             t->def->name);
        abort();
    }

    cru_slist_prepend_atomic(&targ.test->cleanup_stacks, cleanup);

    // Bind the thread to the test before entering the thread's real start
    // function.
    current = (cru_current_test_t) {
        .test = t,
        .cleanup = cleanup,
    };

    // Yield, because the test may already be done. If it's done, there's no
    // reason to enter the thread's start function.
    t_thread_yield();

    targ.start_func(targ.start_arg);

    // Concurrency Cases:
    //
    //   - Control does not reach here if this thread called a noreturn
    //     function, such as t_thread_release().
    //
    //   - If control does reach here and this is the test's sole remaining
    //     thread, then this thread must continue to the next test phase.
    //     Therefore the thread must not exit.
    //
    //   - If control does reach here and the test owns other running threads,
    //     then this thread must defer selection of the test result to
    //     the other threads. Therefore this thread must not call t_end(),
    //     t_pass(), nor t_fail().
    //
    // Instead of deciding here the next correct action, defer the decision to
    // t_thread_release().
    t_thread_release();
}

bool
test_thread_create(test_t *t, void (*start)(void *arg), void *arg)
{
    pthread_t thread = NULL;
    test_thread_arg_t *targ;

    targ = xmalloc(sizeof(*targ));
    *targ = (test_thread_arg_t) {
        .test = t,
        .start_func = start,
        .start_arg = arg,
    };

    // To prevent race conditions, this thread must increment the test's thread
    // count *before* the new thread starts. The increment remains tentative
    // until phtread_create() returns, indicating success or failure. On
    // failue, this thread rolls back the increment.
    //
    // This thread *must not* access the thread count during the increment's
    // tentative window.  In particular, it cannot call any function that may
    // access the thread count, such as t_thread_release() nor
    // t_enter_next_phase(). Otherwise, the thread count may underflow past 0.
    //
    // Other threads may safely access the thread count during the increment's
    // tentative window. The test's phase transitions (should!) provide the
    // needed safety.
    t->num_threads += 1;

    if (pthread_create(&thread, NULL, test_thread_start, targ) == -1) {
        t->num_threads -= 1;
        return false;
    }

    return true;
}

void
t_thread_start(void (*start)(void *arg), void *arg)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (!test_thread_create(t, start, arg))
        t_failf("%s()", __func__);
}

noreturn void
t_thread_release(void)
{
    GET_CURRENT_TEST(t);

    // The phase is volatile. Cache it locally so the correct phase appears in
    // the error message below.
    test_phase_t phase = t->phase;

    switch (phase) {
    case TEST_PHASE_PRESTART:
    case TEST_PHASE_SETUP:
    case TEST_PHASE_MAIN:
    case TEST_PHASE_PRECLEANUP:
    case TEST_PHASE_CLEANUP:
        t_enter_next_phase();
        break;
    case TEST_PHASE_STOPPED:
    default:
        log_internal_error("%s: invalid phase %d", __func__, phase);
        break;
    }
}

void
t_thread_yield(void)
{
    GET_CURRENT_TEST(t);

    if (t->result_is_final && t->num_threads > 1) {
        // The thread's current exeuction path can do no useful work towards
        // selecting the test result, because the test result is already final.
        // Therefore, the thread should diverge from its current path,
        // releasing itself to the framework.
        t_thread_release();
    }
}

void
t_thread_sieve(void)
{
    GET_CURRENT_TEST(t);

    uint32_t n = atomic_fetch_sub(&t->num_threads, 1);

    // The test's thread count is temporarily invalid: it is one less than the
    // actual thread count. Be careful!

    if (n == 0) {
        log_internal_error("%s: test's thread count was 0 in a running test "
                           "thread", __func__);
    } else if (n == 1) {
        // The test owns exactly one thread (this one!). So bump the thread
        // count back to one.
        t->num_threads = 1;
        t_thread_yield();
        return;
    } else {
        pthread_exit(NULL);
    }
}
