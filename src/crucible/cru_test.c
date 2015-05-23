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

#include <fnmatch.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <crucible/cru_image.h>
#include <crucible/cru_log.h>
#include <crucible/cru_misc.h>
#include <crucible/cru_refcount.h>
#include <crucible/cru_slist.h>
#include <crucible/string.h>
#include <crucible/xalloc.h>

#include "cru_test.h"

typedef struct cru_test_thread_arg cru_test_thread_arg_t;

/// Tests proceed through the stages in the order listed.
enum cru_test_phase {
    CRU_TEST_PHASE_PRESTART,
    CRU_TEST_PHASE_SETUP,
    CRU_TEST_PHASE_MAIN,
    CRU_TEST_PHASE_CANCELLED,
    CRU_TEST_PHASE_CLEANUP,
    CRU_TEST_PHASE_DEAD,
};

struct cru_test {
    const cru_test_def_t *def;
    cru_slist_t *cleanup_stacks; ///< List of `cru_cleanup_stack_t *`.
    cru_slist_t *threads; ///< List of `pthread_t *`.

    /// Threads coordinate activity with the phase.
    _Atomic enum cru_test_phase phase;

    pthread_mutex_t result_mutex;
    pthread_cond_t result_cond;
    enum cru_test_result result;

    /// Run the test in bootstrap mode.
    bool bootstrap;

    /// Disable image dumps.
    ///
    /// \see t_dump_image()
    bool no_dump;

    /// Don't run the cleanup commands in cru_test::cleanup_stacks.
    bool no_cleanup;

    string_t ref_image_filename;
    cru_image_t *image;

    /// Atomic counter for t_dump_seq_image().
    cru_refcount_t dump_seq;

    uint32_t width;
    uint32_t height;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    VkCmdBuffer cmd_buffer;
    VkImage rt_image;
    VkColorAttachmentView image_color_view;
    VkImageView image_texture_view;
    VkFramebuffer framebuffer;
};

struct cru_test_thread_arg {
    cru_test_t *test;
    void (*start_func)(void *start_arg);
    void *start_arg;
};

static __thread cru_test_t *cru_current_test
    __attribute__((tls_model("local-exec")));

static __thread cru_cleanup_stack_t *cru_current_test_cleanup
    __attribute__((tls_model("local-exec")));

const char *
cru_test_result_to_string(cru_test_result_t result)
{
    switch (result) {
    case CRU_TEST_RESULT_PASS:
        return "pass";
    case CRU_TEST_RESULT_SKIP:
        return "skip";
    case CRU_TEST_RESULT_FAIL:
        return "fail";
    }

    cru_unreachable;
}

cru_test_t *
cru_test_get_current(void)
{
    return cru_current_test;
}

static void
cru_test_set_image_filename(cru_test_t *t)
{
    // Always define the reference image's filename, even when
    // cru_test_def_t::no_image is set. This will be useful for tests that
    // generate their reference images at runtime and wish to dump them to
    // disk.

    assert(t->phase == CRU_TEST_PHASE_PRESTART);
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

static void
cru_test_load_image_file(void)
{
    cru_test_t *t = cru_current_test;

    if (t->image)
        return;

    assert(!t->def->no_image);
    assert(t->ref_image_filename.len > 0);

    t->image = cru_image_load_file(t->ref_image_filename.buf);
    if (!t->image)
        t_fail();

    t_cleanup_push(t->image);
    t->width = cru_image_get_width(t->image);
    t->height = cru_image_get_height(t->image);

    assert(t->width > 0);
    assert(t->height > 0);
}

void
cru_test_destroy(cru_test_t *t)
{
    if (!t)
        return;

    assert(t->phase == CRU_TEST_PHASE_PRESTART ||
           t->phase == CRU_TEST_PHASE_DEAD);

    pthread_mutex_destroy(&t->result_mutex);
    pthread_cond_destroy(&t->result_cond);
    string_finish(&t->ref_image_filename);
    free(t);
}

cru_test_t *
cru_test_create(const cru_test_def_t *def)
{
    cru_test_t *t = NULL;
    int err;

    t = xzalloc(sizeof(*t));
    t->def = def;
    t->phase = CRU_TEST_PHASE_PRESTART;
    t->result = CRU_TEST_RESULT_PASS;
    t->ref_image_filename = STRING_INIT;
    t->no_dump = true;

    // Disable the cleanup handlers because they consitently crash Mesa.
    // TODO: Enable cleanup after Mesa is fixed.
    t->no_cleanup = true;

    if (t->def->samples > 0) {
        cru_loge("%s: multisample tests not yet supported", t->def->name);
        goto fail;
    }

    err = pthread_mutex_init(&t->result_mutex, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized mutex later.
        cru_loge("%s: failed to init mutex during test creation",
                 t->def->name);
        abort();
    }

    err = pthread_cond_init(&t->result_cond, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized cond later.
        cru_loge("%s: failed to init thread condition during test creation",
                 t->def->name);
        abort();
    }

    cru_test_set_image_filename(t);

    return t;

fail:
    t->result = CRU_TEST_PHASE_DEAD;
    t->result = CRU_TEST_RESULT_FAIL;
    return t;
}

bool
cru_test_enable_bootstrap(cru_test_t *t,
                          uint32_t image_width, uint32_t image_height)
{
    assert(t->phase == CRU_TEST_PHASE_PRESTART);
    assert(!cru_current_test);

    if (!t->def->no_image && (image_width == 0 || image_height == 0)) {
        cru_loge("%s: bootstrap image must have non-zero size", t->def->name);
        return false;
    }

    t->bootstrap = true;
    t->no_cleanup = true;
    t->width = image_width;
    t->height = image_height;

    return true;
}

void
cru_test_enable_dump(cru_test_t *t, bool enable)
{
    assert(t->phase == CRU_TEST_PHASE_PRESTART);
    assert(!cru_current_test);

    t->no_dump = !enable;
}

bool
cru_test_disable_cleanup(cru_test_t *t)
{
    assert(t->phase == CRU_TEST_PHASE_PRESTART);
    assert(!cru_current_test);

    t->no_cleanup = true;
    return true;
}

const VkInstance *
__t_instance(void)
{
    return &cru_current_test->instance;
}

const VkDevice *
__t_device(void)
{
    return &cru_current_test->device;
}

const VkPhysicalDevice *
__t_physical_device(void)
{
    return &cru_current_test->physical_device;
}

const VkQueue *
__t_queue(void)
{
    return &cru_current_test->queue;
}

const VkCmdBuffer *
__t_cmd_buffer(void)
{
    return &cru_current_test->cmd_buffer;
}

const VkImage *
__t_image(void)
{
    t_assert(!cru_current_test->def->no_image);
    return &cru_current_test->rt_image;
}

const VkColorAttachmentView *
__t_image_color_view(void)
{
    t_assert(!cru_current_test->def->no_image);
    return &cru_current_test->image_color_view;
}

const VkImageView *
__t_image_texture_view(void)
{
    t_assert(!cru_current_test->def->no_image);
    return &cru_current_test->image_texture_view;
}

const VkFramebuffer *
__t_framebuffer(void)
{
    t_assert(!cru_current_test->def->no_image);
    return &cru_current_test->framebuffer;
}

const uint32_t *
__t_height(void)
{
    t_assert(!cru_current_test->def->no_image);
    return &cru_current_test->height;
}

const uint32_t *
__t_width(void)
{
    t_assert(!cru_current_test->def->no_image);
    return &cru_current_test->width;
}

const char *
__t_name(void)
{
    return cru_current_test->def->name;
}

const void *
__t_user_data(void)
{
    return cru_current_test->def->user_data;
}

cru_image_t *
t_ref_image(void)
{
    t_assert(!cru_current_test->def->no_image);
    return cru_current_test->image;
}

/// Illegal to call before cru_test_wait().
cru_test_result_t
cru_test_get_result(cru_test_t *t)
{
    assert(t->phase == CRU_TEST_PHASE_DEAD);
    return t->result;
}

void
t_check_cancelled(void)
{
    cru_test_t *t = cru_current_test;

    // If this is called outside the range of phases below, then it's not
    // a test error but a framework bug.
    assert(t->phase >= CRU_TEST_PHASE_SETUP);
    assert(t->phase <= CRU_TEST_PHASE_CANCELLED);

    if (t->phase == CRU_TEST_PHASE_CANCELLED)
        pthread_exit(NULL);
}

void
t_end(enum cru_test_result result)
{
    cru_test_t *t = cru_current_test;
    int err;

    // If this is called outside the range of phases below, then it's not
    // a test error but a framework bug.
    assert(t->phase >= CRU_TEST_PHASE_SETUP);
    assert(t->phase <= CRU_TEST_PHASE_CANCELLED);

    if (t->phase == CRU_TEST_PHASE_CANCELLED) {
        // A previous call to cru_test_end already cancelled the test and set
        // the test result.
        pthread_exit(NULL);
    }

    err = pthread_mutex_lock(&t->result_mutex);
    if (err) {
        cru_loge("%s: failed to lock test mutex", t->def->name);
        abort();
    }

    if (t->phase >= CRU_TEST_PHASE_CANCELLED) {
        err = pthread_mutex_unlock(&t->result_mutex);
        if (err) {
            cru_loge("%s: failed to unlock mutex", t->def->name);
            abort();
        }

        // A previous call to cru_test_end already cancelled the test and set
        // the test result.
        pthread_exit(NULL);
    }

    t->phase = CRU_TEST_PHASE_CANCELLED;
    t->result = result;

    err = pthread_mutex_unlock(&t->result_mutex);
    if (err) {
        cru_loge("%s: failed to unlock test mutex", t->def->name);
        abort();
    }

    // FINISHME: Run the cleanup handlers here if this is the last test thread.
    pthread_cond_broadcast(&t->result_cond);
    pthread_exit(NULL);
}

void
t_dump_seq_image(cru_image_t *image)
{
    cru_test_t *t = cru_current_test;

    if (t->no_dump)
        return;

    uint64_t seq = cru_refcount_get(&t->dump_seq);
    if (seq > 9999) {
        cru_loge("image sequence count exceeds 9999");
        t_fail();
    }

    string_t filename = STRING_INIT;
    string_printf(&filename, "%s.seq%04lu.png", t_name, seq);
    cru_image_write_file(image, filename.buf);
}

void cru_printflike(2, 3)
t_dump_image_f(cru_image_t *image, const char *format, ...)
{
    va_list va;

    va_start(va, format);
    t_dump_image_fv(image, format, va);
    va_end(va);
}

void
t_dump_image_fv(cru_image_t *image, const char *format, va_list va)
{
    cru_test_t *t = cru_current_test;

    if (t->no_dump)
        return;

    string_t filename = STRING_INIT;
    string_appendf(&filename, t_name);
    string_append_char(&filename, '.');
    string_vappendf(&filename, format, va);

    cru_image_write_file(image, filename.buf);
}

void
t_compare_image(void)
{
    cru_test_t *t = cru_current_test;

    t_check_cancelled();

    // Fail if the user accidentially tries to check the image in a non-image
    // test.
    t_assert(!t->def->no_image);

    assert(t->width > 0);
    assert(t->height > 0);

    VkBuffer buffer;
    vkCreateBuffer(t_device,
        &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 4 * t_width * t_height,
            .usage = VK_BUFFER_USAGE_TRANSFER_DESTINATION_BIT,
            .flags = 0
        },
        &buffer);

    VkMemoryRequirements buffer_reqs;
    size_t buffer_reqs_size = sizeof(buffer_reqs);
    vkGetObjectInfo(t_device, VK_OBJECT_TYPE_BUFFER, buffer,
                    VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS,
                    &buffer_reqs_size, &buffer_reqs);

    size_t mem_size = buffer_reqs.size;

    VkDeviceMemory mem;
    vkAllocMemory(t_device,
        &(VkMemoryAllocInfo) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO,
            .allocationSize = mem_size,
            .memProps = VK_MEMORY_PROPERTY_HOST_DEVICE_COHERENT_BIT,
            .memPriority = VK_MEMORY_PRIORITY_NORMAL,
        },
        &mem);

    void *map;
    vkMapMemory(t_device, mem, 0, mem_size, 0, &map);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER, buffer,
                            /*index*/ 0, mem, /*offset*/ 0);

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

    vkCmdCopyImageToBuffer(t_cmd_buffer, t_image,
                           VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &copy);
    vkEndCommandBuffer(t_cmd_buffer);
    vkQueueSubmit(t_queue, 1, &t_cmd_buffer, 0);
    vkQueueWaitIdle(t_queue);

    cru_image_t *actual_image =
        cru_image_from_pixels(map, VK_FORMAT_R8G8B8A8_UNORM,
                              t_width, t_height);
    t_assert(actual_image);
    t_cleanup_push(actual_image);

    if (t->bootstrap) {
        assert(!t->image);
        t_assert(cru_image_write_file(actual_image, t->ref_image_filename.buf));
        t_pass();
    }

    assert(t->image);

    if (!cru_image_compare(actual_image, t_ref_image())) {
        cru_loge("actual and reference images differ");

        // Dump the actual image for inspection.
        //
        // FINISHME: Add a cmdline flag to toggle image dumping.
        // FINISHME: Dump the image diff too.
        string_t actual_path = STRING_INIT;
        string_copy(&actual_path, cru_prefix_path());
        path_append_cstr(&actual_path, "data");
        path_append_cstr(&actual_path, t_name);
        string_append_cstr(&actual_path, ".actual.png");
        cru_image_write_file(actual_image, actual_path.buf);

        t_fail();
    }

    t_pass();
}

void
t_pass(void)
{
    t_end(CRU_TEST_RESULT_PASS);
}

void
t_skip(void)
{
    t_end(CRU_TEST_RESULT_SKIP);
}

void
t_fail(void)
{
    t_end(CRU_TEST_RESULT_FAIL);
}

void
__t_assert(const char *file, int line,
              bool cond, const char *cond_string)
{
    t_check_cancelled();

    if (!cond) {
        cru_loge("%s:%d: assertion failed: %s", file, line, cond_string);
        t_fail();
    }
}

static void
cru_test_setup_thread(cru_test_t *t)
{
    assert(t);
    assert(!cru_current_test);
    assert(!cru_current_test_cleanup);

    // Bind this thread to the test.
    cru_current_test = t;

    // Kill this thread if the test is already done.
    t_check_cancelled();

    // Create a cleanup stack for this thread.
    cru_current_test_cleanup = cru_cleanup_create();
    if (!cru_current_test_cleanup)
        goto fail_create_cleanup_stack;

    if (!cru_slist_prepend(&t->cleanup_stacks, cru_current_test_cleanup))
        goto fail_create_cleanup_stack;

    return;

fail_create_cleanup_stack:
    cru_loge("failed to create cleanup stack for test thread");

    if (cru_current_test_cleanup) {
        cru_cleanup_release(cru_current_test_cleanup);
        cru_current_test_cleanup = NULL;
    }

    t_fail();
}

static void *
t_vk_alloc(void *user_data, size_t size, size_t alignment,
           VkSystemAllocType type)
{
    return xmalloc(size);
}

static void
t_vk_free(void* user_data, void* mem)
{
    free(mem);
}

static void *
cru_test_start_main_thread(void *arg)
{
    cru_test_t *t = arg;
    size_t size;
    VkResult res;

    assert(t->phase == CRU_TEST_PHASE_SETUP);
    cru_test_setup_thread(t);

    if (!t->def->start) {
       cru_loge("test defines no start function");
       t_fail();
    }

    if (!t->bootstrap && !t->def->no_image)
        cru_test_load_image_file();

    vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pAllocCb = &(VkAllocCallbacks) {
                .pfnAlloc = t_vk_alloc,
                .pfnFree = t_vk_free,
            },
            .pAppInfo = &(VkApplicationInfo) {
                .pAppName = "crucible",
                .apiVersion = 1,
            },
        },
        &t->instance);
    t_cleanup_push_vk_instance(t_instance);

    // Crucible uses only the first physical device.
    // FINISHME: Add command-line options to use non-default physical devices.
    uint32_t physical_device_count = 1;
    res = vkEnumeratePhysicalDevices(t_instance, &physical_device_count,
                                     &t->physical_device);
    t_assert(res == VK_SUCCESS);
    t_assert(physical_device_count == 1);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_PHYSICAL_DEVICE,
                             t_physical_device);

    vkCreateDevice(t->physical_device,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueRecordCount = 1,
            .pRequestedQueues = &(VkDeviceQueueCreateInfo) {
                .queueNodeIndex = 0,
                .queueCount = 1,
            },
        },
        &t->device);
    t_cleanup_push_vk_device(t_device);

    vkGetDeviceQueue(t_device, 0, 0, &t->queue);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_QUEUE,
                                    t_queue);

    vkCreateCommandBuffer(t_device,
        &(VkCmdBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO,
            .queueNodeIndex = 0,
            .flags = 0,
         },
        &t->cmd_buffer);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_COMMAND_BUFFER,
                                    t_cmd_buffer);

    vkBeginCommandBuffer(t_cmd_buffer,
        &(VkCmdBufferBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO,
            .flags = 0,
        });

    if (!t->def->no_image) {
        assert(t_width > 0);
        assert(t_height > 0);

        vkCreateImage(t_device,
            &(VkImageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = {
                    .width = t_width,
                    .height = t_height,
                    .depth = 1,
                },
                .mipLevels = 1,
                .arraySize = 1,
                .samples = 1,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .flags = 0,
            },
            &t->rt_image);
        t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_IMAGE,
                                 t_image);

        VkMemoryRequirements rt_mem_reqs;
        size = sizeof(rt_mem_reqs);
        vkGetObjectInfo(t_device, VK_OBJECT_TYPE_IMAGE, t_image,
                        VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS,
                        &size, &rt_mem_reqs);

        VkDeviceMemory rt_mem;
        vkAllocMemory(t_device,
            &(VkMemoryAllocInfo) {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO,
                .allocationSize = rt_mem_reqs.size,
                .memProps = VK_MEMORY_PROPERTY_HOST_DEVICE_COHERENT_BIT,
                .memPriority = VK_MEMORY_PRIORITY_NORMAL,
            },
            &rt_mem);
        t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_DEVICE_MEMORY,
                                 rt_mem);

        vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_IMAGE, t_image,
                                /*allocationIndex*/ 0, rt_mem,
                                /*offset*/ 0);

        vkCreateColorAttachmentView(t_device,
            &(VkColorAttachmentViewCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_COLOR_ATTACHMENT_VIEW_CREATE_INFO,
                .image = t_image,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .mipLevel = 0,
                .baseArraySlice = 0,
                .arraySize = 1,
                .msaaResolveImage = 0,
                .msaaResolveSubResource = {0},
            },
            &t->image_color_view);
        t_cleanup_push_vk_object(t_device,
                                 VK_OBJECT_TYPE_COLOR_ATTACHMENT_VIEW,
                                 t_image_color_view);

        vkCreateImageView(t_device,
            &(VkImageViewCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = t_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .channels = {
                    VK_CHANNEL_SWIZZLE_R,
                    VK_CHANNEL_SWIZZLE_G,
                    VK_CHANNEL_SWIZZLE_B,
                    VK_CHANNEL_SWIZZLE_A,
                },
                .subresourceRange = {
                    .aspect = VK_IMAGE_ASPECT_COLOR,
                    .baseMipLevel = 0,
                    .mipLevels = 1,
                    .baseArraySlice = 0,
                    .arraySize = 1,
                },
                .minLod = 0,
            },
            &t->image_texture_view);
        t_cleanup_push_vk_object(t_device,
                                 VK_OBJECT_TYPE_IMAGE_VIEW,
                                 t_image_texture_view);

        vkCreateFramebuffer(t_device,
            &(VkFramebufferCreateInfo) {
                .colorAttachmentCount = 1,
                .pColorAttachments = (VkColorAttachmentBindInfo[]) {
                    {
                        .view = t_image_color_view,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    },
                },
                .pDepthStencilAttachment = NULL,
                .sampleCount = 1,
                .width = t_width,
                .height = t_height,
                .layers = 1,
            },
            &t->framebuffer);
        t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_FRAMEBUFFER,
                                 t_framebuffer);
    }

    t->phase = CRU_TEST_PHASE_MAIN;
    t->def->start();

    if (!t->def->no_image)
        t_compare_image();

    // The framework must cancel the test when the test's start function
    // returns. Otherwise, cru_test_wait() will deadlock waiting for test
    // cancellation unless the test created a subthread that cancels later.
    t_pass();
}

static void *
cru_test_start_user_thread(void *arg)
{
    cru_test_thread_arg_t test_arg;

    test_arg = *(cru_test_thread_arg_t*) arg;
    free(arg);

    cru_test_setup_thread(test_arg.test);
    test_arg.start_func(test_arg.start_arg);

    return NULL;
}

static void *
cru_test_start_cleanup_thread(void *arg)
{
    cru_test_t *t = arg;
    cru_cleanup_stack_t *cleanup = NULL;

    // The cleanup thread executes outside the test.
    assert(!cru_current_test);
    assert(!cru_current_test_cleanup);

    while ((cleanup = cru_slist_pop(&t->cleanup_stacks))) {
        if (t->no_cleanup)
            cru_cleanup_pop_all_noop(cleanup);

        cru_cleanup_release(cleanup);
    }

    return NULL;
}

void
cru_test_start(cru_test_t *t)
{
    pthread_t *start_thread = NULL;
    int err;

    assert(t->phase == CRU_TEST_PHASE_PRESTART);
    t->phase = CRU_TEST_PHASE_SETUP;

    if (t->def->skip) {
        t->phase = CRU_TEST_PHASE_DEAD;
        t->result = CRU_TEST_RESULT_SKIP;
        return;
    }

    start_thread = xmalloc(sizeof(pthread_t));
    err = pthread_create(start_thread, NULL,
                         cru_test_start_main_thread, t);
    if (err) {
        cru_loge("%s: failed to start main test thread", t->def->name);
        goto fail;
    }

    if (!cru_slist_prepend(&t->threads, start_thread)) {
        cru_loge("%s: leaked the main test thread", t->def->name);
        pthread_cancel(*start_thread);
        goto fail;
    }

    return;

fail:
    t->phase = CRU_TEST_PHASE_DEAD;
    t->result = CRU_TEST_RESULT_FAIL;

    free(start_thread);
}

/// Create a new test thread. Must be called from an existing test thread.
///
/// TODO: Document join behavior. Users can't join test threads. Test runner
/// joins them.
void
t_create_thread(void (*start)(void *arg), void *arg)
{
    cru_test_thread_arg_t *test_arg = NULL;
    pthread_t *thread = NULL;
    int err;

    // Must be called from an existing test thread.
    assert(cru_current_test);
    assert(start);

    test_arg = xzalloc(sizeof(*test_arg));
    test_arg->test = cru_current_test;
    test_arg->start_func = start;
    test_arg->start_arg = arg;

    thread = xmalloc(sizeof(pthread_t));
    err = pthread_create(thread, NULL, cru_test_start_user_thread, test_arg);
    if (err) {
        cru_loge("%s() failed to create thread", __func__);
        goto fail;
    }

    if (!cru_slist_prepend(&cru_current_test->threads, thread)) {
        cru_loge("%s() leaked the new thread", __func__);
        pthread_cancel(*thread);
        goto fail;
    }

    return;

fail:
    free(thread);
    t_fail();
}

static void
cru_test_join_threads(cru_test_t *t)
{
    int err;
    pthread_t *thread = NULL;

    // Test threads cannot join themselves.
    assert(!cru_current_test);
    assert(t->phase == CRU_TEST_PHASE_CANCELLED);

    while ((thread = cru_slist_pop(&t->threads))) {
        err = pthread_join(*thread, NULL);
        if (err) {
            cru_loge("%s: failed to join a test thread", t->def->name);
            t->result = CRU_TEST_RESULT_FAIL;
        }

        free(thread);
    }
}

static void
cru_test_run_cleanup_handlers(cru_test_t *t)
{
    pthread_t cleanup_thread;
    int err;

    // TODO: Document thread-safety of cleanup handlers.
    // TODO: Document that cleanup handlers run outside of test.
    // TODO: Maybe fail test if cleanup handler tries to access test data.

    // Cleanup handlers are ran by the runner, outside the test itself.
    assert(!cru_current_test);
    assert(t->phase == CRU_TEST_PHASE_CLEANUP);

    err = pthread_create(&cleanup_thread, NULL, cru_test_start_cleanup_thread, t);
    if (err) {
        cru_loge("%s: failed to create cleanup thread", t->def->name);
        t->result = CRU_TEST_RESULT_FAIL;
        return;
    }

    err = pthread_join(cleanup_thread, NULL);
    if (err) {
        cru_loge("%s: failed to join cleanup thread", t->def->name);
        t->result = CRU_TEST_RESULT_FAIL;
        return;
    }
}

/// Caller must have `test->result_mutex`.
static void
cru_test_wait_for_phase(cru_test_t *t, enum cru_test_phase phase)
{
    int err;

    while (t->phase < phase) {
        err = pthread_cond_wait(&t->result_cond, &t->result_mutex);
        if (err) {
            cru_loge("%s: failed to wait on thread condition", t->def->name);
            abort();
        }
    }
}

void
cru_test_wait(cru_test_t *t)
{
    int err;

    if (t->phase >= CRU_TEST_PHASE_DEAD)
        return;

    err = pthread_mutex_lock(&t->result_mutex);
    if (err) {
        cru_loge("%s: failed to lock test mutex", t->def->name);
        abort();
    }

    cru_test_wait_for_phase(t, CRU_TEST_PHASE_CANCELLED);

    if (t->phase == CRU_TEST_PHASE_CANCELLED) {
        cru_test_join_threads(t);
        t->phase = CRU_TEST_PHASE_CLEANUP;
        cru_test_run_cleanup_handlers(t);
        t->phase = CRU_TEST_PHASE_DEAD;
    }

    cru_test_wait_for_phase(t, CRU_TEST_PHASE_DEAD);

    pthread_mutex_unlock(&t->result_mutex);
}

void
t_cleanup_push_command(enum cru_cleanup_cmd cmd, ...)
{
    va_list va;

    va_start(va, cmd);
    t_cleanup_push_commandv(cmd, va);
    va_end(va);
}

void
t_cleanup_push_commandv(enum cru_cleanup_cmd cmd, va_list va)
{
    assert(cru_current_test_cleanup);
    cru_cleanup_push_commandv(cru_current_test_cleanup, cmd, va);
}

void
t_cleanup_pop(void)
{
    assert(cru_current_test_cleanup);
    cru_cleanup_pop(cru_current_test_cleanup);
}

void
t_cleanup_pop_all(void)
{
    cru_cleanup_pop_all(cru_current_test_cleanup);
}

bool
cru_test_def_match(const cru_test_def_t *def, const char *glob)
{
    return fnmatch(glob, def->name, 0) == 0;
}

uint32_t
cru_num_defs(void)
{
    return &__stop_cru_test_defs - &__start_cru_test_defs;
}

const cru_test_def_t *
cru_find_def(const char *name)
{
    const cru_test_def_t *def;

    cru_foreach_test_def(def) {
        if (cru_streq(def->name, name)) {
            return def;
        }
    }

    return NULL;
}
