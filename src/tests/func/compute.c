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

#include "compute-spirv.h"

static void
test(void)
{
    VkDescriptorSetLayout set_layout;

    set_layout = qoCreateDescriptorSetLayout(t_device,
            .count = 2,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .arraySize = 1,
                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .arraySize = 1,
                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .descriptorSetCount = 1,
        .pSetLayouts = &set_layout);

    VkShader cs = qoCreateShaderGLSL(
        t_device, COMPUTE,

        layout (local_size_x = 8, local_size_y = 2, local_size_z = 2) in;

        void main()
        {
        }
    );

    VkPipeline pipeline;
    vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .cs = {
                .stage = VK_SHADER_STAGE_COMPUTE,
                .shader = cs,
            },
            .flags = 0,
            .layout = pipeline_layout
        }, &pipeline);

    VkDescriptorSet set;
    qoAllocDescriptorSets(t_device, QO_NULL_DESCRIPTOR_POOL,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          1, &set_layout, &set);

    VkBuffer buffer = qoCreateBuffer(t_device, .size = 1024);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .memoryTypeIndex = t_mem_type_index_for_device_access);

    qoBindBufferMemory(t_device, buffer, mem, 0);

    VkBufferView buffer_view;
    buffer_view = qoCreateBufferView(t_device,
        .buffer = buffer,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = 0, .range = 64);

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
        .maxLod = 5,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 2,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .destSet = set,
                .destBinding = 0,
                .destArrayElement = 0,
                .count = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pDescriptors = (VkDescriptorInfo[]) {
                    { .bufferView = buffer_view },
                },
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .destSet = set,
                .destBinding = 1,
                .destArrayElement = 0,
                .count = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .pDescriptors = (VkDescriptorInfo[]) {
                    { .sampler = sampler, },
                },
            },
        }, 0, NULL);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline_layout, 0, 1,
                            &set, 0, NULL);

    vkCmdDispatch(t_cmd_buffer, 8, 2, 2);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

test_define {
    .name = "func.compute",
    .start = test,
    .no_image = true,
};
