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

#include <crucible/cru.h>

#include "first-spirv.h"

static void
create_pipeline(VkDevice device, VkPipeline *pipeline,
                VkPipelineLayout pipeline_layout)
{
    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 1) in vec4 a_color;
        layout(set = 0, binding = 0) uniform block1 {
            vec4 color;
        } u1;
        layout(set = 0, binding = 1) uniform block2 {
            vec4 color;
        } u2;
        layout(set = 1, binding = 0) uniform block3 {
            vec4 color;
        } u3;
        out vec4 v_color;
        void main()
        {
            gl_Position = a_position;
            v_color = a_color + u1.color + u2.color + u3.color;
        }
    );

    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        out vec4 f_color;
        in vec4 v_color;
        layout(set = 0, binding = 0) uniform sampler2D tex;
        void main()
        {
            f_color = v_color + texture2D(tex, vec2(0.1, 0.1));
        }
    );

    VkPipelineVertexInputCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_CREATE_INFO,
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

    *pipeline = qoCreateGraphicsPipeline(t_device,
        &(QoExtraGraphicsPipelineCreateInfo) {
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &vi_create_info,
            .flags = 0,
            .layout = pipeline_layout
        }});
}

static void
test(void)
{
    VkDescriptorSetLayout set_layout[2];

    set_layout[0] = qoCreateDescriptorSetLayout(t_device,
            .count = 3,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .count = 2,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .count = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .count = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    set_layout[1] = qoCreateDescriptorSetLayout(t_device,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .count = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .descriptorSetCount = 2,
        .pSetLayouts = set_layout);

    VkPipeline pipeline;
    create_pipeline(t_device, &pipeline, pipeline_layout);

    uint32_t set_count = 1;
    VkDescriptorSet set[2];
    vkAllocDescriptorSets(t_device, /*pool*/ 0,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          2, set_layout, set, &set_count);
    t_cleanup_push_vk_descriptor_set(t_device, set[0]);
    t_cleanup_push_vk_descriptor_set(t_device, set[1]);

    VkBuffer buffer = qoCreateBuffer(t_device, .size = 1024,
                                     .usage = VK_BUFFER_USAGE_GENERAL);

    VkBuffer vertex_buffer =
        qoCreateBuffer(t_device, .size = 1024,
                       .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkBuffer image_buffer =
        qoCreateBuffer(t_device, .size = 4 * t_width * t_height,
                       .usage = VK_BUFFER_USAGE_TRANSFER_DESTINATION_BIT);

    VkMemoryRequirements ib_reqs =
        qoBufferGetMemoryRequirements(t_device, image_buffer);

    size_t mem_size = ib_reqs.size + 2048 + 16 * 16 * 4;

    VkDeviceMemory mem = qoAllocMemory(t_device, .allocationSize = mem_size);
    void *map = qoMapMemory(t_device, mem, 0, mem_size, 0);
    memset(map, 192, mem_size);

    qoQueueBindBufferMemory(t_queue, buffer, /*index*/ 0, mem, 128);

    static const float color[12] = {
        0.0, 0.2, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.5, 0.5
    };
    memcpy(map + 128 + 16, color, sizeof(color));

    VkBufferView buffer_view[3];
    buffer_view[0] = qoCreateBufferView(t_device,
        .buffer = buffer,
        .viewType = VK_BUFFER_VIEW_TYPE_RAW,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = 16, .range = 64);

    buffer_view[1] = qoCreateBufferView(t_device,
        .buffer = buffer,
        .viewType = VK_BUFFER_VIEW_TYPE_RAW,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = 32, .range = 64);

    buffer_view[2] = qoCreateBufferView(t_device,
        .buffer = buffer,
        .viewType = VK_BUFFER_VIEW_TYPE_RAW,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = 48, .range = 64);

    qoQueueBindBufferMemory(t_queue, vertex_buffer, /*index*/ 0, mem, 1024);

    static const float vertex_data[] = {
        // Triangle coordinates
        -0.5, -0.5, 0.0, 1.0,
         0.5, -0.5, 0.0, 1.0,
         0.0,  0.5, 0.0, 1.0,

         // Color
         1.0,  0.0, 0.0, 0.2,
    };

    memcpy(map + 1024, vertex_data, sizeof(vertex_data));

    qoQueueBindBufferMemory(t_queue, image_buffer, /*index*/ 0, mem, 0);

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

    qoQueueBindImageMemory(t_queue, texture, /*index*/ 0, mem,
                           2048 + 256 * 256 * 4);

    VkImageView tex_view = qoCreateImageView(t_device,
        .image = texture,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM);

    VkSampler sampler = qoCreateSampler(t_device,
        .magFilter = VK_TEX_FILTER_LINEAR,
        .minFilter = VK_TEX_FILTER_LINEAR,
        .mipMode = VK_TEX_MIPMAP_MODE_NEAREST,
        .addressU = VK_TEX_ADDRESS_CLAMP,
        .addressV = VK_TEX_ADDRESS_CLAMP,
        .addressW = VK_TEX_ADDRESS_CLAMP,
        .mipLodBias = 0,
        .maxAnisotropy = 0,
        .compareOp = VK_COMPARE_OP_GREATER,
        .minLod = 0,
        .maxLod = 0,
        .borderColor = VK_BORDER_COLOR_TRANSPARENT_BLACK);

   vkUpdateDescriptors(t_device,
        set[0], 3,
        (const void * []) {
            &(VkUpdateBuffers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_BUFFERS,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .arrayIndex = 0,
                .binding = 0,
                .count = 2,
                .pBufferViews = (VkBufferViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view[0],
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view[1],
                    },
                },
            },
            &(VkUpdateImages) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_IMAGES,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .binding = 2,
                .count = 1,
                .pImageViews = (VkImageViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ATTACH_INFO,
                        .view = tex_view,
                        .layout = VK_IMAGE_LAYOUT_GENERAL,
                    },
                },
            },
            &(const VkUpdateSamplers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_SAMPLERS,
                .binding = 3,
                .count = 1,
                .pSamplers = (const VkSampler[]) { sampler }
            },
        });

    vkUpdateDescriptors(t_device,
        set[1], 1,
        (const void * []) {
            &(VkUpdateBuffers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_BUFFERS,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .arrayIndex = 0,
                .count = 1,
                .pBufferViews = (VkBufferViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view[2]
                    },
                },
            },
        });

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .renderArea = {{0, 0}, {t_width, t_height}},
        .pColorFormats = (VkFormat[]) { VK_FORMAT_R8G8B8A8_UNORM },
        .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_CLEAR },
        .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE },
        .pColorLoadClearValues = (VkClearColor[]) {
            {
                .color = { .floatColor = { 1.0, 0.0, 0.0, 1.0 } },
                .useRawValue = false,
            },
        });

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBegin) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
        });
    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, 3 * 4 * sizeof(float) });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 1,
                            &set[0], 0, NULL);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 1, 1,
                            &set[1], 0, NULL);
    vkCmdDraw(t_cmd_buffer, /*firstVertex*/ 0, /*vertexCount*/ 3,
              /*firstInstance*/ 0, /*instanceCount*/ 1);
    vkCmdEndRenderPass(t_cmd_buffer, pass);
    qoEndCommandBuffer(t_cmd_buffer);
    vkQueueSubmit(t_queue, 1, &t_cmd_buffer, 0);
}

cru_define_test {
    .name = "func.first",
    .start = test,
};
