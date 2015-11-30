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

#include "first-spirv.h"

static VkPipeline
create_pipeline(VkDevice device, VkPipelineLayout pipeline_layout,
                VkRenderPass pass)
{
    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 1) in vec4 a_color;
        layout(set = 0, binding = 0) uniform block1 {
            vec4 color;
        } u1[2];
        layout(set = 1, binding = 0) uniform block3 {
            vec4 color;
        } u2;
        layout(location = 0) out vec4 v_color;
        void main()
        {
            gl_Position = a_position;
            v_color = a_color +
                      u1[0].color +
                      u1[1].color +
                      u2.color;
        }
    );

    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(location = 0) in vec4 v_color;
        layout(set = 0, binding = 1) uniform sampler2D tex;
        void main()
        {
            f_color = v_color + texture(tex, vec2(0.1, 0.1));
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .bindingCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 16, // TODO: What is this? rgba_f32? xyzw_f32?
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
            },
            {
                .binding = 1,
                .strideInBytes = 0,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
            },
        },
        .attributeCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0,
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0,
            },
        },
    };

    return qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
}

static void
test(void)
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

    VkDescriptorSetLayout set_layout[2];

    set_layout[0] = qoCreateDescriptorSetLayout(t_device,
            .count = 2,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .arraySize = 2,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .arraySize = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    set_layout[1] = qoCreateDescriptorSetLayout(t_device,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .arraySize = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .descriptorSetCount = 2,
        .pSetLayouts = set_layout);

    VkPipeline pipeline = create_pipeline(t_device, pipeline_layout, pass);

    VkDescriptorSet set[2];
    qoAllocDescriptorSets(t_device, QO_NULL_DESCRIPTOR_POOL,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          2, set_layout, set);

    static const float uniform_data[12] = {
        0.0, 0.2, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.5, 0.5
    };

    VkBuffer uniform_buffer = qoCreateBuffer(t_device,
        .size = sizeof(uniform_data));

    VkDeviceMemory uniform_mem = qoAllocBufferMemory(t_device, uniform_buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    qoBindBufferMemory(t_device, uniform_buffer,
                       uniform_mem, /*offset*/ 0);

    memcpy(qoMapMemory(t_device, uniform_mem, /*offset*/ 0,
                       sizeof(uniform_data), /*flags*/ 0),
           uniform_data,
           sizeof(uniform_data));

    static const float vertex_data[] = {
        // Triangle coordinates
        -0.5, -0.5, 0.0, 1.0,
         0.5, -0.5, 0.0, 1.0,
         0.0,  0.5, 0.0, 1.0,

         // Color
         1.0,  0.0, 0.0, 0.2,
    };

    VkBuffer vertex_buffer = qoCreateBuffer(t_device,
        .size = sizeof(vertex_data),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vertex_mem = qoAllocBufferMemory(t_device, vertex_buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    memcpy(qoMapMemory(t_device, vertex_mem, /*offset*/ 0,
                       sizeof(vertex_data), /*flags*/ 0),
           vertex_data,
           sizeof(vertex_data));

    qoBindBufferMemory(t_device, vertex_buffer, vertex_mem,
                       /*offset*/ 0);

    const uint32_t texture_width = 16, texture_height = 16;

    VkImage texture = qoCreateImage(t_device,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .extent = {
            .width = texture_width,
            .height = texture_height,
            .depth = 1,
        });

    VkMemoryRequirements texture_reqs =
        qoGetImageMemoryRequirements(t_device, texture);

    VkDeviceMemory texture_mem = qoAllocMemoryFromRequirements(t_device,
        &texture_reqs,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    qoBindImageMemory(t_device, texture, texture_mem, /*offset*/ 0);

    // Initialize texture data
    memset(qoMapMemory(t_device, texture_mem, /*offset*/ 0,
                       texture_reqs.size, /*flags*/ 0),
           192, texture_reqs.size);

    VkImageView tex_view = qoCreateImageView(t_device,
        .image = texture,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM);

    VkSampler sampler = qoCreateSampler(t_device,
        .magFilter = VK_TEX_FILTER_LINEAR,
        .minFilter = VK_TEX_FILTER_LINEAR,
        .mipMode = VK_TEX_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_TEX_ADDRESS_MODE_CLAMP,
        .addressModeV = VK_TEX_ADDRESS_MODE_CLAMP,
        .addressModeW = VK_TEX_ADDRESS_MODE_CLAMP,
        .mipLodBias = 0,
        .maxAnisotropy = 0,
        .compareOp = VK_COMPARE_OP_GREATER,
        .minLod = 0,
        .maxLod = 0,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);

    vkUpdateDescriptorSets(t_device,
        3, /* writeCount */
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .destSet = set[0],
                .destBinding = 0,
                .destArrayElement = 0,
                .count = 2,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pDescriptors = (VkDescriptorInfo[]) {
                    {
                        .bufferInfo = {
                            .buffer = uniform_buffer,
                            .offset = 0,
                            .range = 64,
                        },
                    },
                    {
                        .bufferInfo = {
                            .buffer = uniform_buffer,
                            .offset = 4 * sizeof(float),
                            .range = 64,
                        },
                    }
                },
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .destSet = set[0],
                .destBinding = 1,
                .destArrayElement = 0,
                .count = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pDescriptors = (VkDescriptorInfo[]) {
                    {
                        .imageView = tex_view,
                        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .sampler = sampler,
                    },
                },
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .destSet = set[1],
                .destBinding = 0,
                .destArrayElement = 0,
                .count = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pDescriptors = (VkDescriptorInfo[]) {
                    {
                        .bufferInfo = {
                            .buffer = uniform_buffer,
                            .offset = 4 * sizeof(float) * 2,
                            .range = 64,
                        },
                    }
                },
            },
        }, 0, NULL);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_RENDER_PASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, 3 * 4 * sizeof(float) });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set[0], 0, NULL);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 1, 1,
                            &set[1], 0, NULL);
    vkCmdDraw(t_cmd_buffer, /*vertexCount*/ 3, /*instanceCount*/ 1,
              /*firstVertex*/ 0, /*firstInstance*/ 0);
    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

test_define {
    .name = "func.first",
    .start = test,
};
