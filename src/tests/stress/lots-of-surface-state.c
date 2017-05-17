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
#include <math.h>
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
test_lots_of_surface_state(VkShaderModule vs, VkShaderModule fs,
                           VkShaderStageFlagBits ubo_stage,
                           bool use_dynamic_offsets)
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

    //  Create the descriptor set layout.
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 1,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = use_dynamic_offsets ?
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 12,
                    .stageFlags = ubo_stage,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 8,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        },
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0,
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
            .layout = pipeline_layout,
            .renderPass = pass,
            .subpass = 0,
        }});

    size_t ubo_size = 1024 * 3 * sizeof(float);

    VkBuffer ubo =
        qoCreateBuffer(t_device, .size = ubo_size,
                       .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDeviceMemory ubo_mem = qoAllocBufferMemory(t_device, ubo,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    float *const ubo_map = qoMapMemory(t_device, ubo_mem, /*offset*/ 0,
                                       ubo_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, ubo, ubo_mem, /*offset*/ 0);

    size_t vbo_size = 32 * 32 * 2 * sizeof(float);

    VkBuffer vbo =
        qoCreateBuffer(t_device, .size = vbo_size,
                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vbo_mem = qoAllocBufferMemory(t_device, vbo,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1,
                           (VkBuffer[]) { vbo },
                           (VkDeviceSize[]) { 0 });

    const VkDescriptorPoolSize pool_sizes[2] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 12
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 12 * 1024
        }
    };

    const VkDescriptorPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = 1024,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes
    };

    VkDescriptorPool desc_pool;
    VkResult res = vkCreateDescriptorPool(t_device, &create_info, NULL,
                                          &desc_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_descriptor_pool(t_device, desc_pool);

    VkDescriptorSet set[1024];
    if (use_dynamic_offsets) {
        // Allocate and set up a single descriptor set.  We'll just re-bind
        // it with new dynamic offsets each time.
        set[0] = qoAllocateDescriptorSet(t_device,
                                         .descriptorPool = desc_pool,
                                         .pSetLayouts = &set_layout);

        VkDescriptorBufferInfo buffer_info[12];
        for (int i = 0; i < 12; i++) {
            buffer_info[i] = (VkDescriptorBufferInfo) {
                .buffer = ubo,
                .offset = 0,
                .range = 4,
            };
        }

        vkUpdateDescriptorSets(t_device,
            /*writeCount*/ 1,
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set[0],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 12,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .pBufferInfo = buffer_info,
                },
            }, 0, NULL);
    } else {
        VkDescriptorSetLayout layouts[1024];
        for (int i = 0; i < 1024; i++)
            layouts[i] = set_layout;

        VkResult result = vkAllocateDescriptorSets(t_device,
            &(VkDescriptorSetAllocateInfo) {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = desc_pool,
                .descriptorSetCount = 1024,
                .pSetLayouts = layouts,
            }, set);
        t_assert(result == VK_SUCCESS);
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
            VkDescriptorBufferInfo buffer_info[12];
            for (int j = 0; j < 12; j++) {
                buffer_info[j] = (VkDescriptorBufferInfo) {
                    .buffer = ubo,
                    .offset = offsets[j],
                    .range = 4,
                };
            }

            vkUpdateDescriptorSets(t_device,
                /*writeCount*/ 1,
                (VkWriteDescriptorSet[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 12,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = buffer_info,
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
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

static void
test_lots_of_surface_state_vs(bool use_dynamic_offsets)
{
    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 0) out vec4 v_color;
        layout(set = 0, binding =  0) uniform block { float f; } u[12];
        void main()
        {
            float zero = u[0].f + u[1].f + u[2].f + u[3].f + u[4].f + u[5].f;
            float one = u[6].f + u[7].f + u[8].f + u[9].f + u[10].f + u[11].f;

            v_color = vec4(zero, one, zero, one);
            gl_Position = a_position;
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) in vec4 v_color;
        layout(location = 0) out vec4 f_color;
        void main()
        {
            f_color = v_color;
        }
    );

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_VERTEX_BIT,
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
    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    // The fragment shader takes 12 UBOs.
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(set = 0, binding =  0) uniform block { float f; } u[12];
        void main()
        {
            float zero = u[0].f + u[1].f + u[2].f + u[3].f + u[4].f + u[5].f;
            float one = u[6].f + u[7].f + u[8].f + u[9].f + u[10].f + u[11].f;

            f_color = vec4(zero, one, zero, one);
        }
    );

    test_lots_of_surface_state(vs, fs, VK_SHADER_STAGE_FRAGMENT_BIT,
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

static void
test_lots_of_surface_state_cs(bool use_dynamic_offsets)
{
    // The compute shader takes 12 UBOs and one SSBO.
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform block { float f; } u[12];
        layout(set = 0, binding = 1) buffer Storage {
            float zero;
            float one;
        };
        void main()
        {
            zero = u[0].f + u[1].f + u[2].f + u[3].f + u[4].f + u[5].f;
            one = u[6].f + u[7].f + u[8].f + u[9].f + u[10].f + u[11].f;
        }
    );

    //  Create the descriptor set layout.
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 2,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = use_dynamic_offsets ?
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 12,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL,
            },
            {
                .binding = 1,
                .descriptorType = use_dynamic_offsets ?
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipeline pipeline;
    vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(const VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = cs,
                .pName = "main",
            },
            .layout = pipeline_layout,
        }, NULL, &pipeline);
    t_cleanup_push_vk_pipeline(t_device, pipeline);

    size_t ubo_size = 1024 * 3 * sizeof(float);

    VkBuffer ubo =
        qoCreateBuffer(t_device, .size = ubo_size,
                       .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDeviceMemory ubo_mem = qoAllocBufferMemory(t_device, ubo,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    float *const ubo_map = qoMapMemory(t_device, ubo_mem, /*offset*/ 0,
                                       ubo_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, ubo, ubo_mem, /*offset*/ 0);

    srand(0);

    // Fill the first 1024 floats in the UBO with random numbers in the
    // range [-1, 2].  Why that range?  Some are negative but it's centered
    // around 0.5.  In other words, it seemed good at the time.
    for (int i = 0; i < 1024; i++) {
        ubo_map[i] = ((float)rand() / RAND_MAX) * 3 - 1;
    }

    size_t ssbo_size = 1024 * 2 * sizeof(float);

    VkBuffer ssbo =
        qoCreateBuffer(t_device, .size = ssbo_size,
                       .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDeviceMemory ssbo_mem = qoAllocBufferMemory(t_device, ssbo,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    qoBindBufferMemory(t_device, ssbo, ssbo_mem, /*offset*/ 0);

    vkCmdPipelineBarrier(t_cmd_buffer,
                         VK_PIPELINE_STAGE_HOST_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         false, 0, NULL, 1,
        (VkBufferMemoryBarrier[]) {
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                .buffer = ubo,
                .offset = 0,
                .size = ubo_size,
            },
        }, 0, NULL);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    const VkDescriptorPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = 1024,
        .poolSizeCount = 4,
        .pPoolSizes = (const VkDescriptorPoolSize[]) {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = 12,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 12 * 1024,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                .descriptorCount = 1,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1 * 1024,
            },
        }
    };

    VkDescriptorPool desc_pool;
    VkResult res = vkCreateDescriptorPool(t_device, &create_info, NULL,
                                          &desc_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_descriptor_pool(t_device, desc_pool);

    VkDescriptorSet set[1024];
    if (use_dynamic_offsets) {
        // Allocate and set up a single descriptor set.  We'll just re-bind
        // it with new dynamic offsets each time.
        set[0] = qoAllocateDescriptorSet(t_device,
                                         .descriptorPool = desc_pool,
                                         .pSetLayouts = &set_layout);

        VkDescriptorBufferInfo buffer_info[12];
        for (int i = 0; i < 12; i++) {
            buffer_info[i] = (VkDescriptorBufferInfo) {
                .buffer = ubo,
                .offset = 0,
                .range = 4,
            };
        }


        vkUpdateDescriptorSets(t_device,
            /*writeCount*/ 2,
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set[0],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 12,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .pBufferInfo = buffer_info,
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set[0],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                    .pBufferInfo = &(const VkDescriptorBufferInfo) {
                        .buffer = ssbo,
                        .offset = 0,
                        .range = 8,
                    },
                },
            }, 0, NULL);
    } else {
        VkDescriptorSetLayout layouts[1024];
        for (int i = 0; i < 1024; i++)
            layouts[i] = set_layout;

        VkResult result = vkAllocateDescriptorSets(t_device,
            &(VkDescriptorSetAllocateInfo) {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = desc_pool,
                .descriptorSetCount = 1024,
                .pSetLayouts = layouts,
            }, set);
        t_assert(result == VK_SUCCESS);
    }

    for (int i = 0; i < 1024; i++) {
        uint32_t offsets[13];

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

        /* SSBO offset */
        offsets[12] = i * 2 * sizeof(float);

        if (use_dynamic_offsets) {
            vkCmdBindDescriptorSets(t_cmd_buffer,
                                    VK_PIPELINE_BIND_POINT_COMPUTE,
                                    pipeline_layout, 0, 1,
                                    set, 13, offsets);
        } else {
            VkDescriptorBufferInfo buffer_info[12];
            for (int j = 0; j < 12; j++) {
                buffer_info[j] = (VkDescriptorBufferInfo) {
                    .buffer = ubo,
                    .offset = offsets[j],
                    .range = 4,
                };
            }

            vkUpdateDescriptorSets(t_device,
                /*writeCount*/ 2,
                (VkWriteDescriptorSet[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 12,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = buffer_info,
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set[i],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pBufferInfo = &(const VkDescriptorBufferInfo) {
                            .buffer = ssbo,
                            .offset = offsets[12],
                            .range = 8,
                        },
                    },
                }, 0, NULL);

            vkCmdBindDescriptorSets(t_cmd_buffer,
                                    VK_PIPELINE_BIND_POINT_COMPUTE,
                                    pipeline_layout, 0, 1,
                                    &set[i], 0, NULL);
        }

        vkCmdDispatch(t_cmd_buffer, 1, 1, 1);
    }

    vkCmdPipelineBarrier(t_cmd_buffer,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_HOST_BIT,
                         false, 0, NULL, 1,
        (VkBufferMemoryBarrier[]) {
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
                .buffer = ssbo,
                .offset = 0,
                .size = ssbo_size,
            },
        }, 0, NULL);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    float *const ssbo_map = qoMapMemory(t_device, ssbo_mem, /*offset*/ 0,
                                        ssbo_size, /*flags*/ 0);

    for (unsigned i = 0; i < 1024; i++) {
        t_assert(fabs(ssbo_map[i * 2 + 0] - 0.0f) < 0.0001);
        t_assert(fabs(ssbo_map[i * 2 + 1] - 1.0f) < 0.0001);
    }
}

static void
test_lots_of_surface_state_cs_dynamic(void)
{
    test_lots_of_surface_state_cs(true);
}

test_define {
    .name = "stress.lots-of-surface-state.cs.dynamic",
    .start = test_lots_of_surface_state_cs_dynamic,
    .no_image = true,
};

static void
test_lots_of_surface_state_cs_static(void)
{
    test_lots_of_surface_state_cs(false);
}

test_define {
    .name = "stress.lots-of-surface-state.cs.static",
    .start = test_lots_of_surface_state_cs_static,
    .no_image = true,
};
