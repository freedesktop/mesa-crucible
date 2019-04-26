// Copyright 2019 Intel Corporation
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
#include "util/misc.h"
#include <time.h>
#include <stdio.h>

#define DESCRIPTOR_SETS_PER_POOL 4096
#define CREATE_RESET_CYCLES 4096

static uint64_t
gettime_ns()
{
    struct timespec current;
    int ret = clock_gettime(CLOCK_MONOTONIC, &current);
    t_assert (ret >= 0);
    if (ret < 0)
        return 0;

    return (uint64_t) current.tv_sec * 1000000000ULL + current.tv_nsec;
}

static void
test()
{
    VkDescriptorSetLayout layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 2,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 2,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL,
            },
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkDescriptorPool pool;
    vkCreateDescriptorPool(t_device,
        &(VkDescriptorPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = 0,
            .maxSets = DESCRIPTOR_SETS_PER_POOL,
            .poolSizeCount = 2,
            .pPoolSizes = (VkDescriptorPoolSize[]) {
                {
                    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 2 * DESCRIPTOR_SETS_PER_POOL,
                },
                {
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1 * DESCRIPTOR_SETS_PER_POOL,
                },
            }
        }, NULL, &pool);
    t_cleanup_push_vk_descriptor_pool(t_device, pool);

    VkDescriptorSetLayout layouts[DESCRIPTOR_SETS_PER_POOL];
    for (unsigned i = 0; i < DESCRIPTOR_SETS_PER_POOL; i++)
        layouts[i] = layout;

    uint64_t start = gettime_ns();

    VkDescriptorSet sets[DESCRIPTOR_SETS_PER_POOL];
    for (unsigned i = 0; i < CREATE_RESET_CYCLES; i++) {
        vkAllocateDescriptorSets(t_device,
            &(VkDescriptorSetAllocateInfo) {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = pool,
                .descriptorSetCount = DESCRIPTOR_SETS_PER_POOL,
                .pSetLayouts = layouts,
            }, sets);
        vkResetDescriptorPool(t_device, pool, 0);
    }

    uint64_t end = gettime_ns();

    logi("Time: %f seconds\n", (double)(end - start) / 1000000000.0);
}

test_define {
    .name = "bench.descriptor-pool-reset",
    .start = test,
    .no_image = true,
};
