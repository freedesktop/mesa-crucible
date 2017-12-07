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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "test.h"
#include "t_phase_setup.h"

static void *
test_vk_alloc(void *pUserData, size_t size, size_t alignment,
              VkSystemAllocationScope scope)
{
    assert(pUserData == (void *)0xdeadbeef);
    void *mem = malloc(size);
    memset(mem, 139, size);
    return mem;
}

static void *
test_vk_realloc(void *pUserData, void *pOriginal, size_t size,
                size_t alignment, VkSystemAllocationScope scope)
{
    assert(pUserData == (void *)0xdeadbeef);
    return realloc(pOriginal, size);
}

static void
test_vk_free(void *pUserData, void *pMem)
{
    assert(pUserData == (void *)0xdeadbeef);
    free(pMem);
}

static void
test_vk_dummy_notify(void *pUserData, size_t size,
                     VkInternalAllocationType allocationType,
                     VkSystemAllocationScope allocationScope)
{ }

static const VkAllocationCallbacks test_alloc_cb = {
    .pUserData = (void *)0xdeadbeef,
    .pfnAllocation = test_vk_alloc,
    .pfnReallocation = test_vk_realloc,
    .pfnFree = test_vk_free,
    .pfnInternalAllocation = test_vk_dummy_notify,
    .pfnInternalFree = test_vk_dummy_notify,
};

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
        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

    VkRenderPass color_pass = qoCreateRenderPass(t_device,
        .attachmentCount = 1,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorAttachmentCount = 1,
                .pColorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_GENERAL,
                    },
                },
            }
        });

    VkRenderPass pass = color_pass;

    if (t->def->depthstencil_format != VK_FORMAT_UNDEFINED) {
        VkFormatProperties depth_format_props;

        VkFormat format = t->def->depthstencil_format;

        vkGetPhysicalDeviceFormatProperties(t->vk.physical_dev,
                                               format,
                                               &depth_format_props);

        if (depth_format_props.optimalTilingFeatures == 0) {
            /* upgrade to a supported format */
            switch (format) {
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D16_UNORM:
                format = VK_FORMAT_D32_SFLOAT;
                break;
            case VK_FORMAT_D24_UNORM_S8_UINT:
                format = VK_FORMAT_D32_SFLOAT_S8_UINT;
                break;
            default:
                break;
            }
        }

        t->vk.ds_image = qoCreateImage(t->vk.device,
            .format = format,
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
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        qoBindImageMemory(t->vk.device, t->vk.ds_image, ds_mem,
                          /*offset*/ 0);

        VkImageAspectFlags aspect = 0;
        switch (t->def->depthstencil_format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        case VK_FORMAT_S8_UINT:
            aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        default:
            assert(!"Invalid depthstencil format");
        }

        t->vk.depthstencil_image_view = qoCreateImageView(t->vk.device,
            QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS,
            .image = t->vk.ds_image,
            .format = t->def->depthstencil_format,
            .subresourceRange = {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            });

        attachments[n_attachments++] = t->vk.depthstencil_image_view;

        VkRenderPass color_depth_pass = qoCreateRenderPass(t_device,
            .attachmentCount = 2,
            .pAttachments = (VkAttachmentDescription[]) {
                {
                    QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                    .format = VK_FORMAT_R8G8B8A8_UNORM,
                },
                {
                    QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                    .format = t->def->depthstencil_format,
                },
            },
            .subpassCount = 1,
            .pSubpasses = (VkSubpassDescription[]) {
                {
                    QO_SUBPASS_DESCRIPTION_DEFAULTS,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = (VkAttachmentReference[]) {
                        {
                            .attachment = 0,
                            .layout = VK_IMAGE_LAYOUT_GENERAL,
                        },
                    },
                    .pDepthStencilAttachment = &(VkAttachmentReference) {
                        .attachment = 1,
                        .layout = VK_IMAGE_LAYOUT_GENERAL,
                    },
                },
            });

        pass = color_depth_pass;
    }

    t->vk.framebuffer = qoCreateFramebuffer(t->vk.device,
        .renderPass = pass,
        .width = t->ref.width,
        .height = t->ref.height,
        .layers = 1,
        .attachmentCount = n_attachments,
        .pAttachments = attachments);
}

static void
t_setup_descriptor_pool(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    VkDescriptorPoolSize pool_sizes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; i++) {
        pool_sizes[i].type = i;
        pool_sizes[i].descriptorCount = 5;
    }

    const VkDescriptorPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 2,
        .poolSizeCount = VK_DESCRIPTOR_TYPE_RANGE_SIZE,
        .pPoolSizes = pool_sizes
    };

    VkResult res = vkCreateDescriptorPool(t->vk.device, &create_info, NULL,
                                          &t->vk.descriptor_pool);
    t_assert(res == VK_SUCCESS);
    t_assert(t->vk.descriptor_pool != VK_NULL_HANDLE);

    t_cleanup_push_vk_descriptor_pool(t->vk.device, t->vk.descriptor_pool);
}

static VkBool32 debug_cb(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char *pLayerPrefix,
    const char *pMessage,
    void *pUserData)
{
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        logi("object %"PRIu64" type 0x%x location %zu code %u layer \"%s\" msg %s",
             object, objectType, location, messageCode, pLayerPrefix, pMessage);

    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        logw("object %"PRIu64" type 0x%x location %zu code %u layer \"%s\" msg %s",
             object, objectType, location, messageCode, pLayerPrefix, pMessage);

    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        logw("object %"PRIu64" type 0x%x location %zu code %u layer \"%s\" msg %s",
             object, objectType, location, messageCode, pLayerPrefix, pMessage);

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        loge("object %"PRIu64" type 0x%x location %zu code %u layer \"%s\" msg %s",
             object, objectType, location, messageCode, pLayerPrefix, pMessage);

    /* We don't want to spam the logs in case both debug and info bit set. */
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT &&
       !flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        logd("object %"PRIu64" type 0x%x location %zu code %u layer \"%s\" msg %s",
             object, objectType, location, messageCode, pLayerPrefix, pMessage);

    return false;
}

void
t_setup_vulkan(void)
{
    GET_CURRENT_TEST(t);
    VkResult res;
    const char **ext_names;

    res = vkEnumerateInstanceExtensionProperties(NULL,
        &t->vk.instance_extension_count, NULL);
    t_assert(res == VK_SUCCESS);

    t->vk.instance_extension_props =
        malloc(t->vk.instance_extension_count * sizeof(*t->vk.instance_extension_props));
    t_assert(t->vk.instance_extension_props);
    t_cleanup_push_free(t->vk.instance_extension_props);

    res = vkEnumerateInstanceExtensionProperties(NULL,
        &t->vk.instance_extension_count, t->vk.instance_extension_props);
    t_assert(res == VK_SUCCESS);

    ext_names = malloc(t->vk.instance_extension_count * sizeof(*ext_names));
    t_assert(ext_names);

    void *debug_report = NULL;
    VkDebugReportCallbackCreateInfoEXT cb_info = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        NULL,
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        debug_cb,
        t
    };

    for (uint32_t i = 0; i < t->vk.instance_extension_count; i++) {
        ext_names[i] = t->vk.instance_extension_props[i].extensionName;

        if (strcmp(ext_names[i], "VK_EXT_debug_report") == 0)
           debug_report = &cb_info;
    }

    res = vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = debug_report,
            .pApplicationInfo = &(VkApplicationInfo) {
                .pApplicationName = "crucible",
                .apiVersion = VK_MAKE_VERSION(1, 0, 0),
            },
            .enabledExtensionCount = t->vk.instance_extension_count,
            .ppEnabledExtensionNames = ext_names,
        }, &test_alloc_cb, &t->vk.instance);
    free(ext_names);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_instance(t->vk.instance, &test_alloc_cb);

    if (debug_report) {
#define RESOLVE(func)\
    (PFN_ ##func) vkGetInstanceProcAddr(t->vk.instance, #func);
        t->vk.vkCreateDebugReportCallbackEXT = RESOLVE(vkCreateDebugReportCallbackEXT);
        t->vk.vkDestroyDebugReportCallbackEXT = RESOLVE(vkDestroyDebugReportCallbackEXT);
#undef RESOLVE

        assert(t->vk.vkCreateDebugReportCallbackEXT);
        assert(t->vk.vkDestroyDebugReportCallbackEXT);

        VkDebugReportCallbackCreateInfoEXT info;
        info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        info.pNext = NULL;
        info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                     VK_DEBUG_REPORT_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_ERROR_BIT_EXT |
                     VK_DEBUG_REPORT_DEBUG_BIT_EXT;
        info.pfnCallback = debug_cb;
        info.pUserData = t;

        res = t->vk.vkCreateDebugReportCallbackEXT(t_instance, &info, NULL,
                                                   &t->vk.debug_callback);
        t_assert(res == VK_SUCCESS);
        t_assert(t->vk.debug_callback != 0);

        t_cleanup_push_vk_debug_cb(t->vk.vkDestroyDebugReportCallbackEXT,
                                   t->vk.instance, t->vk.debug_callback);
    }

    t_setup_phys_dev();

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(t->vk.physical_dev,
                                             &queue_family_count, NULL);
    for (uint32_t i = 0; i < queue_family_count; i++) {
       VkQueueFamilyProperties *props = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
       vkGetPhysicalDeviceQueueFamilyProperties(t->vk.physical_dev,
                                                &queue_family_count, props);
       free(props);
    }
    qoGetPhysicalDeviceMemoryProperties(t->vk.physical_dev,
                                        &t->vk.physical_dev_mem_props);

    res = vkEnumerateDeviceExtensionProperties(t->vk.physical_dev, NULL,
        &t->vk.device_extension_count, NULL);
    t_assert(res == VK_SUCCESS);

    t->vk.device_extension_props =
        malloc(t->vk.device_extension_count * sizeof(*t->vk.device_extension_props));
    t_assert(t->vk.device_extension_props);
    t_cleanup_push_free(t->vk.device_extension_props);

    res = vkEnumerateDeviceExtensionProperties(t->vk.physical_dev, NULL,
        &t->vk.device_extension_count, t->vk.device_extension_props);
    t_assert(res == VK_SUCCESS);

    ext_names = malloc(t->vk.device_extension_count * sizeof(*ext_names));
    t_assert(ext_names);

    for (uint32_t i = 0; i < t->vk.device_extension_count; i++)
        ext_names[i] = t->vk.device_extension_props[i].extensionName;

    res = vkCreateDevice(t->vk.physical_dev,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &(VkDeviceQueueCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = (float[]) {1.0f},
            },
            .enabledExtensionCount = t->vk.device_extension_count,
            .ppEnabledExtensionNames = ext_names,
        }, NULL, &t->vk.device);
    free(ext_names);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_device(t->vk.device, NULL);

    t_setup_descriptor_pool();

    t_setup_framebuffer();

    vkGetDeviceQueue(t->vk.device, 0, 0, &t->vk.queue);

    t->vk.pipeline_cache = qoCreatePipelineCache(t->vk.device);

    res = vkCreateCommandPool(t->vk.device,
        &(VkCommandPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
            .flags = 0,
        }, NULL, &t->vk.cmd_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(t->vk.device, t->vk.cmd_pool);

    t->vk.cmd_buffer = qoAllocateCommandBuffer(t->vk.device, t->vk.cmd_pool);

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
