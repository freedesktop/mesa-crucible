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

#include "compute-num-workgroups-spirv.h"

typedef struct {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkDescriptorSet set;
    const uint32_t *sizes;
    VkBuffer ssbo_buf;
    VkDeviceMemory ssbo;
    uint32_t ssbo_size;
} CTX;

static void
common_init(CTX *ctx)
{
    VkDescriptorSetLayout set_layout;

    set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 2,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                    .pImmutableSamplers = NULL,
                },
                /* FIXME: Why do I need this for the test to pass? */
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    ctx->pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkShaderModule shader = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,

        layout(set = 0, binding = 0, std140) buffer Storage {
           uvec3 uv3a[];
        } ssbo;

        layout (local_size_x = 64) in;

        void main()
        {
            ssbo.uv3a[gl_LocalInvocationID.x] = gl_NumWorkGroups;
        }
    );

    vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .stage = {
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = shader,
                .pName = "main",
            },
            .flags = 0,
            .layout = ctx->pipeline_layout
        }, NULL, &ctx->pipeline);

    ctx->set = qoAllocateDescriptorSet(t_device, .pSetLayouts = &set_layout);

    ctx->ssbo_buf = qoCreateBuffer(t_device, .size = ctx->ssbo_size);
    ctx->ssbo = qoAllocBufferMemory(t_device, ctx->ssbo_buf,
        .memoryTypeIndex = t_mem_type_index_for_mmap);
    qoBindBufferMemory(t_device, ctx->ssbo_buf, ctx->ssbo, 0);

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 1,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = ctx->set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = ctx->ssbo_buf,
                    .offset = 0,
                    .range = ctx->ssbo_size,
                },
            },
        }, 0, NULL);
}

static void
dispatch_and_wait(CTX *ctx)
{
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      ctx->pipeline);

    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            ctx->pipeline_layout, 0, 1,
                            &ctx->set, 0, NULL);

    vkCmdDispatch(t_cmd_buffer, ctx->sizes[0], ctx->sizes[1], ctx->sizes[2]);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    vkQueueWaitIdle(t_queue);

    vkResetCommandBuffer(t_cmd_buffer, 0);
}

static void
verify_ssbo(CTX *ctx)
{
    uint32_t *map_out = qoMapMemory(t_device, ctx->ssbo, 0, ctx->ssbo_size, 0);
    for (unsigned i = 0; i < 64; i++) {
        for (unsigned j = 0; j < 3; j++) {
            uint32_t found = map_out[3 * i + j];
            t_assertf(found == ctx->sizes[j],
                      "buffer mismatch at (%d, %d): found %u, "
                      "expected %u", i, j, found, ctx->sizes[j]);
        }
    }
    vkUnmapMemory(t_device, ctx->ssbo);
}

static uint32_t scenarios[][3] = {
    { 1, 2, 3 },
    { 4, 5, 6 },
    { 11, 22, 33 },
};

static void
basic(void)
{
    CTX ctx;

    ctx.ssbo_size = 64 * 3 * sizeof(uint32_t);
    common_init(&ctx);

    for (unsigned s = 0; s < ARRAY_LENGTH(scenarios); s++) {
        const uint32_t *expected = scenarios[s];
        ctx.sizes = expected;

        dispatch_and_wait(&ctx);

        verify_ssbo(&ctx);
    }
    t_pass();
}

test_define {
    .name = "func.compute.num-workgroups.basic",
    .start = basic,
    .no_image = true,
};

static void
build_indirect_cmd_buffer(CTX *ctx)
{
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      ctx->pipeline);

    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            ctx->pipeline_layout, 0, 1,
                            &ctx->set, 0, NULL);

    vkCmdDispatchIndirect(t_cmd_buffer, ctx->ssbo_buf,
                          3 * 64 * sizeof(uint32_t));

    qoEndCommandBuffer(t_cmd_buffer);
}

static void
indirect_dispatch_and_wait(CTX *ctx)
{
    uint32_t *ssbo = qoMapMemory(t_device, ctx->ssbo, 0, ctx->ssbo_size, 0);
    for (unsigned i = 0; i < 3; i++)
        ssbo[3 * 64 + i] = ctx->sizes[i];
    vkUnmapMemory(t_device, ctx->ssbo);

    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    vkQueueWaitIdle(t_queue);
}

static void
indirect(void)
{
    CTX ctx;

    ctx.ssbo_size = 65 * 3 * sizeof(uint32_t);
    common_init(&ctx);

    build_indirect_cmd_buffer(&ctx);

    for (unsigned s = 0; s < ARRAY_LENGTH(scenarios); s++) {
        const uint32_t *expected = scenarios[s];
        ctx.sizes = expected;

        indirect_dispatch_and_wait(&ctx);

        verify_ssbo(&ctx);
    }
    t_pass();
}

test_define {
    .name = "func.compute.num-workgroups.indirect",
    .start = indirect,
    .no_image = true,
};
