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
        .memoryTypeIndex = t_mem_type_index_for_mmap);

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

static VkPipeline
create_pipeline()
{
    return qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        }});

}

static VkRenderPass
create_and_begin_render_pass(void)
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
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_RENDER_PASS_CONTENTS_SECONDARY_CMD_BUFFERS);

    return pass;
}

static VkCmdBuffer
make_secondary_cmd_buffer(VkRenderPass pass, VkPipeline pipeline,
                          VkCmdBufferOptimizeFlags opt_flags)
{
    VkCmdBuffer secondary =
        qoCreateCommandBuffer(t_device, t_cmd_pool,
                              .level = VK_CMD_BUFFER_LEVEL_SECONDARY);

    qoBeginCommandBuffer(secondary,
        .flags = opt_flags,
        .renderPass = pass,
        .framebuffer = t_framebuffer);

    vkCmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    return secondary;
}

static void
test_small_secondaries(void)
{
    VkBuffer vbo = make_vbo();
    VkPipeline pipeline = create_pipeline();
    VkRenderPass pass = create_and_begin_render_pass();

    VkCmdBuffer secondaries[1024];

    for (int i = 0; i < 1024; i++) {
        secondaries[i] =
            make_secondary_cmd_buffer(pass, pipeline,
                                      VK_CMD_BUFFER_OPTIMIZE_SMALL_BATCH_BIT);

        vkCmdBindVertexBuffers(secondaries[i], 0, 2,
                               (VkBuffer[]) { vbo, vbo },
                               (VkDeviceSize[]) { (4 + i * 2) * sizeof(float), 0 });
        vkCmdDraw(secondaries[i], 1, 1, 0, 0);
        qoEndCommandBuffer(secondaries[i]);
    }

    vkCmdExecuteCommands(t_cmd_buffer, 1024, secondaries);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

test_define {
    .name = "func.cmd-buffer.small-secondaries",
    .start = test_small_secondaries,
    .image_filename = "32x32-green.ref.png",
};

static void
do_test_large_secondary(VkCmdBufferOptimizeFlags opt_flags)
{
    VkBuffer vbo = make_vbo();
    VkPipeline pipeline = create_pipeline();
    VkRenderPass pass = create_and_begin_render_pass();

    VkCmdBuffer secondary =
        make_secondary_cmd_buffer(pass, pipeline, opt_flags);

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
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

static void
test_large_secondary(void)
{
    do_test_large_secondary(0);
}

test_define {
    .name = "func.cmd-buffer.large-secondary",
    .start = test_large_secondary,
    .image_filename = "32x32-green.ref.png",
};

static void
test_large_secondary_no_simultaneous(void)
{
    do_test_large_secondary(VK_CMD_BUFFER_OPTIMIZE_NO_SIMULTANEOUS_USE_BIT);
}

test_define {
    .name = "func.cmd-buffer.large-secondary-no-simultaneous",
    .start = test_large_secondary_no_simultaneous,
    .image_filename = "32x32-green.ref.png",
};
