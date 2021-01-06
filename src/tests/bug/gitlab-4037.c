// Copyright 2021 Intel Corporation
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

#include <inttypes.h>
#include <stdio.h>
#include "tapi/t.h"

/* This is a test for https://gitlab.freedesktop.org/mesa/mesa/-/issues/4037
 *
 * The issue is a crash in Anv when the render pass ends and we try to
 * resolve a HIZ auxiliary buffer. For resolve to happen we need to
 * trigger a blorp operation which is failing because we don't
 * internally have considered the TRANSFER layout for the render pass
 * resolve operation.
 */

static void
test_gitlab_4037(void)
{
    static const uint32_t width = 64;
    static const uint32_t height = 64;
    const VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT;

    t_require_ext("VK_KHR_depth_stencil_resolve");


    VkImage images[2];

    images[0] = qoCreateImage(t_device,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .mipLevels = 1,
        .arrayLayers = 1,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
        .samples = VK_SAMPLE_COUNT_2_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    images[1] = qoCreateImage(t_device,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .mipLevels = 1,
        .arrayLayers = 1,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_TRANSFER_DST_BIT);


    for (uint32_t i = 0; i < ARRAY_LENGTH(images); i++) {
        VkDeviceMemory mem = qoAllocImageMemory(t_device, images[i],
                                 .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        qoBindImageMemory(t_device, images[i], mem, 0);
    }

    VkImageView views[2];

    for (uint32_t i = 0; i < ARRAY_LENGTH(images); i++) {
        views[i] = qoCreateImageView(
            t_device,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            });
    }

    VkAttachmentReference2 att_resolve_reference = {
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescriptionDepthStencilResolveKHR ds_resolve_info = {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR,
        .depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR,
        .stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR,
        .pDepthStencilResolveAttachment = &att_resolve_reference,
    };

    VkAttachmentReference2 att_reference = {
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription2 att_descs[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .format = format,
            .samples = VK_SAMPLE_COUNT_2_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
        },
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .format = format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        }
    };

    VkClearValue clear_values[2] = {
        {
            .depthStencil = {
                .depth = 0.5,
                .stencil = 0x80,
            },
        },
        {
            .depthStencil = {
                .depth = 0.5,
                .stencil = 0x80,
            },
        }
    };

#define GET_FUNCTION_PTR(name, device)                                  \
    PFN_vk##name name = (PFN_vk##name)vkGetDeviceProcAddr(device, "vk"#name)
    GET_FUNCTION_PTR(CreateRenderPass2KHR, t_device);
#undef GET_FUNCTION_PTR

    VkRenderPass pass;

    CreateRenderPass2KHR(
        t_device,
        &(VkRenderPassCreateInfo2KHR){
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR,
            .attachmentCount = 2,
            .pAttachments = att_descs,
            .subpassCount = 1,
            .pSubpasses = (VkSubpassDescription2[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
                    .pNext = &ds_resolve_info,
                    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .pDepthStencilAttachment = &att_reference,
                },
            }
        },
        NULL,
        &pass);

    VkFramebuffer fb = qoCreateFramebuffer(t_device,
        .renderPass = pass,
        .attachmentCount = 2,
        .pAttachments = views,
        .width = width,
        .height = height,
        .layers = 1);

    VkCommandBuffer cmd = qoAllocateCommandBuffer(t_device, t_cmd_pool);

    qoBeginCommandBuffer(cmd);
    vkCmdBeginRenderPass(cmd,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = pass,
            .framebuffer = fb,
            .renderArea = {{0, 0}, {width, height}},
            .clearValueCount = 2,
            .pClearValues = clear_values,
        },
        VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmd);


    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    t_end(TEST_RESULT_PASS);
}

test_define {
    .name = "bug.gitlab.4037",
    .start = test_gitlab_4037,
    .no_image = true,
};
