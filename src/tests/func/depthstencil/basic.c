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

#include <crucible/cru.h>

#include "basic-spirv.h"

static inline uint32_t
align_u32(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static void
test(void)
{
    VkPipeline pipeline;

    VkImage ds = qoCreateImage(t_device,
        .format = VK_FORMAT_D24_UNORM,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_BIT,
        .extent = {
            .width = t_width,
            .height = t_height,
            .depth = 1,
        });

    VkDeviceMemory ds_mem = qoAllocImageMemory(t_device, ds,
        .memoryTypeIndex = t_mem_type_index_for_device_access);

    qoBindImageMemory(t_device, ds, ds_mem, /*offset*/ 0);

    VkAttachmentView ds_view = qoCreateAttachmentView(t_device,
        .image = ds,
        .format = VK_FORMAT_D24_UNORM);

    VkFramebuffer framebuffer = qoCreateFramebuffer(t_device,
        .attachmentCount = 2,
        .pAttachments = (VkAttachmentBindInfo[]) {
            {
                .view = t_image_attachment_view,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            },
            {
                .view = ds_view,
                .layout = VK_IMAGE_LAYOUT_GENERAL,
            },
        },
        .width = t_width,
        .height = t_height);

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .attachmentCount = 2,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            },
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_D24_UNORM,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorCount = 1,
                .colorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    },
                },
                .depthStencilAttachment = {
                    .attachment = 1,
                    .layout = VK_IMAGE_LAYOUT_GENERAL,
                }
            }
        });

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .bindingCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 16,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX
            },
            {
                .binding = 1,
                .strideInBytes = 16,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_INSTANCE
            }
        },
        .attributeCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0
            }
        }
    };

    pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_create_info,
            .pRasterState = &(VkPipelineRasterStateCreateInfo) {
                QO_PIPELINE_RASTER_STATE_CREATE_INFO_DEFAULTS,
                .depthClipEnable = true,
                .rasterizerDiscardEnable = false,
            },
            .pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo) {
                QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
                .depthTestEnable = true,
                .depthWriteEnable = true,
                .depthCompareOp = VK_COMPARE_OP_GREATER
            },
            .flags = 0,
            .layout = VK_NULL_HANDLE
        }});

    static const float vertex_data[] = {
        /* First triangle coordinates */
        -0.7, -0.5, 0.5, 1.0,
        0.3, -0.5, 0.5, 1.0,
        -0.2,  0.5, 0.5, 1.0,

        /* Second triangle coordinates */
        -0.3, -0.3, 0.4, 1.0,
        0.7, -0.3, 0.4, 1.0,
        0.2,  0.7, 0.8, 1.0,

        /* First triangle color */
        1.0,  1.0, 0.2, 1.0,

        /* Second triangle color */
        0.2,  0.2, 1.0, 1.0,
    };

    VkBuffer vertex_buffer = qoCreateBuffer(t_device,
        .size = sizeof(vertex_data),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vertex_mem = qoAllocBufferMemory(t_device, vertex_buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    qoBindBufferMemory(t_device, vertex_buffer, vertex_mem, /*offset*/ 0);

    memcpy(qoMapMemory(t_device, vertex_mem, /*offset*/ 0,
                       sizeof(vertex_data), /*flags*/ 0),
           vertex_data, sizeof(vertex_data));

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .attachmentCount = 2,
            .pAttachmentClearValues = (VkClearValue[]) {
                { .color = { .f32 = { 0.2, 0.2, 0.2, 1.0 } } },
                { .ds = { .depth = 0.5 } },
            }
        }, VK_RENDER_PASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, 6 * 4 * sizeof(float) });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDraw(t_cmd_buffer, 0, 3, 0, 1);
    vkCmdDraw(t_cmd_buffer, 3, 3, 1, 1);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, 0);
}

cru_define_test {
    .name = "func.depthstencil.basic",
    .start = test
};
