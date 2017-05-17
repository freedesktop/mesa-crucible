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

#include "tapi/t.h"

#include "basic-spirv.h"

static void
test_basic_gs(void)
{
    VkRenderPass pass = qoCreateRenderPass(t_device,
        .attachmentCount = 1,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
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

    VkShaderModule gs = qoCreateShaderModuleGLSL(t_device, GEOMETRY,
        layout(points) in;
        layout(triangle_strip, max_vertices = 4) out;
        layout(location = 0) in vec4 v_color[];
        layout(location = 0) out vec4 g_color;

        void main()
        {
            g_color = v_color[0];
            gl_Position = gl_in[0].gl_Position + vec4(0.2, 0.2, 0.0, 0.0);
            EmitVertex();
            gl_Position = gl_in[0].gl_Position + vec4(-0.2, 0.2, 0.0, 0.0);
            EmitVertex();
            gl_Position = gl_in[0].gl_Position + vec4(0.2, -0.2, 0.0, 0.0);
            EmitVertex();
            gl_Position = gl_in[0].gl_Position + vec4(-0.2, -0.2, 0.0, 0.0);
            EmitVertex();
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 8,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            },
            {
                .binding = 1,
                .stride = 16,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            }
        },
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
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

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .geometryShader = gs,
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_info,
            .renderPass = pass,
            .subpass = 0,
        }});

#define HEX_COLOR(v, f)                         \
    {                                           \
        ((v) >> 16) / 255.0,                    \
        ((v & 0xff00) >> 8) / 255.0,            \
        ((v & 0xff)) / 255.0,                   \
        f                                       \
    }

    static const struct {
        float vertices[4][2];
        float colors[4][4];
    } vertex_data = {
        .vertices = {
            { 0.5, 0.5 },
            { -0.5, 0.5 },
            { 0.5, -0.5 },
            { -0.5, -0.5 },
        },
        .colors = {
            HEX_COLOR(0xa6d49f, 1.0f),
            HEX_COLOR(0xe06d06, 1.0f),
            HEX_COLOR(0xc59849, 1.0f),
            HEX_COLOR(0x9cb380, 1.0f),
        },
    };

    void *map;

    VkBuffer buffer =
        qoCreateBuffer(t_device,
                       .size = sizeof(vertex_data),
                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    map = qoMapMemory(t_device, mem, 0, sizeof(vertex_data), 0);
    qoBindBufferMemory(t_device, buffer, mem, 0);
    memcpy(map, &vertex_data, sizeof(vertex_data));

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = HEX_COLOR(0x522a27, 1.0) } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { buffer, buffer },
                           (VkDeviceSize[]) { 0, sizeof(vertex_data.vertices) });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.gs.basic",
    .start = test_basic_gs,
};
