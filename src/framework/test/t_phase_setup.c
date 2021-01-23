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

/* Maximum supported physical devs. */
#define MAX_PHYSICAL_DEVS 4

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

    VkPhysicalDevice physical_devs[MAX_PHYSICAL_DEVS];

    uint32_t count = 0;
    qoEnumeratePhysicalDevices(t->vk.instance, &count, NULL);
    t_assertf(count > 0, "failed to enumerate any physical devices");
    t_assertf(count <= MAX_PHYSICAL_DEVS, "reached the maximum supported physical devices");
    t_assertf(t->opt.device_id <= count, "requested device id not found");

    qoEnumeratePhysicalDevices(t->vk.instance, &count, physical_devs);
    t->vk.physical_dev = physical_devs[t->opt.device_id - 1];

    vkGetPhysicalDeviceFeatures(t->vk.physical_dev, &t->vk.physical_dev_features);
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
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
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
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    },
                },
            }
        });

    t->vk.render_pass = color_pass;

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
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                },
                {
                    QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                    .format = t->def->depthstencil_format,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
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
                            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        },
                    },
                    .pDepthStencilAttachment = &(VkAttachmentReference) {
                        .attachment = 1,
                        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    },
                },
            });

        t->vk.render_pass = color_depth_pass;
    }

    t->vk.framebuffer = qoCreateFramebuffer(t->vk.device,
        .renderPass = t->vk.render_pass,
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
        .maxSets = 8,
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

    bool has_debug_report = false;
    VkDebugReportCallbackCreateInfoEXT debug_report_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                 VK_DEBUG_REPORT_ERROR_BIT_EXT,
        .pfnCallback = debug_cb,
        .pUserData = t,
    };

    if (t->opt.verbose) {
        debug_report_info.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                   VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    }

    for (uint32_t i = 0; i < t->vk.instance_extension_count; i++) {
        ext_names[i] = t->vk.instance_extension_props[i].extensionName;

        if (strcmp(ext_names[i], "VK_EXT_debug_report") == 0)
            has_debug_report = true;
    }

    uint32_t api_version = t->def->api_version ?
        t->def->api_version : VK_MAKE_VERSION(1, 0, 0);

    res = vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            /* This debug report applies only to the vkCreateInstance
             * and vkDestroyInstance calls.
             */
            .pNext = has_debug_report ? &debug_report_info : NULL,
            .pApplicationInfo = &(VkApplicationInfo) {
                .pApplicationName = "crucible",
                .apiVersion = api_version
            },
            .enabledExtensionCount = t->vk.instance_extension_count,
            .ppEnabledExtensionNames = ext_names,
        }, &test_alloc_cb, &t->vk.instance);
    free(ext_names);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_instance(t->vk.instance, &test_alloc_cb);

    if (has_debug_report) {
#define RESOLVE(func)\
    (PFN_ ##func) vkGetInstanceProcAddr(t->vk.instance, #func);
        t->vk.vkCreateDebugReportCallbackEXT = RESOLVE(vkCreateDebugReportCallbackEXT);
        t->vk.vkDestroyDebugReportCallbackEXT = RESOLVE(vkDestroyDebugReportCallbackEXT);
#undef RESOLVE

        assert(t->vk.vkCreateDebugReportCallbackEXT);
        assert(t->vk.vkDestroyDebugReportCallbackEXT);

        res = t->vk.vkCreateDebugReportCallbackEXT(t_instance, &debug_report_info,
                                                   NULL, &t->vk.debug_callback);
        t_assert(res == VK_SUCCESS);
        t_assert(t->vk.debug_callback != 0);

        t_cleanup_push_vk_debug_cb(t->vk.vkDestroyDebugReportCallbackEXT,
                                   t->vk.instance, t->vk.debug_callback);
    }

    t_setup_phys_dev();

    vkGetPhysicalDeviceQueueFamilyProperties(t->vk.physical_dev,
                                             &t->vk.queue_family_count, NULL);

    t->vk.queue_family_props = malloc(t->vk.queue_family_count *
                                      sizeof(VkQueueFamilyProperties));
    t_assert(t->vk.queue_family_props);
    t_cleanup_push_free(t->vk.queue_family_props);
    vkGetPhysicalDeviceQueueFamilyProperties(t->vk.physical_dev,
                                             &t->vk.queue_family_count,
                                             t->vk.queue_family_props);

    bool queue_found = false;
    uint32_t queue_family = 0;
    uint32_t queue_in_family = 0;
    t->vk.queue_count = 0;
    for (uint32_t i = 0; i < t->vk.queue_family_count; i++) {
        uint32_t next_start =
            t->vk.queue_count + t->vk.queue_family_props[i].queueCount;
        if (t_queue_num >= t->vk.queue_count && t_queue_num < next_start) {
            queue_family = i;
            queue_in_family = t_queue_num - t->vk.queue_count;
            queue_found = true;
        }
        t->vk.queue_count = next_start;
    }

    if (!queue_found)
        t_end(TEST_RESULT_SKIP);

    /* Skip if not the first queue in the queue-family */
    if (queue_in_family != 0)
        t_end(TEST_RESULT_SKIP);

    uint32_t qf =
        t->vk.queue_family_props[queue_family].queueFlags;
    if (qf & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
        qf &= ~VK_QUEUE_TRANSFER_BIT;
    qf &= VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
        VK_QUEUE_TRANSFER_BIT;
    switch (t->def->queue_setup) {
    case QUEUE_SETUP_GFX_AND_COMPUTE:
        if (qf != (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            t_end(TEST_RESULT_SKIP);
        break;
    case QUEUE_SETUP_GRAPHICS:
        if ((qf & VK_QUEUE_GRAPHICS_BIT) == 0)
            t_end(TEST_RESULT_SKIP);
        break;
    case QUEUE_SETUP_COMPUTE:
        if ((qf & VK_QUEUE_COMPUTE_BIT) == 0)
            t_end(TEST_RESULT_SKIP);
        break;
    case QUEUE_SETUP_TRANSFER:
        if (qf == 0 /* gfx and compute imply transfer */)
            t_end(TEST_RESULT_SKIP);
        break;
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

    VkDeviceQueueCreateInfo *qci = calloc(t->vk.queue_count, sizeof(*qci));
    t_assert(qci);

    const float priority = 1.0f;
    for (uint32_t i = 0; i < t->vk.queue_family_count; i++) {
        qci[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci[i].queueFamilyIndex = i;
        qci[i].queueCount = t->vk.queue_family_props[i].queueCount;
        qci[i].pQueuePriorities = &priority;
    }

    VkPhysicalDeviceFeatures pdf;
    vkGetPhysicalDeviceFeatures(t->vk.physical_dev, &pdf);
    pdf.robustBufferAccess = t->def->robust_buffer_access;

    res = vkCreateDevice(t->vk.physical_dev,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = t->vk.queue_family_count,
            .pQueueCreateInfos = qci,
            .enabledExtensionCount = t->vk.device_extension_count,
            .ppEnabledExtensionNames = ext_names,
            .pEnabledFeatures = &pdf,
        }, NULL, &t->vk.device);
    free(qci);
    free(ext_names);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_device(t->vk.device, NULL);

    t_setup_descriptor_pool();

    t_setup_framebuffer();

    t->vk.queue = calloc(t->vk.queue_count, sizeof(*t->vk.queue));
    t_assert(t->vk.queue);
    t_cleanup_push_free(t->vk.queue);

    for (uint32_t qfam = 0, q = 0; qfam < t->vk.queue_family_count; qfam++) {
        uint32_t queues_in_fam = t->vk.queue_family_props[qfam].queueCount;
        for (uint32_t j = 0; j < queues_in_fam; j++) {
            vkGetDeviceQueue(t->vk.device, qfam, j, &t->vk.queue[q + j]);
        }
        q += queues_in_fam;
    }

    t->vk.pipeline_cache = qoCreatePipelineCache(t->vk.device);

    t->vk.cmd_pool =
        calloc(t->vk.queue_count, sizeof(*t->vk.cmd_pool));
    t_assert(t->vk.cmd_pool);
    t_cleanup_push_free(t->vk.cmd_pool);

    for (uint32_t qfam = 0, q = 0; qfam < t->vk.queue_family_count; qfam++) {
        uint32_t queues_in_fam = t->vk.queue_family_props[qfam].queueCount;
        for (uint32_t j = 0; j < queues_in_fam; j++) {
            res = vkCreateCommandPool(t->vk.device,
                &(VkCommandPoolCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .queueFamilyIndex = qfam,
                    .flags = 0,
                }, NULL, &t->vk.cmd_pool[q]);
            t_assert(res == VK_SUCCESS);
            t_cleanup_push_vk_cmd_pool(t->vk.device, t->vk.cmd_pool[q]);
            q++;
        }
    }

    t->vk.graphics_and_compute_queue = -1;
    t->vk.graphics_queue = -1;
    t->vk.compute_queue = -1;
    t->vk.transfer_queue = -1;

    /* Search through the queues looking for a "best match" */
    for (uint32_t qfam = 0, q = 0; qfam < t->vk.queue_family_count; qfam++) {
        uint32_t qf = t->vk.queue_family_props[qfam].queueFlags;
        if (qf & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            qf &= ~VK_QUEUE_TRANSFER_BIT;
        qf &= VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
              VK_QUEUE_TRANSFER_BIT;
        if (qf == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            t->vk.graphics_and_compute_queue = q;
        if (qf == VK_QUEUE_GRAPHICS_BIT)
            t->vk.graphics_queue = q;
        if (qf == VK_QUEUE_COMPUTE_BIT)
            t->vk.compute_queue = q;
        if (qf == VK_QUEUE_TRANSFER_BIT)
            t->vk.transfer_queue = q;
        q += t->vk.queue_family_props[qfam].queueCount;
    }

    /* Search through the queues looking for a "acceptable match" */
    for (uint32_t qfam = 0, q = 0; qfam < t->vk.queue_family_count; qfam++) {
        uint32_t qf = t->vk.queue_family_props[qfam].queueFlags;
        if (qf & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            qf |= VK_QUEUE_TRANSFER_BIT;
        qf &= VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
              VK_QUEUE_TRANSFER_BIT;
        if (t->vk.graphics_queue < 0 && (qf & VK_QUEUE_GRAPHICS_BIT))
            t->vk.graphics_queue = q;
        if (t->vk.compute_queue < 0 && (qf & VK_QUEUE_COMPUTE_BIT))
            t->vk.compute_queue = q;
        if (t->vk.transfer_queue < 0 && (qf & VK_QUEUE_TRANSFER_BIT))
            t->vk.transfer_queue = q;
        q += t->vk.queue_family_props[qfam].queueCount;
    }

    t->vk.cmd_buffer = qoAllocateCommandBuffer(t->vk.device, t_cmd_pool);

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
