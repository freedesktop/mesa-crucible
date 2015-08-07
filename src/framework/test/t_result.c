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

/// Compare the test's rendered image against its reference image.
void
t_compare_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_thread_yield();

    // Fail if the user accidentially tries to check the image in a non-image
    // test.
    t_assert(!t->def->no_image);

    assert(t->ref.width > 0);
    assert(t->ref.height > 0);

    size_t buffer_size = 4 * t->ref.width * t->ref.height;

    VkBuffer buffer = qoCreateBuffer(t->vk.device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DESTINATION_BIT);

    VkDeviceMemory mem = qoAllocBufferMemory(t->vk.device, buffer,
        .memoryTypeIndex = t->vk.mem_type_index_for_mmap);

    void *map = qoMapMemory(t->vk.device, mem, /*offset*/ 0,
                            buffer_size, /*flags*/ 0);

    qoBindBufferMemory(t->vk.device, buffer, mem, /*offset*/ 0);

    VkBufferImageCopy copy = {
        .bufferOffset = 0,
        .imageSubresource = {
            .aspect = VK_IMAGE_ASPECT_COLOR,
            .mipLevel = 0,
            .arraySlice = 0,
        },
        .imageOffset = { .x = 0, .y = 0, .z = 0 },
        .imageExtent = {
            .width = t->ref.width,
            .height = t->ref.height,
            .depth = 1,
        },
    };

    VkCmdBuffer cmd = qoCreateCommandBuffer(t->vk.device, t->vk.cmd_pool);
    qoBeginCommandBuffer(cmd);
    vkCmdCopyImageToBuffer(cmd, t->vk.color_image,
                           VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &copy);
    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t->vk.queue, 1, &cmd, QO_NULL_FENCE);
    vkQueueWaitIdle(t->vk.queue);

    cru_image_t *actual_image =
        cru_image_from_pixels(map, VK_FORMAT_R8G8B8A8_UNORM,
                              t->ref.width, t->ref.height);
    t_assert(actual_image);
    t_cleanup_push(actual_image);

    if (t->opt.bootstrap) {
        assert(!t->ref.image);
        t_assert(cru_image_write_file(actual_image,
                                      string_data(&t->ref.filename)));
        t_pass();
    }

    assert(t->ref.image);

    if (!cru_image_compare(actual_image, t->ref.image)) {
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
}
