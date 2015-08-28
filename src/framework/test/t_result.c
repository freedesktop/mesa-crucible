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

noreturn void
t_end(test_result_t result)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (!atomic_exchange(&t->result_is_final, true)) {
        test_result_merge(&t->result, result);
    }

    t_thread_release();
}

noreturn void
t_pass(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_PASS);
}

noreturn void
__t_skip(const char *file, int line)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_skipf(file, line, NULL);
}

void noreturn
__t_skipf(const char *file, int line, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_skipfv(file, line, format, va);
    va_end(va);
}

noreturn void
__t_skipfv(const char *file, int line, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_thread_yield();

    string_t s = STRING_INIT;
    string_appendf(&s, "%s:%d", file, line);

    if (format) {
        string_append_cstr(&s, ": ");
        string_vappendf(&s, format, va);
    }

    logi(string_data(&s));
    string_finish(&s);

    __t_skip_silent();
}

noreturn void
__t_skip_silent(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_SKIP);
}

noreturn void
__t_fail(const char *file, int line)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_failf(file, line, NULL);
}

noreturn void
__t_failf(const char *file, int line, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_failfv(file, line, format, va);
    va_end(va);
}

noreturn void
__t_failfv(const char *file, int line, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_thread_yield();

    string_t s = STRING_INIT;
    string_appendf(&s, "%s:%d", file, line);

    if (format) {
        string_append_cstr(&s, ": ");
        string_vappendf(&s, format, va);
    }

    loge(string_data(&s));
    string_finish(&s);

    __t_fail_silent();
}

noreturn void
__t_fail_silent(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_FAIL);
}

void
__t_assert(const char *file, int line, bool cond, const char *cond_string)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_assertf(file, line, cond, cond_string, NULL);
}

void printflike(5, 6)
__t_assertf(const char *file, int line, bool cond, const char *cond_string,
            const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_assertfv(file, line, cond, cond_string, format, va);
    va_end(va);
}

void
__t_assertfv(const char *file, int line, bool cond, const char *cond_string,
             const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_thread_yield();

    if (cond)
        return;

    loge("%s:%d: assertion failed: %s", file, line, cond_string);

    if (format) {
        string_t s = STRING_INIT;
        string_appendf(&s, "%s:%d: ", file, line);
        string_vappendf(&s, format, va);
        loge(string_data(&s));
        string_finish(&s);
    }

    t_end(TEST_RESULT_FAIL);
}

static bool
t_compare_color_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    // Fail if the user accidentially tries to check the image in a non-image
    // test.
    t_assert(!t->def->no_image);

    assert(t->ref.width > 0);
    assert(t->ref.height > 0);

    cru_image_t *actual_image = t_new_cru_image_from_vk_image(t->vk.device,
            t->vk.queue, t->vk.color_image, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_ASPECT_COLOR, t->ref.width, t->ref.height,
            /*miplevel*/ 0, /*array_slice*/ 0);

    if (t->opt.bootstrap) {
        assert(!t->ref.image);
        t_assert(cru_image_write_file(actual_image,
                                      string_data(&t->ref.filename)));
        return true;
    }

    assert(t->ref.image);

    if (!cru_image_compare(actual_image, t->ref.image)) {
        loge("actual and reference images differ");

        // Dump the actual image for inspection.
        //
        // FINISHME: Dump the image diff too.
        string_t actual_path = STRING_INIT;
        string_copy(&actual_path, cru_prefix_path());
        path_append_cstr(&actual_path, "data");
        path_append_cstr(&actual_path, t_name);
        string_append_cstr(&actual_path, ".actual.png");
        cru_image_write_file(actual_image, string_data(&actual_path));

        return false;
    }

    return true;
}

static bool
t_compare_stencil_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    // Fail if the user accidentially tries to check the image in a non-image
    // test.
    t_assert(!t->def->no_image);

    assert(t->ref.width > 0);
    assert(t->ref.height > 0);

    const cru_format_info_t *finfo = t_format_info(t->def->depthstencil_format);

    cru_image_t *actual_image = t_new_cru_image_from_vk_image(t->vk.device,
            t->vk.queue, t->vk.ds_image, finfo->stencil_format,
            VK_IMAGE_ASPECT_STENCIL, t->ref.width, t->ref.height,
            /*miplevel*/ 0, /*array_slice*/ 0);

    if (t->opt.bootstrap) {
        assert(!t->ref.stencil_image);
        t_assert(cru_image_write_file(actual_image,
                                      string_data(&t->ref.stencil_filename)));
        return true;
    }

    assert(t->ref.stencil_image);

    if (!cru_image_compare(actual_image, t->ref.stencil_image)) {
        loge("actual and reference stencil images differ");

        // Dump the actual image for inspection.
        //
        // FINISHME: Dump the image diff too.
        string_t actual_path = STRING_INIT;
        string_copy(&actual_path, cru_prefix_path());
        path_append_cstr(&actual_path, "data");
        path_append_cstr(&actual_path, t_name);
        string_append_cstr(&actual_path, ".actual-stencil.png");
        cru_image_write_file(actual_image, string_data(&actual_path));

        return false;
    }

    return true;
}

/// Compare the test's rendered image against its reference image.
void
t_compare_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    t_thread_yield();

    bool ok = true;

    ok &= t_compare_color_image();
    ok &= t_compare_stencil_image();

    if (!ok) {
        // Fail silently because the aspect-specific comparison functions have
        // already logged the errors.
        t_fail_silent();
    }
}
