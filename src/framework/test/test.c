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
    }

    cru_unreachable;
}

bool
test_is_current(void)
{
    return current.test != NULL;
}

static void
test_set_image_filename(test_t *t)
{
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    // Always define the reference image's filename, even when
    // test_def_t::no_image is set. This will be useful for tests that
    // generate their reference images at runtime and wish to dump them to
    // disk.
    assert(t->ref_image_filename.len == 0);

    if (t->def->image_filename) {
        // Test uses a custom filename.
        string_copy_cstr(&t->ref_image_filename, t->def->image_filename);
    } else {
        // Test uses the default filename.
        string_copy_cstr(&t->ref_image_filename, t->def->name);
        string_append_cstr(&t->ref_image_filename, ".ref.png");
    }
}

void
test_destroy(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    if (!t)
        return;

    assert(t->phase == CRU_TEST_PHASE_PRESTART ||
           t->phase == CRU_TEST_PHASE_STOPPED);

    // This test must own no running threads:
    //   - In the "prestart" phase, no test threads have been created yet.
    //   - In the "stopped" phase, all test threads have been joined.
    assert(t->threads == NULL);

    pthread_mutex_destroy(&t->stop_mutex);
    pthread_cond_destroy(&t->stop_cond);
    string_finish(&t->ref_image_filename);

    free(t);
}

test_t *
test_create(const test_def_t *def)
{
    ASSERT_NOT_IN_TEST_THREAD;

    test_t *t = NULL;
    int err;

    t = xzalloc(sizeof(*t));
    t->def = def;
    t->phase = CRU_TEST_PHASE_PRESTART;
    t->result = TEST_RESULT_PASS;
    t->ref_image_filename = STRING_INIT;
    t->opt.no_dump = true;
    t->opt.no_cleanup = false;
    t->opt.use_spir_v = false;
    t->opt.no_separate_cleanup_thread = false;

    if (t->def->samples > 0) {
        loge("%s: multisample tests not yet supported", t->def->name);
        goto fail;
    }

    err = pthread_mutex_init(&t->stop_mutex, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized mutex later.
        loge("%s: failed to init mutex during test creation",
                 t->def->name);
        abort();
    }

    err = pthread_cond_init(&t->stop_cond, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized cond later.
        loge("%s: failed to init thread condition during test creation",
                 t->def->name);
        abort();
    }

    test_set_image_filename(t);

    return t;

fail:
    t->result = CRU_TEST_PHASE_STOPPED;
    t->result = TEST_RESULT_FAIL;
    return t;
}

bool
test_enable_bootstrap(test_t *t,
                          uint32_t image_width, uint32_t image_height)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    if (!t->def->no_image && (image_width == 0 || image_height == 0)) {
        loge("%s: bootstrap image must have non-zero size", t->def->name);
        return false;
    }

    t->opt.bootstrap = true;
    t->opt.no_cleanup = true;
    t->width = image_width;
    t->height = image_height;

    return true;
}

void
test_enable_dump(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.no_dump = false;
}

bool
test_disable_cleanup(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.no_cleanup = true;

    return true;
}

void
test_enable_spir_v(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.use_spir_v = true;
}

void
test_disable_separate_cleanup_thread(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.no_separate_cleanup_thread = true;
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

void
test_start(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->phase = CRU_TEST_PHASE_SETUP;

    if (t->def->skip) {
        t->phase = CRU_TEST_PHASE_STOPPED;
        t->result = TEST_RESULT_SKIP;
        return;
    }

    if (!test_thread_create(t, main_thread_start, t, NULL)) {
        t->phase = CRU_TEST_PHASE_STOPPED;
        t->result = TEST_RESULT_FAIL;
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

    while (t->phase < CRU_TEST_PHASE_STOPPED) {
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
