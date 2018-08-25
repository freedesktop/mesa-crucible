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

static VkBuffer
make_vbo(void)
{
    size_t vbo_size = (4 + 32 * 32 * 2) * sizeof(float);

    VkBuffer vbo =
        qoCreateBuffer(t_device, .size = vbo_size,
                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vbo_mem = qoAllocBufferMemory(t_device, vbo,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    float *vbo_map = qoMapMemory(t_device, vbo_mem, /*offset*/ 0,
                                 vbo_size, /*flags*/ 0);

    // Copy green into the first 4 floats.
    memcpy(vbo_map, (float[]) { 0.0, 1.0, 0.0, 1.0 }, 4 * sizeof(float));
    vbo_map += 4;

    qoBindBufferMemory(t_device, vbo, vbo_mem, /*offset*/ 0);

    // Fill the rest of the VBO with 2D coordinates. One per pixel in a
    // 32x32 image
    for (int x = 0; x < 32; x++) {
        for (int y = 0; y < 32; y++) {
            vbo_map[2 * (y * 32 + x) + 0] = (x + 0.5) / 16.0 - 1.0;
            vbo_map[2 * (y * 32 + x) + 1] = (y + 0.5) / 16.0 - 1.0;
        }
    }

    return vbo;
}

static VkPipelineLayout
create_pipeline_layout(void)
{
    return qoCreatePipelineLayout(t_device);
}

static VkPipeline
create_pipeline(VkRenderPass pass, VkPipelineLayout layout)
{
    return qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .renderPass = pass,
            .layout = layout,
            .subpass = 0,
        }});
}

static VkRenderPass
create_and_begin_render_pass(void)
{
    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    return t_render_pass;
}

static VkCommandBuffer
make_secondary_cmd_buffer(VkRenderPass pass, VkPipeline pipeline,
                          VkCommandBufferUsageFlags usage_flags)
{
    VkCommandBuffer secondary =
        qoAllocateCommandBuffer(t_device, t_cmd_pool,
                                .level = VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    qoBeginCommandBuffer(secondary,
        .flags = usage_flags | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        .pInheritanceInfo = (&(VkCommandBufferInheritanceInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            .renderPass = pass,
            .framebuffer = t_framebuffer,
        }));

    vkCmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    return secondary;
}

static void
test_small_secondaries(void)
{
    VkBuffer vbo = make_vbo();
    VkRenderPass pass = create_and_begin_render_pass();
    VkPipelineLayout pipeline_layout = create_pipeline_layout();
    VkPipeline pipeline = create_pipeline(pass, pipeline_layout);

    VkCommandBuffer secondaries[1024];

    for (int i = 0; i < 1024; i++) {
        secondaries[i] =
            make_secondary_cmd_buffer(pass, pipeline, 0);

        vkCmdBindVertexBuffers(secondaries[i], 0, 2,
                               (VkBuffer[]) { vbo, vbo },
                               (VkDeviceSize[]) { (4 + i * 2) * sizeof(float), 0 });
        vkCmdDraw(secondaries[i], 1, 1, 0, 0);
        qoEndCommandBuffer(secondaries[i]);
    }

    vkCmdExecuteCommands(t_cmd_buffer, 1024, secondaries);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.cmd-buffer.small-secondaries",
    .start = test_small_secondaries,
    .image_filename = "32x32-green.ref.png",
};

static void
do_test_large_secondary(VkCommandBufferUsageFlags usage_flags)
{
    VkBuffer vbo = make_vbo();
    VkRenderPass pass = create_and_begin_render_pass();
    VkPipelineLayout pipeline_layout = create_pipeline_layout();
    VkPipeline pipeline = create_pipeline(pass, pipeline_layout);

    VkCommandBuffer secondary =
        make_secondary_cmd_buffer(pass, pipeline, usage_flags);

    for (int i = 0; i < 1024; i++) {
        vkCmdBindVertexBuffers(secondary, 0, 2,
                               (VkBuffer[]) { vbo, vbo },
                               (VkDeviceSize[]) { (4 + i * 2) * sizeof(float), 0 });
        vkCmdDraw(secondary, 1, 1, 0, 0);
    }

    qoEndCommandBuffer(secondary);

    vkCmdExecuteCommands(t_cmd_buffer, 1, &secondary);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

static void
test_large_secondary(void)
{
    do_test_large_secondary(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
}

test_define {
    .name = "func.cmd-buffer.large-secondary",
    .start = test_large_secondary,
    .image_filename = "32x32-green.ref.png",
};

static void
test_large_secondary_no_simultaneous(void)
{
    do_test_large_secondary(0);
}

test_define {
    .name = "func.cmd-buffer.large-secondary-no-simultaneous",
    .start = test_large_secondary_no_simultaneous,
    .image_filename = "32x32-green.ref.png",
};
