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
#include "t_thread.h"

__thread cru_current_test_t current
    __attribute__((tls_model("local-exec"))) = {0};

const char *
test_result_to_string(test_result_t result)
{
    switch (result) {
    case TEST_RESULT_PASS:
        return "pass";
    case TEST_RESULT_SKIP:
        return "skip";
    case TEST_RESULT_FAIL:
        return "fail";
    case TEST_RESULT_LOST:
        return "lost";
    }

    cru_unreachable;
}

bool
test_is_current(void)
{
    return current.test != NULL;
}

void
test_broadcast_stop(test_t *t)
{
    int err;

    err = pthread_mutex_lock(&t->stop_mutex);
    if (err)
        log_abort("%s: failed to lock mutex", __func__);

    assert(t->num_threads == 0);
    assert(t->phase < TEST_PHASE_STOPPED);

    t->phase = TEST_PHASE_STOPPED;

    err = pthread_mutex_unlock(&t->stop_mutex);
    if (err)
        log_abort("%s: failed to lock mutex", __func__);

    pthread_cond_broadcast(&t->stop_cond);
}

static void
test_set_ref_filenames(test_t *t)
{
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    assert(t->ref.filename.len == 0);
    assert(t->ref.stencil_filename.len == 0);

    if (t->def->image_filename) {
        // Test uses a custom filename.
        string_copy_cstr(&t->ref.filename, t->def->image_filename);
    } else {
        // Test uses the default filename.
        //
        // Always define the reference image's filename, even when
        // test_def_t::no_image is set. This will be useful for tests that
        // generate their reference images at runtime and wish to dump them to
        // disk.
        string_copy_cstr(&t->ref.filename, t->def->name);
        string_append_cstr(&t->ref.filename, ".ref.png");
    }

    if (!t->def->ref_stencil_filename) {
        // Test does not have a reference stencil image
    } else if (cru_streq(t->def->ref_stencil_filename, "DEFAULT")) {
        string_copy_cstr(&t->ref.stencil_filename, t->def->name);
        string_append_cstr(&t->ref.stencil_filename, ".ref-stencil.png");
    } else {
        // Test uses a custom filename.
        string_copy_cstr(&t->ref.stencil_filename,
                         t->def->ref_stencil_filename);
    }
}

void
test_destroy(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    if (!t)
        return;

    assert(t->phase == TEST_PHASE_PRESTART ||
           t->phase == TEST_PHASE_STOPPED);

    // This test must own no running threads:
    //   - In the "prestart" phase, no test threads have been created yet.
    //   - In the "stopped" phase, all test threads have exited.
    assert(t->num_threads == 0);

    pthread_mutex_destroy(&t->stop_mutex);
    pthread_cond_destroy(&t->stop_cond);
    string_finish(&t->ref.filename);
    string_finish(&t->ref.stencil_filename);

    free(t);
}

test_t *
test_create_s(const test_create_info_t *info)
{
    ASSERT_NOT_IN_TEST_THREAD;

    test_t *t = NULL;
    int err;

    t = xzalloc(sizeof(*t));

    t->phase = ATOMIC_VAR_INIT(TEST_PHASE_PRESTART);
    t->result = TEST_RESULT_PASS;
    t->ref.filename = STRING_INIT;
    t->ref.stencil_filename = STRING_INIT;

    t->def = info->def;
    t->opt.no_dump = !info->enable_dump;
    t->opt.no_cleanup = !info->enable_cleanup_phase;
    t->opt.no_separate_cleanup_thread = !info->enable_separate_cleanup_thread;
    t->opt.use_spir_v = info->enable_spir_v;
    t->opt.bootstrap = info->enable_bootstrap;
    t->opt.queue_family_index = info->queue_family_index;

    if (info->enable_bootstrap) {
        if (info->enable_cleanup_phase) {
            loge("%s: enable_bootstrap and enable_cleanup_phase are mutually "
                 "exclusive", __func__);
            goto fail;
        }

        // Force-enable image dumps when in bootstrap mode.
        t->opt.no_dump = false;

        if (!info->def->no_image &&
            (info->bootstrap_image_width == 0 ||
             info->bootstrap_image_height == 0)) {
            loge("%s: bootstrap image must have non-zero size", t->def->name);
            goto fail;
        }

        t->ref.width = info->bootstrap_image_width;
        t->ref.height = info->bootstrap_image_height;
    }

    if (t->def->samples > 0) {
        loge("%s: multisample tests not yet supported", t->def->name);
        goto fail;
    }

    err = pthread_mutex_init(&t->stop_mutex, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized mutex later.
        loge("%s: failed to init mutex during test creation", t->def->name);
        abort();
    }

    err = pthread_cond_init(&t->stop_cond, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized cond later.
        loge("%s: failed to init thread condition during test creation",
             t->def->name);
        abort();
    }

    test_set_ref_filenames(t);

    return t;

fail:
    t->result = TEST_RESULT_FAIL;
    t->phase = TEST_PHASE_STOPPED;
    return t;
}

/// Illegal to call before test_wait().
test_result_t
test_get_result(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_STOPPED_PHASE(t);

    return t->result;
}

const cru_format_info_t *
t_format_info(VkFormat format)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    const cru_format_info_t *info;

    info = cru_format_get_info(format);
    t_assertf(info, "failed to find cru_format_info for VkFormat %d", format);

    return info;
}

static void
t_thread_release_wrapper(void *ignore)
{
    t_thread_release();
}

void
test_start(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    if (t->def->skip) {
        t->result = TEST_RESULT_SKIP;
        test_broadcast_stop(t);
        return;
    }

    // Start the test's first thread in a failure mode [that is, in
    // t_thread_release()] and force it to recover. Doing so provides
    // persistent validation of that recovery path.
    if (!test_thread_create(t, t_thread_release_wrapper, NULL)) {
        loge("%s: failed to create test's start thread", t->def->name);
        t->result = TEST_RESULT_FAIL;
        test_broadcast_stop(t);
        return;
    }
}

void
test_wait(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    int err;

    err = pthread_mutex_lock(&t->stop_mutex);
    if (err) {
        loge("%s: failed to lock test mutex", t->def->name);
        abort();
    }

    while (t->phase < TEST_PHASE_STOPPED) {
        // AVOID DEADLOCK! When a test thread transitions to
        // TEST_PHASE_STOPPED, it must be holding the test::stop_mutex lock.
        err = pthread_cond_wait(&t->stop_cond, &t->stop_mutex);
        if (err) {
            loge("%s: failed to wait on test's result condition",
                  t->def->name);
            abort();
        }
    }

    pthread_mutex_unlock(&t->stop_mutex);
}

void
test_result_merge(test_result_t *accum,
                      test_result_t new_result)
{
    *accum = MAX(*accum, new_result);
}
