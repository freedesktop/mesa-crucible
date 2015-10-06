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
push_vs_offset(float x, float y)
{
    float vec2[2] = { x, y };
    vkCmdPushConstants(t_cmd_buffer, QO_NULL_PIPELINE_LAYOUT,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vec2), vec2);
}

static void
push_fs_color(unsigned first_component, unsigned num_components, float *c)
{
    vkCmdPushConstants(t_cmd_buffer, QO_NULL_PIPELINE_LAYOUT,
                       VK_SHADER_STAGE_FRAGMENT_BIT,
                       first_component * sizeof(float),
                       num_components * sizeof(float), c);
}

static void
test_push_constants(void)
{
    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .bindingCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 8,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX
            },
        },
        .attributeCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offsetInBytes = 0
            },
        }
    };

    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        uniform vec2 offset;
        void main()
        {
            gl_Position = a_position + vec4(offset.xy, 0.0, 0.0);
        }
    );

    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        uniform vec4 u_color;
        layout(location = 0) out vec4 f_color;
        void main()
        {
            f_color = u_color;
        }
    );

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_info,
        }});

    static const float vertices[] = {
        0.0, 0.0,
        1.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
    };

    VkBuffer vbo = qoCreateBuffer(t_device, .size = sizeof(vertices),
                                 .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkDeviceMemory mem =
        qoAllocBufferMemory(t_device, vbo,
                            .memoryTypeIndex = t_mem_type_index_for_mmap);
    qoBindBufferMemory(t_device, vbo, mem, /*offset*/ 0);
    float *const vbo_map = qoMapMemory(t_device, mem, /*offset*/ 0,
                                       sizeof(vertices), /*flags*/ 0);
    memcpy(vbo_map, vertices, sizeof(vertices));

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
                .colorCount = 1,
                .pColorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    },
                },
            }
        });

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .f32 = { 0.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_RENDER_PASS_CONTENTS_INLINE);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1,
                           (VkBuffer[]) { vbo },
                           (VkDeviceSize[]) { 0 });

    /* Start off upper-left and red */
    push_vs_offset(-1.0, -1.0);
    push_fs_color(0, 4, (float[]) { 1.0, 0.0, 0.0, 1.0 });
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    /* Upper-right */
    push_vs_offset(0.0, -1.0);
    /* Update just the green component to get yellow */
    push_fs_color(1, 1, (float[]) { 1.0 });
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    /* Bottom-left */
    push_vs_offset(-1.0, 0.0);
    /* Update just the red component to get green */
    push_fs_color(0, 1, (float[]) { 0.0 });
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    /* Bottom-right */
    push_vs_offset(0.0, 0.0);
    /* Update the green and blue components to get blue */
    push_fs_color(1, 2, (float[]) { 0.0, 1.0 });
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

test_define {
    .name = "func.push-constants.basic",
    .start = test_push_constants,
};
