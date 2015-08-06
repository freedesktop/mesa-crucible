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

void cru_noreturn
t_end(enum test_result result)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (atomic_exchange(&t->phase, CRU_TEST_PHASE_PENDING_CLEANUP)
        >= CRU_TEST_PHASE_PENDING_CLEANUP) {
        // A previous call to test_end already cancelled the test and set
        // the test result.
        t_thread_exit();
    }

    // This thread wins! It now unbinds itself from the test and becomes the
    // "result" thread.
    ASSERT_IN_TEST_THREAD;
    current = (cru_current_test_t) {0};
    t->result = result;
    t->result_thread = pthread_self();
    t->phase = CRU_TEST_PHASE_PENDING_CLEANUP;
    ASSERT_NOT_IN_TEST_THREAD;

    result_thread_join_others(t);

    // This thread, the "result" thread, is the test's sole remaining thread.
    assert(cru_slist_length(t->threads) == 0);

    result_thread_enter_cleanup_phase(t);
}

void cru_noreturn
t_pass(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_PASS);
}

void cru_noreturn
__t_skip(const char *file, int line)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_skipf(file, line, NULL);
}

void cru_noreturn
__t_skipf(const char *file, int line, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_skipfv(file, line, format, va);
    va_end(va);
}

void cru_noreturn
__t_skipfv(const char *file, int line, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_check_cancelled();

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

void cru_noreturn
__t_skip_silent(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_SKIP);
}

void cru_noreturn
__t_fail(const char *file, int line)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_failf(file, line, NULL);
}

void cru_noreturn
__t_failf(const char *file, int line, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_failfv(file, line, format, va);
    va_end(va);
}

void cru_noreturn
__t_failfv(const char *file, int line, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_check_cancelled();

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

void cru_noreturn
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

void cru_printflike(5, 6)
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
    t_check_cancelled();

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

void
t_compare_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_check_cancelled();

    // Fail if the user accidentially tries to check the image in a non-image
    // test.
    t_assert(!t->def->no_image);

    assert(t->width > 0);
    assert(t->height > 0);

    size_t buffer_size = 4 * t_width * t_height;

    VkBuffer buffer = qoCreateBuffer(t->device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DESTINATION_BIT);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    void *map = qoMapMemory(t_device, mem, /*offset*/ 0,
                            buffer_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, buffer, mem, /*offset*/ 0);

    VkBufferImageCopy copy = {
        .bufferOffset = 0,
        .imageSubresource = {
            .aspect = VK_IMAGE_ASPECT_COLOR,
            .mipLevel = 0,
            .arraySlice = 0,
        },
        .imageOffset = { .x = 0, .y = 0, .z = 0 },
        .imageExtent = {
            .width = t_width,
            .height = t_height,
            .depth = 1,
        },
    };

    VkCmdBuffer cmd = qoCreateCommandBuffer(t->device, t->cmd_pool);
    qoBeginCommandBuffer(cmd);
    vkCmdCopyImageToBuffer(cmd, t_color_image,
                           VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &copy);
    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
    vkQueueWaitIdle(t_queue);

    cru_image_t *actual_image =
        cru_image_from_pixels(map, VK_FORMAT_R8G8B8A8_UNORM,
                              t_width, t_height);
    t_assert(actual_image);
    t_cleanup_push(actual_image);

    if (t->opt.bootstrap) {
        assert(!t->image);
        t_assert(cru_image_write_file(actual_image,
                                      string_data(&t->ref_image_filename)));
        t_pass();
    }

    assert(t->image);

    if (!cru_image_compare(actual_image, t_ref_image())) {
        loge("actual and reference images differ");

        // Dump the actual image for inspection.
        //
        // FINISHME: Add a cmdline flag to toggle image dumping.
        // FINISHME: Dump the image diff too.
        string_t actual_path = STRING_INIT;
        string_copy(&actual_path, cru_prefix_path());
        path_append_cstr(&actual_path, "data");
        path_append_cstr(&actual_path, t_name);
        string_append_cstr(&actual_path, ".actual.png");
        cru_image_write_file(actual_image, string_data(&actual_path));

        t_fail_silent();
    }

    t_pass();
}
