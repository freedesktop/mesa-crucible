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

#include <stdlib.h>
#include "tapi/t.h"

#include "basic-spirv.h"

typedef struct test_params {
    float depth_clear_value;
    VkCompareOp depth_compare_op;
} test_params_t;

static void
test(void)
{
    const test_params_t *params = t_user_data;

    VkPipeline pipeline;

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 16,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            },
            {
                .binding = 1,
                .stride = 16,
                .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
            }
        },
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0
            }
        }
    };

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device);

    pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_create_info,
            .pRasterizationState = &(VkPipelineRasterizationStateCreateInfo) {
                QO_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_DEFAULTS,
                .rasterizerDiscardEnable = false,
            },
            .pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo) {
                QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
                .depthTestEnable = true,
                .depthWriteEnable = true,
                .depthCompareOp = params->depth_compare_op,
            },
            .flags = 0,
            .layout = pipeline_layout,
            .renderPass = t_render_pass,
            .subpass = 0,
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
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    qoBindBufferMemory(t_device, vertex_buffer, vertex_mem, /*offset*/ 0);

    memcpy(qoMapMemory(t_device, vertex_mem, /*offset*/ 0,
                       sizeof(vertex_data), /*flags*/ 0),
           vertex_data, sizeof(vertex_data));

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 2,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 0.2, 0.2, 0.2, 1.0 } } },
                { .depthStencil = { .depth = params->depth_clear_value } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, 6 * 4 * sizeof(float) });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDraw(t_cmd_buffer, 3, 1, 0, 0);
    vkCmdDraw(t_cmd_buffer, 3, 1, 3, 1);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.depthstencil.basic-depth.clear-0.0.op-less",
    .start = test,
    .depthstencil_format = VK_FORMAT_X8_D24_UNORM_PACK32,
    .user_data = &(test_params_t) {
        .depth_compare_op = VK_COMPARE_OP_LESS,
        .depth_clear_value = 0.0,
    },
};

test_define {
    .name = "func.depthstencil.basic-depth.clear-0.0.op-greater",
    .start = test,
    .depthstencil_format = VK_FORMAT_X8_D24_UNORM_PACK32,
    .user_data = &(test_params_t) {
        .depth_compare_op = VK_COMPARE_OP_GREATER,
        .depth_clear_value = 0.0,
    },
};

test_define {
    .name = "func.depthstencil.basic-depth.clear-0.5.op-greater-equal",
    .start = test,
    .depthstencil_format = VK_FORMAT_X8_D24_UNORM_PACK32,
    .user_data = &(test_params_t) {
        .depth_clear_value = 0.5,
        .depth_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
    },
};

test_define {
    .name = "func.depthstencil.basic-depth.clear-1.0.op-greater",
    .start = test,
    .depthstencil_format = VK_FORMAT_X8_D24_UNORM_PACK32,
    .user_data = &(test_params_t) {
        .depth_clear_value = 1.0,
        .depth_compare_op = VK_COMPARE_OP_GREATER,
    },
};
