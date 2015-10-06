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

#include <stdio.h>
#include "tapi/t.h"

#include "lots-of-surface-state-spirv.h"

// This file implements a test that pokes a particular aspect of the Intel
// open-source vulkan driver.  In particular, the Intel hardware has a
// maximum 16 bits in which to store the binding table offsets so all
// binding tables must be placed in the first 64K relative to the surface
// state base address.  This test creates a single command buffer that uses
// more than 64K worth of binding tablees while consuming a very small
// amount of actual batch buffer space.

static void
test_lots_of_surface_state(VkShader vs, VkShader fs, VkShaderStage ubo_stage,
                           bool use_dynamic_offsets)
{
    //  Create the descriptor set layout.
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = use_dynamic_offsets ?
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
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

    VkDeviceMemory ubo_mem = qoAllocBufferMemory(t_device, ubo,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    float *const ubo_map = qoMapMemory(t_device, ubo_mem, /*offset*/ 0,
                                       ubo_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, ubo, ubo_mem, /*offset*/ 0);

    size_t vbo_size = 32 * 32 * 2 * sizeof(float);

    VkBuffer vbo =
        qoCreateBuffer(t_device, .size = vbo_size,
                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vbo_mem = qoAllocBufferMemory(t_device, vbo,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    float *const vbo_map = qoMapMemory(t_device, vbo_mem, /*offset*/ 0,
                                       vbo_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, vbo, vbo_mem, /*offset*/ 0);

    // Fill the VBO with 2D coordinates. One per pixel in a 32x32 image.
    for (int x = 0; x < 32; x++) {
        for (int y = 0; y < 32; y++) {
            vbo_map[2 * (y * 32 + x) + 0] = (x + 0.5) / 16.0 - 1.0;
            vbo_map[2 * (y * 32 + x) + 1] = (y + 0.5) / 16.0 - 1.0;
        }
    }

    srand(0);

    // Fill the first 1024 floats in the UBO with random numbers in the
    // range [-1, 2].  Why that range?  Some are negative but it's centered
    // around 0.5.  In other words, it seemed good at the time.
    for (int i = 0; i < 1024; i++) {
        ubo_map[i] = ((float)rand() / RAND_MAX) * 3 - 1;
    }

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
                { .color = { .f32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_RENDER_PASS_CONTENTS_INLINE);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1,
                           (VkBuffer[]) { vbo },
                           (VkDeviceSize[]) { 0 });

    VkDescriptorSet set[1024];
    if (use_dynamic_offsets) {
        // Allocate and set up a single descriptor set.  We'll just re-bind
        // it with new dynamic offsets each time.
        qoAllocDescriptorSets(t_device, QO_NULL_DESCRIPTOR_POOL,
                              VK_DESCRIPTOR_SET_USAGE_STATIC,
                              1, &set_layout, set);

        VkBufferView ubo_view = qoCreateBufferView(t_device,
            .buffer = ubo,
            .format = VK_FORMAT_R32_SFLOAT,
            .range = ubo_size);

        VkDescriptorInfo desc_info[12];
        for (int i = 0; i < 12; i++)
            desc_info[i] = (VkDescriptorInfo) { .bufferView = ubo_view };

        vkUpdateDescriptorSets(t_device,
            /*writeCount*/ 1,
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .destSet = set[0],
                    .destBinding = 0,
                    .destArrayElement = 0,
                    .count = 12,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pDescriptors = desc_info,
                },
            }, 0, NULL);
    } else {
        VkDescriptorSetLayout layouts[1024];
        for (int i = 0; i < 1024; i++)
            layouts[i] = set_layout;

        qoAllocDescriptorSets(t_device, QO_NULL_DESCRIPTOR_POOL,
                              VK_DESCRIPTOR_SET_USAGE_STATIC,
                              1024, layouts, set);
    }

    for (int i = 0; i < 1024; i++) {
        uint32_t offsets[12];

        // Compute the offsets and the extra data value.  These are
        // computed so that the sum of the first 6 UBO entries will be zero
        // and the sum of the second 6 will be one.
        float sum = 0;
        for (int j = 0; j < 5; j++) {
            int idx = rand() % 1024;
            offsets[j] = idx * sizeof(float);
            sum += ubo_map[idx];
        }

        // The first batch should sum to zero.
        ubo_map[1024 + i * 2 + 0] = 0 - sum;
        offsets[5] = (1024 + i * 2 + 0) * sizeof(float);

        sum = 0;
        for (int j = 6; j < 11; j++) {
            int idx = rand() % 1024;
            offsets[j] = idx * sizeof(float);
            sum += ubo_map[idx];
        }

        // The second batch should sum to one.
        ubo_map[1024 + i * 2 + 1] = 1 - sum;
        offsets[11] = (1024 + i * 2 + 1) * sizeof(float);

        if (use_dynamic_offsets) {
            vkCmdBindDescriptorSets(t_cmd_buffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layout, 0, 1,
                                    set, 12, offsets);
        } else {
            VkDescriptorInfo desc_info[12];
            for (int j = 0; j < 12; j++) {
                desc_info[j].bufferView = qoCreateBufferView(t_device,
                    .buffer = ubo,
                    .format = VK_FORMAT_R32_SFLOAT,
                    .offset = offsets[j],
                    .range = ubo_size);
            }

            vkUpdateDescriptorSets(t_device,
                /*writeCount*/ 1,
                (VkWriteDescriptorSet[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .destSet = set[i],
                        .destBinding = 0,
                        .destArrayElement = 0,
                        .count = 12,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pDescriptors = desc_info,
                    },
                }, 0, NULL);

            vkCmdBindDescriptorSets(t_cmd_buffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layout, 0, 1,
                                    &set[i], 0, NULL);
        }

        vkCmdDraw(t_cmd_buffer, 1, 1, i, 0);
    }

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

static void
test_lots_of_surface_state_vs(bool use_dynamic_offsets)
{
    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 0) out vec4 v_color;
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
        layout(location = 0) in vec4 v_color;
        layout(location = 0) out vec4 f_color;
        void main()
        {
            f_color = v_color;
        }
    );

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_VERTEX,
                               use_dynamic_offsets);
}

static void
test_lots_of_surface_state_vs_dynamic(void)
{
    test_lots_of_surface_state_vs(true);
}

test_define {
    .name = "stress.lots-of-surface-state.vs.dynamic",
    .start = test_lots_of_surface_state_vs_dynamic,
    .image_filename = "32x32-green.ref.png",
};

static void
test_lots_of_surface_state_vs_static(void)
{
    test_lots_of_surface_state_vs(false);
}

test_define {
    .name = "stress.lots-of-surface-state.vs.static",
    .start = test_lots_of_surface_state_vs_static,
    .image_filename = "32x32-green.ref.png",
};

static void
test_lots_of_surface_state_fs(bool use_dynamic_offsets)
{
    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    // The fragment shader takes 12 UBOs.
    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
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

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_FRAGMENT,
                               use_dynamic_offsets);
}

static void
test_lots_of_surface_state_fs_dynamic(void)
{
    test_lots_of_surface_state_fs(true);
}

test_define {
    .name = "stress.lots-of-surface-state.fs.dynamic",
    .start = test_lots_of_surface_state_fs_dynamic,
    .image_filename = "32x32-green.ref.png",
};

static void
test_lots_of_surface_state_fs_static(void)
{
    test_lots_of_surface_state_fs(false);
}

test_define {
    .name = "stress.lots-of-surface-state.fs.static",
    .start = test_lots_of_surface_state_fs_static,
    .image_filename = "32x32-green.ref.png",
};
