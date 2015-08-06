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

static void *
cru_alloc(void *pUserData, size_t size, size_t alignment,
          VkSystemAllocType allocType)
{
    void *mem = malloc(size);
    memset(mem, 139, size);
    return mem;
}

static void
cru_free(void *pUserData, void *pMem)
{
    free(pMem);
}

static const VkAllocCallbacks cru_alloc_cb = {
    .pfnAlloc = cru_alloc,
    .pfnFree = cru_free,
};

/// Find the best VkMemoryType whose properties contain each flag in
/// required_flags and contain no flag in the union of required_flags and
/// allowed_flags.
///
/// On success, return the type's index into the
/// VkPhysicalDeviceMemoryProperties::memoryTypes array.
/// On failure, return UINT32_MAX.
static uint32_t
find_best_mem_type_index(
        const VkPhysicalDeviceMemoryProperties *mem_props,
        VkMemoryPropertyFlags required_flags,
        VkMemoryPropertyFlags allowed_flags)
{
    uint32_t best_type_index = UINT32_MAX;
    VkMemoryHeap best_heap;

    allowed_flags |= required_flags;

    for (uint32_t i = 0; i < mem_props->memoryTypeCount; ++i) {
        VkMemoryType type = mem_props->memoryTypes[i];
        VkMemoryHeap heap = mem_props->memoryHeaps[type.heapIndex];

        if ((type.propertyFlags & required_flags) != required_flags)
            continue;

        if ((type.propertyFlags & ~allowed_flags) != 0)
            continue;

        // Prefer the type with the largest heap.
        if (best_type_index == UINT32_MAX || heap.size > best_heap.size) {
            best_type_index = i;
            best_heap = heap;
        }
    }

    return best_type_index;
}

static void
t_init_physical_dev(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    // Crucible uses only the first physical device.
    // FINISHME: Add a command-line option to use non-default physical device.

    uint32_t count = 0;
    qoEnumeratePhysicalDevices(t_instance, &count, NULL);
    t_assertf(count > 0, "failed to enumerate any physical devices");

    count = 1;
    qoEnumeratePhysicalDevices(t_instance, &count, &t->physical_dev);
    t_assertf(count == 1, "enumerated %u physical devices, expected 1", count);
}

static void
t_init_physical_dev_mem_props(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    qoGetPhysicalDeviceMemoryProperties(t->physical_dev,
                                        &t->physical_dev_mem_props);

    // The Vulkan spec (git aaed022) requires the implementation to expose at
    // least one host-visible and host-coherent memory type.
    t->mem_type_index_for_mmap = find_best_mem_type_index(
        &t->physical_dev_mem_props,
        /*require*/ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        /*allow*/ ~VK_MEMORY_PROPERTY_HOST_NON_COHERENT_BIT);

    // The best memory type for device-access is one which gives the best
    // performance, which is likely one that is device-visible but not
    // host-visible.
    t->mem_type_index_for_device_access = find_best_mem_type_index(
        &t->physical_dev_mem_props,
        /*require*/ VK_MEMORY_PROPERTY_DEVICE_ONLY,
        /*allow*/ VK_MEMORY_PROPERTY_DEVICE_ONLY);

    if (t->mem_type_index_for_device_access == UINT32_MAX) {
        // There exists no device-only memory type. For device-access, then,
        // simply prefer the overall "best" memory type.
        t->mem_type_index_for_device_access = find_best_mem_type_index(
            &t->physical_dev_mem_props,
            /*require*/ 0, /*allow*/ ~0);
    }

    t_assertf(t->mem_type_index_for_mmap != UINT32_MAX,
              "failed to find a host-visible, host-coherent VkMemoryType in "
              "VkPhysicalDeviceMemoryProperties");

    t_assert(t->mem_type_index_for_device_access != UINT32_MAX);
}

static void
t_create_attachment(VkDevice dev,
                    VkFormat format,
                    VkImageUsageFlags image_usage_flags,
                    VkImageAspect image_aspect,
                    uint32_t width, uint32_t height,
                    VkImage *out_image,
                    VkAttachmentView *out_attachment_view,
                    VkImageView *out_image_view)
{
    ASSERT_TEST_IN_SETUP_PHASE;

    if (format == VK_FORMAT_UNDEFINED) {
        *out_image = QO_NULL_IMAGE;
        *out_attachment_view = QO_NULL_ATTACHMENT_VIEW;
        *out_image_view = QO_NULL_IMAGE_VIEW;
        return;
    }

    t_assert(width > 0);
    t_assert(height > 0);

    *out_image = qoCreateImage(dev,
        .format = format,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
        .usage = image_usage_flags);

    VkDeviceMemory mem = qoAllocImageMemory(dev, *out_image,
        .memoryTypeIndex = t_mem_type_index_for_device_access);

    qoBindImageMemory(dev, *out_image, mem, /*offset*/ 0);

    *out_attachment_view = qoCreateAttachmentView(dev,
        .image = *out_image,
        .format = format);

    *out_image_view = qoCreateImageView(dev,
        .image = *out_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspect = image_aspect,
            .baseMipLevel = 0,
            .mipLevels = 1,
            .baseArraySlice = 0,
            .arraySize = 1,
        });
}

static void
t_create_framebuffer(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    VkAttachmentBindInfo bind_info[2];
    uint32_t att_count = 0;

    if (t->def->no_image)
        return;

    t_create_attachment(t->device, VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                        VK_IMAGE_ASPECT_COLOR,
                        t->width, t->height,
                        &t->rt_image,
                        &t->color_attachment_view,
                        &t->color_texture_view);

    bind_info[att_count++] = (VkAttachmentBindInfo) {
        .view = t->color_attachment_view,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    if (t->def->depthstencil_format != VK_FORMAT_UNDEFINED) {
        t_create_attachment(t->device, t->def->depthstencil_format,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_BIT,
                            VK_IMAGE_ASPECT_DEPTH,
                            t->width, t->height,
                            &t->ds_image,
                            &t->ds_attachment_view,
                            &t->depth_image_view);

        bind_info[att_count++] = (VkAttachmentBindInfo) {
            .view = t->ds_attachment_view,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }

    t->framebuffer = qoCreateFramebuffer(t_device,
        .width = t->width,
        .height = t->height,
        .attachmentCount = att_count,
        .pAttachments = bind_info);
}

static void
join_thread(pthread_t thread)
{
    int err;

    err = pthread_join(thread, NULL);
    if (err) {
        // Abort because there is no safe way to recover.
        loge("failed to join thread... abort!");
        abort();
    }
}

void
result_thread_join_others(test_t *t)
{
    ASSERT_IN_RESULT_THREAD(t);

    pthread_t *thread = NULL;

    while ((thread = cru_slist_pop(&t->threads))) {
        if (!pthread_equal(*thread, t->result_thread)) {
            join_thread(*thread);
        }

        free(thread);
    }
}

void
t_thread_yield(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (t->phase >= CRU_TEST_PHASE_PENDING_CLEANUP)
        t_thread_exit();
}

static void
test_stop(test_t *t)
{
    assert(t->phase < CRU_TEST_PHASE_STOPPED);
    t->phase = CRU_TEST_PHASE_STOPPED;
    pthread_cond_broadcast(&t->stop_cond);
}

void cru_noreturn
result_thread_enter_cleanup_phase(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_IN_RESULT_THREAD(t);

    int err;

    t->phase = CRU_TEST_PHASE_CLEANUP;

    if (t->opt.no_separate_cleanup_thread) {
        t->cleanup_thread = pthread_self();
        cleanup_thread_start(t);
    } else {
        err = pthread_create(&t->cleanup_thread, NULL, cleanup_thread_start, t);
        if (err) {
            loge("%s: failed to start cleanup thread", t->def->name);

            t->result = TEST_RESULT_FAIL;
            test_stop(t);

            t_thread_exit();
        }

        // The cleanup thread is now responsible for broadcasting the test result.
        // Broadcasting in the cleanup thread instead of here, in the result
        // thread, avoids the overhead of an extra context switch and
        // pthread_join().
    }

    t_thread_exit();
}

static void
thread_bind_to_test(test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    cru_cleanup_stack_t *cleanup;

    cleanup = cru_cleanup_create();
    if (!cleanup)
        goto fail_create_cleanup_stack;

    cru_slist_prepend_atomic(&t->cleanup_stacks, cleanup);

    current = (cru_current_test_t) {
        .test = t,
        .cleanup = cleanup,
    };

    ASSERT_IN_TEST_THREAD;

    // Die now if a different thread cancelled the test.
    t_thread_yield();

    return;

fail_create_cleanup_stack:
    // Without a cleanup stack, this is not a well-formed test thread. That
    // prevents us from calling any "t_*" functions, even t_end(). So give up
    // and die.
    loge("failed to create cleanup stack for test thread");
    abort();
}

static void
t_load_image_file(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    if (t->image)
        return;

    assert(!t->def->no_image);
    assert(t->ref_image_filename.len > 0);

    t->image = cru_image_load_file(string_data(&t->ref_image_filename));
    t_assert(t->image);

    t_cleanup_push(t->image);
    t->width = cru_image_get_width(t->image);
    t->height = cru_image_get_height(t->image);

    t_assert(t->width > 0);
    t_assert(t->height > 0);
}

void *
main_thread_start(void *arg)
{
    ASSERT_NOT_IN_TEST_THREAD;
    thread_bind_to_test((test_t *) arg);
    ASSERT_IN_TEST_THREAD;

    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    t_assertf(t->def->start, "test defines no start function");

    if (!t->opt.bootstrap && !t->def->no_image)
        t_load_image_file();

    vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pAppInfo = &(VkApplicationInfo) {
                .pAppName = "crucible",
                .apiVersion = 1,
            },
            .pAllocCb = &cru_alloc_cb,
        },
        &t->instance);
    t_cleanup_push_vk_instance(t_instance);

    t_init_physical_dev();
    t_init_physical_dev_mem_props();

    vkCreateDevice(t->physical_dev,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueRecordCount = 1,
            .pRequestedQueues = &(VkDeviceQueueCreateInfo) {
                .queueFamilyIndex = 0,
                .queueCount = 1,
            },
        },
        &t->device);
    t_cleanup_push_vk_device(t_device);

    vkGetDeviceQueue(t_device, 0, 0, &t->queue);

    t->pipeline_cache = qoCreatePipelineCache(t->device);

    t->dynamic_vp_state = qoCreateDynamicViewportState(t->device,
        .viewportAndScissorCount = 1,
        .pViewports = (VkViewport[]) {
            {
                .originX = 0,
                .originY = 0,
                .width = t->width,
                .height = t->height,
                .minDepth = 0,
                .maxDepth = 1
            },
        },
        .pScissors = (VkRect2D[]) {
            {{ 0, 0 }, { t->width, t->height }},
        }
    );
    t->dynamic_rs_state = qoCreateDynamicRasterState(t_device);
    t->dynamic_cb_state = qoCreateDynamicColorBlendState(t_device);
    t->dynamic_ds_state = qoCreateDynamicDepthStencilState(t_device);

    VkResult res = vkCreateCommandPool(t_device,
        &(VkCmdPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
            .flags = 0,
        }, &t->cmd_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(t->device, t->cmd_pool);

    t->cmd_buffer = qoCreateCommandBuffer(t_device, t_cmd_pool);

    qoBeginCommandBuffer(t_cmd_buffer);
    vkCmdBindDynamicViewportState(t->cmd_buffer, t->dynamic_vp_state);
    vkCmdBindDynamicRasterState(t->cmd_buffer, t->dynamic_rs_state);
    vkCmdBindDynamicColorBlendState(t->cmd_buffer, t->dynamic_cb_state);
    vkCmdBindDynamicDepthStencilState(t->cmd_buffer, t->dynamic_ds_state);

    t_create_framebuffer();

    t->phase = CRU_TEST_PHASE_MAIN;
    t->def->start();

    if (!t->def->no_image)
        t_compare_image();

    // Don't prematurely end the test before the test has completed executing.
    vkQueueWaitIdle(t->queue);

    // The framework must cancel the test when the test's start function
    // returns. Otherwise, test_wait() will deadlock waiting for test
    // cancellation unless the test created a subthread that cancels later.
    t_pass();
}

static void *
user_thread_start(void *_arg)
{
    user_thread_arg_t arg;

    // To avoid leaking the thread argument, we must copy it.
    arg = *(user_thread_arg_t*) _arg;
    free(_arg);

    // Connect this newly created thread to the test framework before giving
    // control to the user-supplied thread start function.
    ASSERT_NOT_IN_TEST_THREAD;
    thread_bind_to_test(arg.test);
    ASSERT_IN_TEST_THREAD;

    arg.start_func(arg.start_arg);
    t_thread_exit();
}

/// \brief The cleanup thread's start function.
///
/// The cleanup thread runs after all other test thread's have exited
/// and unwinds all the test's cleanup stacks.
///
/// \see test::cleanup_stacks
void *
cleanup_thread_start(void *arg)
{
    test_t *t = arg;
    cru_cleanup_stack_t *cleanup = NULL;

    ASSERT_IN_CLEANUP_THREAD(t);

    while ((cleanup = cru_slist_pop(&t->cleanup_stacks))) {
        if (t->opt.no_cleanup)
            cru_cleanup_pop_all_noop(cleanup);

        cru_cleanup_release(cleanup);
    }

    test_stop(t);

    return NULL;
}

bool
test_thread_create(test_t *t, void *(*start)(void *arg), void *arg,
                   pthread_t *out_thread)
{
    pthread_t *thread = NULL;
    int err;

    thread = xmalloc(sizeof(pthread_t));

    err = pthread_create(thread, NULL, start, arg);
    if (err) {
        loge("%s: failed to start thread", t->def->name);
        goto fail;
    }

    cru_slist_prepend_atomic(&t->threads, thread);

    if (out_thread)
        *out_thread = *thread;

    return true;

fail:
    free(thread);
    return false;
}

/// Create a new test thread. Must be called from an existing test thread.
///
/// TODO: Document join behavior. Users can't join test threads. Test runner
/// joins them.
void
t_thread_create(void (*start)(void *arg), void *arg)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    user_thread_arg_t *test_arg = NULL;

    assert(start);

    test_arg = xzalloc(sizeof(*test_arg));
    test_arg->test = t;
    test_arg->start_func = start;
    test_arg->start_arg = arg;

    if (!test_thread_create(t, user_thread_start, test_arg, NULL)) {
        t_fail_silent();
    }
}

/// All test threads must exit through this function.
void cru_noreturn
t_thread_exit(void)
{
    pthread_exit(NULL);
}
