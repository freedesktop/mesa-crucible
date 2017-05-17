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

#include "fragcoord-spirv.h"

static void
test_fragcoord(void)
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
                .preserveAttachmentCount = 1,
                .pPreserveAttachments = (uint32_t[]) { 0 },
            }
        });

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 0) out vec2 v_attr;
        void main()
        {
            gl_Position.xy = a_position.xy * 2 - vec2(1.0);
            gl_Position.zw = a_position.zw;
            v_attr.x = -a_position.x;
            v_attr.y = 1.0 - a_position.y;
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(origin_upper_left) in vec4 gl_FragCoord;
        layout(location = 0) in vec2 v_attr;
        layout(location = 0) out vec4 f_color;
        void main()
        {
            f_color.xy = v_attr + gl_FragCoord.xy / 32.0;
            f_color.zw = gl_FragCoord.zw;
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 8,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            },
        },
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0
            },
        }
    };

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo) {
                QO_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_DEFAULTS,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            },
            .pVertexInputState = &vi_info,
            .renderPass = pass,
            .subpass = 0,
        }});

    const float vertices[] = {
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };

    VkBuffer vb = qoCreateBuffer(t_device, .size =  sizeof(vertices),
                                 .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vb_mem = qoAllocBufferMemory(t_device, vb,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, vb, vb_mem, 0);

    void *vb_map = qoMapMemory(t_device, vb_mem, 0, sizeof(vertices), 0);

    memcpy(vb_map, vertices, sizeof(vertices));

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = {1.0, 0.0, 0.0, 1.0} } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1, (VkBuffer[]) { vb },
                           (VkDeviceSize[]) { 0 });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.shader.fragcoord",
    .start = test_fragcoord,
    .image_filename = "32x32-green.ref.png",
};
