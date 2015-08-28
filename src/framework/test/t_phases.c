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
#include "t_phase_setup.h"
#include "t_phases.h"
#include "t_thread.h"

static noreturn void
t_enter_setup_phase(void)
{
    GET_CURRENT_TEST(t);
    assert(t->num_threads == 1);
    t->phase = TEST_PHASE_SETUP;

    if (!t->opt.bootstrap && !t->def->no_image) {
        t_setup_ref_images();
    }

    t_setup_vulkan();
    t_enter_next_phase();
}

static noreturn void
t_enter_main_phase(void)
{
    GET_CURRENT_TEST(t);
    assert(t->num_threads == 1);
    t->phase = TEST_PHASE_MAIN;

    if (t->result_is_final) {
        // A previous phase has already selected the test's result. Therefore
        // there is no reason to run the test's user-supplied start function.
        t_enter_next_phase();
    }

    // Run the user-supplied start function.
    t_assertf(t->def->start, "test defines no start function");
    t->def->start();

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

static noreturn void
t_enter_precleanup_phase(void)
{
    GET_CURRENT_TEST(t);
    assert(t->num_threads == 1);
    t->phase = TEST_PHASE_PRECLEANUP;

    if (t->vk.queue) {
        // Don't prematurely end the test before the test has completed executing.
        vkQueueWaitIdle(t->vk.queue);
    }

    if (!t->result_is_final && !t->def->no_image) {
        t_compare_image();
    }

    t_enter_next_phase();
}

static noreturn void
t_unwind_cleanup_stacks(void *ignore)
{
    GET_CURRENT_TEST(t);
    assert(t->phase == TEST_PHASE_CLEANUP);

    cru_cleanup_stack_t *cleanup;

    while ((cleanup = cru_slist_pop(&t->cleanup_stacks))) {
        if (t->opt.no_cleanup)
            cru_cleanup_pop_all_noop(cleanup);

        cru_cleanup_release(cleanup);
    }

    t_enter_next_phase();
}

static noreturn void
t_enter_cleanup_phase(void)
{
    GET_CURRENT_TEST(t);
    assert(t->num_threads == 1);
    t->phase = TEST_PHASE_CLEANUP;

    if (t->opt.no_separate_cleanup_thread) {
        t_unwind_cleanup_stacks(NULL);
        t_enter_next_phase();
    } else {
        t_thread_start(t_unwind_cleanup_stacks, NULL);
        t_thread_release();
    }
}

static noreturn void
t_enter_stopped_phase(void)
{
    GET_CURRENT_TEST(t);
    assert(t->num_threads == 1);

    // The test is moments away from death. Maybe its result remains untouched
    // since test initialization, or maybe its result was previously set by
    // t_end(). Regardless, the test's current result value becomes the final
    // result value.
    t->result_is_final = true;

    // To avoid race conditions with test_wait(), the test's thread count must
    // be zero before the test transitions to TEST_PHASE_STOPPED;
    t->num_threads = 0;
    test_broadcast_stop(t);

    // The test's thread count is now invalid: the test still owns a running
    // thread (this one!) despite its thread count being zero. We must take
    // extra care to avoid all code paths that modify the thread count or
    // expect it to be non-zero. That's easily accomplished by exiting the
    // thread.
    pthread_exit(NULL);
}

noreturn void
t_enter_next_phase(void)
{
    GET_CURRENT_TEST(t);

    // Eliminate all race conditions during the phase transition by reducing
    // the test's thread count to 1.
    t_thread_sieve();

    switch (t->phase) {
    case TEST_PHASE_PRESTART:
        t_enter_setup_phase();
        break;
    case TEST_PHASE_SETUP:
        t_enter_main_phase();
        break;
    case TEST_PHASE_MAIN:
        t_enter_precleanup_phase();
        break;
    case TEST_PHASE_PRECLEANUP:
        t_enter_cleanup_phase();
        break;
    case TEST_PHASE_CLEANUP:
        t_enter_stopped_phase();
        break;
    case TEST_PHASE_STOPPED:
    default:
        log_internal_error("%s: invalid phase %d", __func__, t->phase);
        break;
    }
}
