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

#include "compute-local-id-spirv.h"

static VkDeviceMemory
common_init(VkShaderModule cs, const uint32_t ssbo_size, VkPipelineLayout *p_layout)
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

    VkPushConstantRange constants = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = 4,
    };
    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &constants);

    *p_layout = pipeline_layout;
    VkPipeline pipeline;
    vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
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

static void
dispatch_and_wait(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch(t_cmd_buffer, x, y, z);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);
}

static void
basic(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,

        layout(set = 0, binding = 0, std430) buffer Storage {
           uint ua[];
        } ssbo;

        layout (local_size_x = 64) in;

        void main()
        {
            ssbo.ua[gl_LocalInvocationID.x] = gl_LocalInvocationID.x;
        }
    );

    const uint32_t ssbo_size = 64 * sizeof(uint32_t);
    VkPipelineLayout p_layout;
    VkDeviceMemory mem_out = common_init(cs, ssbo_size, &p_layout);

    dispatch_and_wait(1, 1, 1);

    uint32_t *map_out = qoMapMemory(t_device, mem_out, 0, ssbo_size, 0);
    for (unsigned i = 0; i < 64; i++) {
        t_assertf(map_out[i] == i,
                  "buffer mismatch at uint %d: found %u, "
                  "expected %u", i, map_out[i], i);
    }
    t_pass();
}

test_define {
    .name = "func.compute.local-id.basic",
    .start = basic,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};

static void
push_constant(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,

        layout(push_constant, std430) uniform Push {
            uint add;
        } pc;

        layout(set = 0, binding = 0, std430) buffer Storage {
           uint ua[];
        } ssbo;

        layout (local_size_x = 64) in;

        void main()
        {
            ssbo.ua[gl_LocalInvocationID.x] = pc.add + gl_LocalInvocationID.x;
        }
    );

    const uint32_t ssbo_size = 64 * sizeof(uint32_t);
    VkPipelineLayout p_layout;
    VkDeviceMemory mem_out = common_init(cs, ssbo_size, &p_layout);

    uint32_t add = 42;
    vkCmdPushConstants(t_cmd_buffer, p_layout,
                       VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(add), &add);

    dispatch_and_wait(1, 1, 1);

    uint32_t *map_out = qoMapMemory(t_device, mem_out, 0, ssbo_size, 0);
    for (unsigned i = 0; i < 64; i++) {
        t_assertf(map_out[i] == add + i,
                  "buffer mismatch at uint %d: found %u, "
                  "expected %u", i, map_out[i], add + i);
    }
    t_pass();
}

test_define {
    .name = "func.compute.local-id.push-constant",
    .start = push_constant,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};

/* Based on the piglit test:
 * spec/arb_compute_shader/execution/basic-local-id-atomic.shader_test
 */
static void
local_ids(void)
{
    if (t_physical_dev_props->limits.maxComputeWorkGroupInvocations < 512) {
        t_skipf("test requires a workgroup size of 512, but physical device "
                "supports only %d",
                t_physical_dev_props->limits.maxComputeWorkGroupInvocations);
    }

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,

        layout(set = 0, binding = 0, std430) buffer Storage {
           uint ua[];
        } ssbo;

        layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

        void main()
        {
            uint x = gl_LocalInvocationID.x;
            uint y = gl_LocalInvocationID.y;
            uint z = gl_LocalInvocationID.z;

            if (((x & y) & z) == 0u)
                atomicAdd(ssbo.ua[0], 1);
            if (((x | y) | z) == 7u)
                atomicAdd(ssbo.ua[1], 1);
            if (x == y && y == z)
                atomicAdd(ssbo.ua[2], 1);
            if (x != y && y != z && x != z)
                atomicAdd(ssbo.ua[3], 1);
            if (((x & y) & z) == 2u)
                atomicAdd(ssbo.ua[4], 1);
            if (((x | y) | z) == 5u)
                atomicAdd(ssbo.ua[5], 1);
            if (x < 4u && y < 4u && z < 4u)
                atomicAdd(ssbo.ua[6], 1);
            if (x >= 4u || y >= 4u || z >= 4u)
                atomicAdd(ssbo.ua[7], 1);
        }
    );

    const uint32_t ssbo_size = 8 * sizeof(uint32_t);
    VkPipelineLayout p_layout;
    VkDeviceMemory mem_out = common_init(cs, ssbo_size, &p_layout);

    dispatch_and_wait(1, 1, 1);

    const uint32_t expected[] = {
        343,
        343,
        8,
        336,
        49,
        49,
        64,
        448,
    };

    uint32_t *map_out = qoMapMemory(t_device, mem_out, 0, ssbo_size, 0);
    for (unsigned i = 0; i < ARRAY_LENGTH(expected); i++) {
        t_assertf(map_out[i] == expected[i],
                  "buffer mismatch at uint %d: found %u, "
                  "expected %u", i, map_out[i], expected[i]);
    }
    t_pass();
}

test_define {
    .name = "func.compute.local-id.local-ids",
    .start = local_ids,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};
