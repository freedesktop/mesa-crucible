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
    VkDescriptorSetLayout set_layout;
    vkCreateDescriptorSetLayout(t_device,
        &(VkDescriptorSetLayoutCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .count = 12,
                    .stageFlags = (1 << ubo_stage),
                    .pImmutableSamplers = NULL,
                },
            },
        },
        &set_layout);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                             set_layout);

    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(t_device,
        &(VkPipelineLayoutCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .descriptorSetCount = 1,
            .pSetLayouts = &set_layout,
        },
        &pipeline_layout);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                             pipeline_layout);

    VkPipelineVertexInputCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_CREATE_INFO,
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

    struct cru_GraphicsPipelineCreateInfo cru_info = {
        .sType = CRU_STRUCTURE_TYPE_PIPELINE_CREATE_INFO,
        .pNext = &vi_create_info,

        .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .vertex_shader = vs,
        .fragment_shader = fs,
    };

    VkPipeline pipeline;
    cru_CreateGraphicsPipeline(t_device,
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &cru_info,
            .flags = 0,
            .layout = pipeline_layout
        },
        &pipeline);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_PIPELINE, pipeline);

    /* Create the UBO and vertex buffer and their associated memory */
    VkBuffer ubo;
    vkCreateBuffer(t_device,
        &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 1024 * 3 * sizeof(float),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .flags = 0
        },
        &ubo);

    VkMemoryRequirements ubo_reqs =
       qoBufferGetMemoryRequirements(t_device, ubo);

    VkBuffer vbo;
    vkCreateBuffer(t_device,
        &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 32 * 32 * 2 * sizeof(float),
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .flags = 0,
        },
        &vbo);

    VkMemoryRequirements vbo_reqs =
       qoBufferGetMemoryRequirements(t_device, vbo);

    size_t mem_size = ubo_reqs.size + vbo_reqs.size;

    VkDeviceMemory mem;
    vkAllocMemory(t_device,
        &(VkMemoryAllocInfo) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO,
            .allocationSize = mem_size,
            .memProps = VK_MEMORY_PROPERTY_HOST_DEVICE_COHERENT_BIT,
            .memPriority = VK_MEMORY_PRIORITY_NORMAL,
        },
        &mem);

    void *map;
    vkMapMemory(t_device, mem, 0, mem_size, 0, &map);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER, ubo,
                            /*index*/ 0, mem, 0);
    float *ubo_map = map;
    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER, vbo,
                            /*index*/ 0, mem, ubo_reqs.size);
    float *vbo_map = map + ubo_reqs.size;

    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_BUFFER, ubo);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_BUFFER, vbo);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_DEVICE_MEMORY, mem);

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

    VkRenderPass pass;
    vkCreateRenderPass(t_device,
        &(VkRenderPassCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .renderArea = {{0, 0}, {t_width, t_height}},
            .colorAttachmentCount = 1,
            .extent = { },
            .sampleCount = 1,
            .layers = 1,
            .pColorFormats = (VkFormat[]) { VK_FORMAT_R8G8B8A8_UNORM },
            .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
            .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_CLEAR },
            .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE },
            .pColorLoadClearValues = (VkClearColor[]) {
                {
                    .color = { .floatColor = { 1.0, 0.0, 0.0, 1.0 } },
                    .useRawValue = false,
                },
            },
            .depthStencilFormat = VK_FORMAT_UNDEFINED,
        },
        &pass);

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
        .pScissors = (VkRect[]) {
            {{  0,  0 }, {t_width, t_height}},
        }
    );
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_VIEWPORT, vp_state);

    VkDynamicRsState rs_state =
        qoCreateDynamicRasterState(t_device, .pointSize = 1.0);
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_RASTER, rs_state);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1,
                           (VkBuffer[]) { vbo },
                           (VkDeviceSize[]) { 0 });

    VkBufferView ubo_view;
    vkCreateBufferView(t_device,
        &(VkBufferViewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .buffer = ubo,
            .viewType = VK_BUFFER_VIEW_TYPE_RAW,
            .format = VK_FORMAT_R32_SFLOAT,
            .offset = 0,
            .range = ubo_reqs.size,
        },
        &ubo_view);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_BUFFER_VIEW, ubo_view);

    VkDescriptorSet set;
    uint32_t set_count = 1;
    vkAllocDescriptorSets(t_device, /*pool*/ 0,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          1, &set_layout, &set, &set_count);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_DESCRIPTOR_SET, set);

    VkBufferViewAttachInfo attach_info[12];
    for (int i = 0; i < 12; i++)
        attach_info[i] = (VkBufferViewAttachInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
            .view = ubo_view,
        };

   vkUpdateDescriptors(t_device, set, 1,
        (const void * []) {
            &(VkUpdateBuffers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_BUFFERS,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .arrayIndex = 0,
                .binding = 0,
                .count = 12,
                .pBufferViews = attach_info,
            },
        });

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
                                VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 1,
                                &set, 12, offsets);

        vkCmdDraw(t_cmd_buffer, i, 1, 0, 1);
    }

    vkCmdEndRenderPass(t_cmd_buffer, pass);
}

static void
test_lots_of_surface_state_vs(void)
{
    static const char vs_source[] = GLSL(330,
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

    VkShader vs;
    vkCreateShader(t_device,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .codeSize = sizeof(vs_source),
            .pCode = vs_source,
            .flags = 0,
        },
        &vs);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_SHADER, vs);

    /* The fragment shader takes 12 UBOs */
    static const char fs_source[] = GLSL(330,
        in vec4 v_color;
        out vec4 f_color;
        void main()
        {
            f_color = v_color;
        }
    );

    VkShader fs;
    vkCreateShader(t_device,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .codeSize = sizeof(fs_source),
            .pCode = fs_source,
            .flags = 0,
        },
        &fs);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_SHADER, fs);


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
    static const char vs_source[] = GLSL(330,
        layout(location = 0) in vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    VkShader vs;
    vkCreateShader(t_device,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .codeSize = sizeof(vs_source),
            .pCode = vs_source,
            .flags = 0,
        },
        &vs);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_SHADER, vs);

    /* The fragment shader takes 12 UBOs */
    static const char fs_source[] = GLSL(330,
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

    VkShader fs;
    vkCreateShader(t_device,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .codeSize = sizeof(fs_source),
            .pCode = fs_source,
            .flags = 0,
        },
        &fs);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_SHADER, fs);

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_FRAGMENT);
}

cru_define_test {
    .name = "stress.lots-of-surface-state.fs",
    .start = test_lots_of_surface_state_fs,
    .image_filename = "32x32-green.ref.png",
};
