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

static void *
test_vk_alloc(void *pUserData, size_t size, size_t alignment,
             VkSystemAllocType allocType)
{
    void *mem = malloc(size);
    memset(mem, 139, size);
    return mem;
}

static void
test_vk_free(void *pUserData, void *pMem)
{
    free(pMem);
}

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
t_setup_phys_dev(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    // Crucible uses only the first physical device.
    // FINISHME: Add a command-line option to use non-default physical device.

    uint32_t count = 0;
    qoEnumeratePhysicalDevices(t->vk.instance, &count, NULL);
    t_assertf(count > 0, "failed to enumerate any physical devices");

    count = 1;
    qoEnumeratePhysicalDevices(t->vk.instance, &count, &t->vk.physical_dev);
    t_assertf(count == 1, "enumerated %u physical devices, expected 1", count);
}

static void
t_setup_phys_dev_mem_props(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    qoGetPhysicalDeviceMemoryProperties(t->vk.physical_dev,
                                        &t->vk.physical_dev_mem_props);

    // The Vulkan spec (git aaed022) requires the implementation to expose at
    // least one host-visible and host-coherent memory type.
    t->vk.mem_type_index_for_mmap = find_best_mem_type_index(
        &t->vk.physical_dev_mem_props,
        /*require*/ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        /*allow*/ ~VK_MEMORY_PROPERTY_HOST_NON_COHERENT_BIT);

    // The best memory type for device-access is one which gives the best
    // performance, which is likely one that is device-visible but not
    // host-visible.
    t->vk.mem_type_index_for_device_access = find_best_mem_type_index(
        &t->vk.physical_dev_mem_props,
        /*require*/ VK_MEMORY_PROPERTY_DEVICE_ONLY,
        /*allow*/ VK_MEMORY_PROPERTY_DEVICE_ONLY);

    if (t->vk.mem_type_index_for_device_access == UINT32_MAX) {
        // There exists no device-only memory type. For device-access, then,
        // simply prefer the overall "best" memory type.
        t->vk.mem_type_index_for_device_access = find_best_mem_type_index(
            &t->vk.physical_dev_mem_props,
            /*require*/ 0, /*allow*/ ~0);
    }

    t_assertf(t->vk.mem_type_index_for_mmap != UINT32_MAX,
              "failed to find a host-visible, host-coherent VkMemoryType in "
              "VkPhysicalDeviceMemoryProperties");

    t_assert(t->vk.mem_type_index_for_device_access != UINT32_MAX);
}

static void
t_setup_attachment(VkDevice dev,
                   VkFormat format,
                   VkImageUsageFlags image_usage_flags,
                   uint32_t width, uint32_t height,
                   VkImage *out_image,
                   VkAttachmentView *out_attachment_view)
{
    GET_CURRENT_TEST(t);
    ASSERT_TEST_IN_SETUP_PHASE;

    if (format == VK_FORMAT_UNDEFINED) {
        *out_image = QO_NULL_IMAGE;
        *out_attachment_view = QO_NULL_ATTACHMENT_VIEW;
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
        .memoryTypeIndex = t->vk.mem_type_index_for_device_access);

    qoBindImageMemory(dev, *out_image, mem, /*offset*/ 0);

    *out_attachment_view = qoCreateAttachmentView(dev,
        .image = *out_image,
        .format = format);
}

static void
t_setup_framebuffer(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    VkAttachmentBindInfo bind_info[2];
    uint32_t att_count = 0;

    if (t->def->no_image)
        return;

    t_setup_attachment(t->vk.device, VK_FORMAT_R8G8B8A8_UNORM,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       t->ref.width, t->ref.height,
                       &t->vk.color_image,
                       &t->vk.color_attachment_view);

    bind_info[att_count++] = (VkAttachmentBindInfo) {
        .view = t->vk.color_attachment_view,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    if (t->def->depthstencil_format != VK_FORMAT_UNDEFINED) {
        t_setup_attachment(t->vk.device, t->def->depthstencil_format,
                           VK_IMAGE_USAGE_DEPTH_STENCIL_BIT,
                           t->ref.width, t->ref.height,
                           &t->vk.ds_image,
                           &t->vk.ds_attachment_view);

        bind_info[att_count++] = (VkAttachmentBindInfo) {
            .view = t->vk.ds_attachment_view,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }

    t->vk.framebuffer = qoCreateFramebuffer(t->vk.device,
        .width = t->ref.width,
        .height = t->ref.height,
        .attachmentCount = att_count,
        .pAttachments = bind_info);
}

static void
t_setup_image_views(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    if (t->vk.color_image.handle) {
        t->vk.color_image_view = qoCreateImageView(t->vk.device,
            .image = t->vk.color_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .subresourceRange = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
                .baseMipLevel = 0,
                .mipLevels = 1,
                .baseArraySlice = 0,
                .arraySize = 1,
            });
    }

    if (t->vk.ds_image.handle) {
        const cru_format_info_t *finfo =
            t_format_info(t->def->depthstencil_format);

        if (finfo->depth_format) {
            t->vk.depth_image_view = qoCreateImageView(t->vk.device,
                .image = t->vk.ds_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = finfo->depth_format,
                .subresourceRange = {
                    .aspect = VK_IMAGE_ASPECT_DEPTH,
                    .baseMipLevel = 0,
                    .mipLevels = 1,
                    .baseArraySlice = 0,
                    .arraySize = 1,
                });
        }

        if (finfo->stencil_format) {
            t->vk.stencil_image_view = qoCreateImageView(t->vk.device,
                .image = t->vk.ds_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = finfo->stencil_format,
                .subresourceRange = {
                    .aspect = VK_IMAGE_ASPECT_STENCIL,
                    .baseMipLevel = 0,
                    .mipLevels = 1,
                    .baseArraySlice = 0,
                    .arraySize = 1,
                });
        }
    }
}

void
t_setup_vulkan(void)
{
    GET_CURRENT_TEST(t);

    vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pAppInfo = &(VkApplicationInfo) {
                .pAppName = "crucible",
                .apiVersion = 1,
            },
            .pAllocCb = &(VkAllocCallbacks) {
                .pfnAlloc = test_vk_alloc,
                .pfnFree = test_vk_free,
            },
        }, &t->vk.instance);
    t_cleanup_push_vk_instance(t->vk.instance);

    t_setup_phys_dev();
    t_setup_phys_dev_mem_props();

    vkCreateDevice(t->vk.physical_dev,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueRecordCount = 1,
            .pRequestedQueues = &(VkDeviceQueueCreateInfo) {
                .queueFamilyIndex = 0,
                .queueCount = 1,
            },
        }, &t->vk.device);

    t_cleanup_push_vk_device(t->vk.device);

    t_setup_framebuffer();
    t_setup_image_views();

    vkGetDeviceQueue(t->vk.device, 0, 0, &t->vk.queue);

    t->vk.pipeline_cache = qoCreatePipelineCache(t->vk.device);

    t->vk.dynamic_vp_state = qoCreateDynamicViewportState(t->vk.device,
        .viewportAndScissorCount = 1,
        .pViewports = (VkViewport[]) {
            {
                .originX = 0,
                .originY = 0,
                .width = t->ref.width,
                .height = t->ref.height,
                .minDepth = 0,
                .maxDepth = 1
            },
        },
        .pScissors = (VkRect2D[]) {
            {{ 0, 0 }, { t->ref.width, t->ref.height }},
        }
    );
    t->vk.dynamic_rs_state = qoCreateDynamicRasterState(t->vk.device);
    t->vk.dynamic_cb_state = qoCreateDynamicColorBlendState(t->vk.device);
    t->vk.dynamic_ds_state = qoCreateDynamicDepthStencilState(t->vk.device);

    VkResult res = vkCreateCommandPool(t->vk.device,
        &(VkCmdPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
            .flags = 0,
        }, &t->vk.cmd_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(t->vk.device, t->vk.cmd_pool);

    t->vk.cmd_buffer = qoCreateCommandBuffer(t->vk.device, t->vk.cmd_pool);

    qoBeginCommandBuffer(t->vk.cmd_buffer);
    vkCmdBindDynamicViewportState(t->vk.cmd_buffer, t->vk.dynamic_vp_state);
    vkCmdBindDynamicRasterState(t->vk.cmd_buffer, t->vk.dynamic_rs_state);
    vkCmdBindDynamicColorBlendState(t->vk.cmd_buffer, t->vk.dynamic_cb_state);
    vkCmdBindDynamicDepthStencilState(t->vk.cmd_buffer, t->vk.dynamic_ds_state);
}

void
t_setup_ref_images(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    if (t->ref.image)
        return;

    assert(!t->def->no_image);
    assert(t->ref.filename.len > 0);

    t->ref.image = t_new_cru_image_from_filename(string_data(&t->ref.filename));

    t->ref.width = cru_image_get_width(t->ref.image);
    t->ref.height = cru_image_get_height(t->ref.image);

    t_assert(t->ref.width > 0);
    t_assert(t->ref.height > 0);

    if (t->def->ref_stencil_filename) {
        assert(t->ref.stencil_filename.len > 0);

        t->ref.stencil_image = t_new_cru_image_from_filename(
            string_data(&t->ref.stencil_filename));

        t_assert(t->ref.width == cru_image_get_width(t->ref.stencil_image));
        t_assert(t->ref.height == cru_image_get_height(t->ref.stencil_image));
    }
}
