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

    qoGetPhysicalDeviceProperties(t->vk.physical_dev, &t->vk.physical_dev_props);
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
t_setup_framebuffer(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    if (t->def->no_image)
        return;

    VkImageView attachments[2];
    uint32_t n_attachments = 0;

    t_assert(t->ref.width > 0);
    t_assert(t->ref.height > 0);

    t->vk.color_image = qoCreateImage(t->vk.device,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = {
            .width = t->ref.width,
            .height = t->ref.height,
            .depth = 1,
        },
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_SAMPLED_BIT |
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    VkDeviceMemory color_mem = qoAllocImageMemory(t->vk.device,
        t->vk.color_image,
        .memoryTypeIndex = t->vk.mem_type_index_for_device_access);

    qoBindImageMemory(t->vk.device, t->vk.color_image, color_mem,
                      /*offset*/ 0);

    t->vk.color_image_view = qoCreateImageView(t->vk.device,
        QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS,
        .image = t->vk.color_image,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        });

    attachments[n_attachments++] = t->vk.color_image_view;

    if (t->def->depthstencil_format != VK_FORMAT_UNDEFINED) {
        t->vk.ds_image = qoCreateImage(t->vk.device,
            .format = t->def->depthstencil_format,
            .extent = {
                .width = t->ref.width,
                .height = t->ref.height,
                .depth = 1,
            },
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

        VkDeviceMemory ds_mem = qoAllocImageMemory(t->vk.device,
            t->vk.ds_image,
            .memoryTypeIndex = t->vk.mem_type_index_for_device_access);

        qoBindImageMemory(t->vk.device, t->vk.ds_image, ds_mem,
                          /*offset*/ 0);

        t->vk.depthstencil_image_view = qoCreateImageView(t->vk.device,
            QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS,
            .image = t->vk.ds_image,
            .format = t->def->depthstencil_format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
                              VK_IMAGE_ASPECT_STENCIL_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            });

        attachments[n_attachments++] = t->vk.depthstencil_image_view;
    }

    t->vk.framebuffer = qoCreateFramebuffer(t->vk.device,
        .width = t->ref.width,
        .height = t->ref.height,
        .layers = 1,
        .attachmentCount = n_attachments,
        .pAttachments = attachments);
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
                .apiVersion = VK_API_VERSION,
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
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &(VkDeviceQueueCreateInfo) {
                .queueFamilyIndex = 0,
                .queueCount = 1,
            },
        }, &t->vk.device);

    t_cleanup_push_vk_device(t->vk.device);

    t_setup_framebuffer();

    vkGetDeviceQueue(t->vk.device, 0, 0, &t->vk.queue);

    t->vk.pipeline_cache = qoCreatePipelineCache(t->vk.device);

    VkResult res = vkCreateCommandPool(t->vk.device,
        &(VkCommandPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
            .flags = 0,
        }, &t->vk.cmd_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(t->vk.device, t->vk.cmd_pool);

    t->vk.cmd_buffer = qoCreateCommandBuffer(t->vk.device, t->vk.cmd_pool);

    qoBeginCommandBuffer(t->vk.cmd_buffer);
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
