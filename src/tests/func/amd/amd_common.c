// Copyright 2018 Google LLC
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
#include "amd_common.h"

VkDeviceMemory
common_init(VkShaderModule cs, const uint32_t ssbo_size)
{
    VkDescriptorSetLayout set_layout;

    set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 1,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout,
        .pushConstantRangeCount = 0);

    VkPipeline pipeline;
    vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .stage = {
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = cs,
                .pName = "main",
            },
            .flags = 0,
            .layout = pipeline_layout
        }, NULL, &pipeline);

    VkDescriptorSet set =
        qoAllocateDescriptorSet(t_device,
                                .descriptorPool = t_descriptor_pool,
                                .pSetLayouts = &set_layout);

    VkBuffer buffer_out = qoCreateBuffer(t_device, .size = ssbo_size);
    VkDeviceMemory mem_out = qoAllocBufferMemory(t_device, buffer_out,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, buffer_out, mem_out, 0);

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 1,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = buffer_out,
                    .offset = 0,
                    .range = ssbo_size,
                },
            },
        }, 0, NULL);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline_layout, 0, 1,
                            &set, 0, NULL);

    return mem_out;
}

void
dispatch_and_wait(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch(t_cmd_buffer, x, y, z);
    VkMemoryBarrier memoryBarrier = {
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        NULL,
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_HOST_READ_BIT
    };
    vkCmdPipelineBarrier(t_cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memoryBarrier, 0, NULL, 0, NULL);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);
}
