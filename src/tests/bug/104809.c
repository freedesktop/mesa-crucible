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
#include "util/misc.h"

/* This is a test for https://bugs.freedesktop.org/show_bug.cgi?id=104809
 *
 * At the root of the hang was the fact that we had a hole in our vertex
 * element state array.  Because the Intel Vulkan driver compacts down the
 * inputs to only those used by the shader, you need to have a location
 * which is used by the shader but not provided by the vertex input state
 * in order to trigger the hang.  This test provides a simple pipeline
 * which does just that.
 */

#include "src/tests/bug/104809-spirv.h"

static void
test(void)
{
    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 1) in vec4 a_junk;
        layout(location = 2) in vec4 a_zero;
        void main()
        {
            gl_Position = a_position + a_junk * a_zero;
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(set = 0, binding = 1) uniform sampler2D tex;

        void main()
        {
            f_color = vec4(0, 1, 0, 1);
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 16,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
            {
                .binding = 1,
                .stride = 0,
                .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
            },
        },
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0,
            },
            {
                .location = 2,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0,
            },
        },
    };

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_create_info,
            .flags = 0,
            .layout = qoCreatePipelineLayout(t_device, 0, NULL),
            .renderPass = t_render_pass,
            .subpass = 0,
        }});

    static const float vertex_data[] = {
        // Triangle coordinates
        -1.0, -1.0, 0.0, 1.0,
         1.0, -1.0, 0.0, 1.0,
        -1.0,  1.0, 0.0, 1.0,
         1.0,  1.0, 0.0, 1.0,

         // Zeros
         0.0,  0.0, 0.0, 0.0,
    };

    VkBuffer vertex_buffer = qoCreateBuffer(t_device,
        .size = sizeof(vertex_data),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vertex_mem = qoAllocBufferMemory(t_device, vertex_buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    memcpy(qoMapMemory(t_device, vertex_mem, /*offset*/ 0,
                       sizeof(vertex_data), /*flags*/ 0),
           vertex_data,
           sizeof(vertex_data));

    qoBindBufferMemory(t_device, vertex_buffer, vertex_mem,
                       /*offset*/ 0);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, 4 * 4 * sizeof(float) });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(t_cmd_buffer, /*vertexCount*/ 4, /*instanceCount*/ 1,
              /*firstVertex*/ 0, /*firstInstance*/ 0);
    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "bug.104809",
    .start = test,
    .image_filename = "32x32-green.ref.png",
};
