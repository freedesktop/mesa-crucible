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

#include "src/tests/func/bind-points-spirv.h"

static void
test_bind_points(void)
{
    VkResult result;

    VkDescriptorSetLayout compute_set_layout;
    compute_set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 2,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL,
            },
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPipelineLayout compute_pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &compute_set_layout);

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(set = 0, binding = 0) uniform block1 {
            vec4 data[3];
        } u;
        layout(set = 0, binding = 1, std430) buffer block2 {
            vec4 data[];
        } ssbo;

        layout (local_size_x = 1) in;
        void main()
        {
            for (uint i = 0; i < 3; ++i)
                ssbo.data[i] = u.data[i];
        }
    );

    VkPipeline compute_pipeline;
    result = vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .stage = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = cs,
                .pName = "main",
            },
            .flags = 0,
            .layout = compute_pipeline_layout,
        }, NULL, &compute_pipeline);
    t_assert(result == VK_SUCCESS);

    VkDescriptorSetLayout graphics_set_layout;
    graphics_set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 2,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL,
            },
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPipelineLayout graphics_pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &graphics_set_layout);

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        void main()
        {
            vec2 coords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
            gl_Position = vec4(coords * vec2(2, -2) + vec2(-1, 1), 0, 1);
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(set = 0, binding = 0) uniform sampler samp;
        layout(set = 0, binding = 1) uniform texture2D tex;

        void main()
        {
            f_color = texture(sampler2D(tex, samp), vec2(0.5, 0.5));
        }
    );

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_info,
            .layout = graphics_pipeline_layout,
            .renderPass = t_render_pass,
            .subpass = 0,
        }});

    VkDescriptorSet set[2];
    result = vkAllocateDescriptorSets(t_device,
        &(VkDescriptorSetAllocateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = t_descriptor_pool,
            .descriptorSetCount = 2,
            .pSetLayouts = (VkDescriptorSetLayout[]) {
                compute_set_layout,
                graphics_set_layout,
            },
        }, set);
    t_assert(result == VK_SUCCESS);

    static const float uniform_data[12] = {
        0.0, 0.2, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.5, 0.5
    };

    VkBuffer uniform_buffer = qoCreateBuffer(t_device,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .size = sizeof(uniform_data));
    VkDeviceMemory uniform_mem = qoAllocBufferMemory(t_device, uniform_buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, uniform_buffer,
                       uniform_mem, /*offset*/ 0);
    memcpy(qoMapMemory(t_device, uniform_mem, /*offset*/ 0,
                       sizeof(uniform_data), /*flags*/ 0),
           uniform_data,
           sizeof(uniform_data));

    VkBuffer ssbo = qoCreateBuffer(t_device,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        .size = sizeof(uniform_data));
    VkDeviceMemory ssbo_mem = qoAllocBufferMemory(t_device, ssbo,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, ssbo, ssbo_mem, 0);

    VkImage texture = qoCreateImage(t_device,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .extent = {
            .width = 16,
            .height = 16,
            .depth = 1,
        });
    VkMemoryRequirements texture_reqs =
        qoGetImageMemoryRequirements(t_device, texture);
    VkDeviceMemory texture_mem = qoAllocMemoryFromRequirements(t_device,
        &texture_reqs, .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    qoBindImageMemory(t_device, texture, texture_mem, /*offset*/ 0);

    VkImageView tex_view = qoCreateImageView(t_device,
        .image = texture,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM);

    VkSampler sampler = qoCreateSampler(t_device,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0,
        .maxAnisotropy = 1,
        .compareOp = VK_COMPARE_OP_GREATER,
        .minLod = 0,
        .maxLod = 0,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);

    vkUpdateDescriptorSets(t_device,
        4, /* writeCount */
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set[0],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = uniform_buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE,
                },
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set[0],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = ssbo,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE,
                },
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set[1],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .pImageInfo = (VkDescriptorImageInfo[]) {
                    {
                        .sampler = sampler,
                    }
                },
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set[1],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .pImageInfo = (VkDescriptorImageInfo[]) {
                    {
                        .imageView = tex_view,
                        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                    }
                },
            },
        }, 0, NULL);

    vkCmdPipelineBarrier(t_cmd_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, NULL, 0, NULL, 1,
        &(VkImageMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = texture,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        });

    vkCmdClearColorImage(t_cmd_buffer,
        texture,
        VK_IMAGE_LAYOUT_GENERAL,
        &(VkClearColorValue) { .float32 = { 0.0, 1.0, 0.0, 1.0 }, },
        1,
        &(VkImageSubresourceRange) {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        });

    vkCmdPipelineBarrier(t_cmd_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1,
        &(VkImageMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .image = texture,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      compute_pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            compute_pipeline_layout, 0, 1,
                            &set[0], 0, NULL);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphics_pipeline_layout, 0, 1,
                            &set[1], 0, NULL);

    vkCmdDispatch(t_cmd_buffer, 1, 1, 1);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = &(VkClearValue) {
                .color = { .float32 = { 0.0, 0.0, 0.0, 1.0 } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);
    vkCmdEndRenderPass(t_cmd_buffer);

    vkCmdPipelineBarrier(t_cmd_buffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        0, 0, NULL, 1,
        &(VkBufferMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
            .buffer = ssbo,
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        }, 0, NULL);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    float *ssbo_data = qoMapMemory(t_device, ssbo_mem, 0, VK_WHOLE_SIZE, 0);
    for (unsigned i = 0; i < sizeof(uniform_data) / sizeof(*uniform_data); i++) {
        t_assertf(ssbo_data[i] == uniform_data[i],
                  "buffer mismatch at %u: found %.8e, expected %.8e",
                  i, ssbo_data[i], uniform_data[i]);
    }

    vkDestroyPipeline(t_device, compute_pipeline, NULL);
}

test_define {
    .name = "func.bind-points",
    .start = test_bind_points,
    .image_filename = "32x32-green.ref.png",
};
