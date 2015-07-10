/*
 * Copyright 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <crucible/cru.h>
#include <stdio.h>

#include "lots-of-surface-state-spirv.h"

/* This file implements a test that pokes a particular aspect of the Intel
 * open-source vulkan driver.  In particular, the Intel hardware has a
 * maximum 16 bits in which to store the binding table offsets so all
 * binding tables must be placed in the first 64K relative to the surface
 * state base address.  This test creates a single command buffer that uses
 * more than 64K worth of binding tablees while consuming a very small
 * amount of actual batch buffer space.
 */

static void
test_lots_of_surface_state(VkShader vs, VkShader fs, VkShaderStage ubo_stage)
{
    /* Create the descriptor set layout. */
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .arraySize = 12,
                    .stageFlags = (1 << ubo_stage),
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .descriptorSetCount = 1,
        .pSetLayouts = &set_layout);

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .bindingCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 8,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
            },
        },
        .attributeCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offsetInBytes = 0,
            },
        },
    };

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_create_info,
            .flags = 0,
            .layout = pipeline_layout
        }});

    size_t ubo_size = 1024 * 3 * sizeof(float);

    VkBuffer ubo =
        qoCreateBuffer(t_device, .size = ubo_size,
                       .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkMemoryRequirements ubo_reqs =
       qoGetBufferMemoryRequirements(t_device, ubo);

    VkDeviceMemory ubo_mem = qoAllocMemory(t_device,
        .allocationSize = ubo_reqs.size,
        .memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    float *const ubo_map = qoMapMemory(t_device, ubo_mem, /*offset*/ 0,
                                       ubo_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, ubo, ubo_mem, /*offset*/ 0);

    size_t vbo_size = 32 * 32 * 2 * sizeof(float);

    VkBuffer vbo =
        qoCreateBuffer(t_device, .size = vbo_size,
                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkMemoryRequirements vbo_reqs =
       qoGetBufferMemoryRequirements(t_device, vbo);

    VkDeviceMemory vbo_mem = qoAllocMemory(t_device,
        .allocationSize = vbo_reqs.size,
        .memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    float *const vbo_map = qoMapMemory(t_device, vbo_mem, /*offset*/ 0,
                                       vbo_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, vbo, vbo_mem, /*offset*/ 0);

    /* Fill the VBO with 2D coordinates. One per pixel in a 32x32 image */
    for (int x = 0; x < 32; x++) {
        for (int y = 0; y < 32; y++) {
            vbo_map[2 * (y * 32 + x) + 0] = (x + 0.5) / 16.0 - 1.0;
            vbo_map[2 * (y * 32 + x) + 1] = (y + 0.5) / 16.0 - 1.0;
        }
    }

    srand(0);

    /* Fill the first 1024 floats in the UBO with random numbers in the
     * range [-1, 2].  Why that range?  Some are negative but it's centered
     * around 0.5.  In other words, it seemed good at the time.
     */
    for (int i = 0; i < 1024; i++) {
        ubo_map[i] = ((float)rand() / RAND_MAX) * 3 - 1;
    }

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .renderArea = {{0, 0}, {t_width, t_height}},
        .pColorFormats = (VkFormat[]) { VK_FORMAT_R8G8B8A8_UNORM },
        .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_CLEAR },
        .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE },
        .pColorLoadClearValues = (VkClearColorValue[]) {
            { .f32 = { 1.0, 0.0, 0.0, 1.0 } },
        });

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBegin) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
        });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDynamicVpState vp_state = qoCreateDynamicViewportState(t_device,
        .viewportAndScissorCount = 1,
        .pViewports = (VkViewport[]) {
            {
                .originX = 0,
                .originY = 0,
                .width = t_width,
                .height = t_height,
                .minDepth = 0,
                .maxDepth = 1
            },
        },
        .pScissors = (VkRect2D[]) {
            {{  0,  0 }, {t_width, t_height}},
        }
    );
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_VIEWPORT, vp_state);

    VkDynamicRsState rs_state = qoCreateDynamicRasterState(t_device);
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_RASTER, rs_state);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1,
                           (VkBuffer[]) { vbo },
                           (VkDeviceSize[]) { 0 });

    VkBufferView ubo_view = qoCreateBufferView(t_device,
        .buffer = ubo,
        .viewType = VK_BUFFER_VIEW_TYPE_RAW,
        .format = VK_FORMAT_R32_SFLOAT,
        .range = ubo_reqs.size);

    VkDescriptorSet set;
    qoAllocDescriptorSets(t_device, /*pool*/ 0,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          1, &set_layout, &set);

    VkDescriptorInfo desc_info[12];
    for (int i = 0; i < 12; i++)
        desc_info[i] = (VkDescriptorInfo) { .bufferView = ubo_view };

    vkUpdateDescriptorSets(t_device,
        1, /* writeCount */
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .destSet = set,
                .destBinding = 0,
                .destArrayElement = 0,
                .count = 12,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pDescriptors = desc_info,
            },
        }, 0, NULL);

    for (int i = 0; i < 1024; i++) {
        uint32_t offsets[12];

        /* Compute the offsets and the extra data value.  These are
         * computed so that the sum of the first 6 UBO entries will be zero
         * and the sum of the second 6 will be one.
         */
        float sum = 0;
        for (int j = 0; j < 5; j++) {
            int idx = rand() % 1024;
            offsets[j] = idx * sizeof(float);
            sum += ubo_map[idx];
        }

        /* The first batch should sum to zero */
        ubo_map[1024 + i * 2 + 0] = 0 - sum;
        offsets[5] = (1024 + i * 2 + 0) * sizeof(float);

        sum = 0;
        for (int j = 6; j < 11; j++) {
            int idx = rand() % 1024;
            offsets[j] = idx * sizeof(float);
            sum += ubo_map[idx];
        }

        /* The second batch should sum to one */
        ubo_map[1024 + i * 2 + 1] = 1 - sum;
        offsets[11] = (1024 + i * 2 + 1) * sizeof(float);

        vkCmdBindDescriptorSets(t_cmd_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layout, 0, 1,
                                &set, 12, offsets);

        vkCmdDraw(t_cmd_buffer, i, 1, 0, 1);
    }

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, 0);
}

static void
test_lots_of_surface_state_vs(void)
{
    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        out vec4 v_color;
        layout(set = 0, binding =  0) uniform block0  { float f; } u0;
        layout(set = 0, binding =  1) uniform block1  { float f; } u1;
        layout(set = 0, binding =  2) uniform block2  { float f; } u2;
        layout(set = 0, binding =  3) uniform block3  { float f; } u3;
        layout(set = 0, binding =  4) uniform block4  { float f; } u4;
        layout(set = 0, binding =  5) uniform block5  { float f; } u5;
        layout(set = 0, binding =  6) uniform block6  { float f; } u6;
        layout(set = 0, binding =  7) uniform block7  { float f; } u7;
        layout(set = 0, binding =  8) uniform block8  { float f; } u8;
        layout(set = 0, binding =  9) uniform block9  { float f; } u9;
        layout(set = 0, binding = 10) uniform block10 { float f; } u10;
        layout(set = 0, binding = 11) uniform block11 { float f; } u11;
        void main()
        {
            float zero = u0.f + u1.f + u2.f + u3.f + u4.f + u5.f;
            float one = u6.f + u7.f + u8.f + u9.f + u10.f + u11.f;

            v_color = vec4(zero, one, zero, one);
            gl_Position = a_position;
        }
    );

    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        in vec4 v_color;
        out vec4 f_color;
        void main()
        {
            f_color = v_color;
        }
    );

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_VERTEX);
}

cru_define_test {
    .name = "stress.lots-of-surface-state.vs",
    .start = test_lots_of_surface_state_vs,
    .image_filename = "32x32-green.ref.png",
};

static void
test_lots_of_surface_state_fs(void)
{
    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    /* The fragment shader takes 12 UBOs */
    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        out vec4 f_color;
        layout(set = 0, binding =  0) uniform block0  { float f; } u0;
        layout(set = 0, binding =  1) uniform block1  { float f; } u1;
        layout(set = 0, binding =  2) uniform block2  { float f; } u2;
        layout(set = 0, binding =  3) uniform block3  { float f; } u3;
        layout(set = 0, binding =  4) uniform block4  { float f; } u4;
        layout(set = 0, binding =  5) uniform block5  { float f; } u5;
        layout(set = 0, binding =  6) uniform block6  { float f; } u6;
        layout(set = 0, binding =  7) uniform block7  { float f; } u7;
        layout(set = 0, binding =  8) uniform block8  { float f; } u8;
        layout(set = 0, binding =  9) uniform block9  { float f; } u9;
        layout(set = 0, binding = 10) uniform block10 { float f; } u10;
        layout(set = 0, binding = 11) uniform block11 { float f; } u11;
        void main()
        {
            float zero = u0.f + u1.f + u2.f + u3.f + u4.f + u5.f;
            float one = u6.f + u7.f + u8.f + u9.f + u10.f + u11.f;

            f_color = vec4(zero, one, zero, one);
        }
    );

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_FRAGMENT);
}

cru_define_test {
    .name = "stress.lots-of-surface-state.fs",
    .start = test_lots_of_surface_state_fs,
    .image_filename = "32x32-green.ref.png",
};
