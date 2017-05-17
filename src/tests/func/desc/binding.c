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

// \file binding.c
// \brief Test that binding a DescriptorSet with undefined descriptors works without
//        crashing the driver.
//
// Vulkan Specification 13.2.3. Allocation of Descriptor Sets:
//     When a descriptor set is allocated, the initial state is largely
//     uninitialized and all descriptors are undefined. However, the descriptor
//     set can be bound in a command buffer without causing errors or
//     exceptions. All entries that are statically used by a pipeline in a
//     drawing or dispatching command must have been populated before the
//     descriptor set is bound for use by that command. Entries that are not
//     statically used by a pipeline can have uninitialized descriptors or
//     descriptors of resources that have been destroyed, and executing a draw
//     or dispatch with such a descriptor set bound does not cause undefined
//     behavior. This means applications need not populate unused entries with
//     dummy descriptors.

#include "tapi/t.h"

enum bindings_defined {
  BD_NONE,
  BD_SOME,
  BD_ALL,
};

struct params {
    enum bindings_defined num_def;
};

static void
test(void)
{
    const struct params *params = t_user_data;

    /* Create a uniform buffer with memory-backing */
    const uint32_t buffer_size = 4096;
    VkBuffer buffer = qoCreateBuffer(t_device,
                                     .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                     .size = buffer_size);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    qoBindBufferMemory(t_device, buffer, mem, 0);

    /* Allocate a descriptor set consisting of two bindings */
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 2,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkDescriptorSet set = qoAllocateDescriptorSet(t_device,
                                .descriptorPool = t_descriptor_pool,
                                .pSetLayouts = &set_layout);

    /* Update some number of bindings in the set, leaving the remaining
     * (if any) undefined.
     */
    switch(params->num_def) {
    case BD_ALL:
        vkUpdateDescriptorSets(t_device,
            /*writeCount*/ 2,
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .pBufferInfo = &(VkDescriptorBufferInfo) {
                        .buffer = buffer,
                        .offset = 0,
                        .range = buffer_size,
                    },
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .pBufferInfo = &(VkDescriptorBufferInfo) {
                        .buffer = buffer,
                        .offset = 0,
                        .range = buffer_size,
                    },
                },
            }, 0, NULL);
        break;
    case BD_SOME:
        vkUpdateDescriptorSets(t_device,
            /*writeCount*/ 1,
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .pBufferInfo = &(VkDescriptorBufferInfo) {
                        .buffer = buffer,
                        .offset = 0,
                        .range = buffer_size,
                    },
                },
            }, 0, NULL);
        break;
    case BD_NONE:
        break;
    }

    /* Test that the driver doesn't fail due to binding the descriptor set. */
    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    uint32_t dynamic_offsets[2] = { 0 };
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 2, dynamic_offsets);

    qoEndCommandBuffer(t_cmd_buffer);
}

test_define {
    .name = "func.desc.binding.defined.none",
    .start = test,
    .user_data = &(struct params) {
        .num_def = BD_NONE,
    },
    .no_image = true,
};

test_define {
    .name = "func.desc.binding.defined.some",
    .start = test,
    .user_data = &(struct params) {
        .num_def = BD_SOME,
    },
    .no_image = true,
};

test_define {
    .name = "func.desc.binding.defined.all",
    .start = test,
    .user_data = &(struct params) {
        .num_def = BD_ALL,
    },
    .no_image = true,
};
