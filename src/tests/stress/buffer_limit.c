// Copyright 2016 Intel Corporation
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

// \file buffer_limit.c
// \brief Update a buffer descriptor to have the maximum range possible.

#include "tapi/t.h"

struct params {
    VkDescriptorType descriptor_type;
};


static void
test_max_buffer()
{
    const struct params *params = t_user_data;

    const uint32_t buffer_size = (params->descriptor_type ==
                                  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ?
			t_physical_dev_props->limits.maxUniformBufferRange :
			t_physical_dev_props->limits.maxStorageBufferRange;

    /* Create a uniform buffer with-out memory-backing */
    VkBuffer buffer = qoCreateBuffer(t_device,
                                     .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                     .size = buffer_size);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer);
    qoBindBufferMemory(t_device, buffer, mem, 0);
    /* Allocate a descriptor set consisting of one binding */
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 1,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = params->descriptor_type,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkDescriptorSet set = qoAllocateDescriptorSet(t_device,
                                .descriptorPool = t_descriptor_pool,
                                .pSetLayouts = &set_layout);

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 1,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = params->descriptor_type,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = buffer,
                    .offset = 0,
                    .range = buffer_size,
                },
            },
        }, 0, NULL);

    qoEndCommandBuffer(t_cmd_buffer);
}

test_define {
    .name = "stress.limits.buffer-update.range.uniform",
    .start = test_max_buffer,
    .no_image = true,
    .user_data = &(struct params) {
        .descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    },
};

test_define {
    .name = "stress.limits.buffer-update.range.storage",
    .start = test_max_buffer,
    .no_image = true,
    .user_data = &(struct params) {
        .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    },
};
