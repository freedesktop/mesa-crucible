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

#include "util/simple_pipeline.h"
#include "tapi/t.h"
#include <stdio.h>

#include "src/tests/func/shader_ballot/amd_shader_ballot-spirv.h"

static void
basic(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    t_require_ext("VK_AMD_shader_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
    QO_EXTENSION GL_ARB_shader_ballot : enable
    QO_EXTENSION GL_AMD_shader_ballot : enable
        layout(location = 0) out vec4 f_color;

        uint bitCount64(uint64_t val) {
            uvec2 split = unpackUint2x32(val);
            return bitCount(split.x) + bitCount(split.y);
        }

        void main() {
            int sum = addInvocationsNonUniformAMD(1);

            uint count = bitCount64(ballotARB(true));
            if (sum != count) {
                f_color = vec4(1.0, float(sum) / 255., float(count) / 255., 1.0);
            } else {
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            }
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.amd-shader-ballot.basic",
    .start = basic,
    .image_filename = "32x32-green.ref.png",
};

static void
inclusive_scan_iadd(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    t_require_ext("VK_AMD_shader_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
    QO_EXTENSION GL_ARB_shader_ballot : enable
    QO_EXTENSION GL_AMD_shader_ballot : enable
        layout(location = 0) out vec4 f_color;

        uint bitCount64(uint64_t val) {
            uvec2 split = unpackUint2x32(val);
            return bitCount(split.x) + bitCount(split.y);
        }

        uint mbcnt() {
            uint64_t active_set = ballotARB(true);
            uint invocation = gl_SubGroupInvocationARB;
            uint64_t mask_le = invocation == 63 ? ~0ul : (1ul << (invocation + 1)) - 1;
            return bitCount64(active_set & mask_le);
        }

        void main() {
            int sum = addInvocationsInclusiveScanNonUniformAMD(1);

            int count = int(mbcnt());
            if (sum != count) {
                f_color = vec4(1.0, float(sum) / 255., 0.0, float(gl_SubGroupInvocationARB) / 255.);
            } else {
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            }
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.amd-shader-ballot.inclusive-scan-iadd",
    .start = inclusive_scan_iadd,
    .image_filename = "32x32-green.ref.png",
};

static VkDeviceMemory
common_init(VkShaderModule cs, const uint32_t ssbo_size)
{
    VkDescriptorSetLayout set_layout;

    set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 2,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
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

static void
dispatch_and_wait(uint32_t x, uint32_t y, uint32_t z)
{
    vkCmdDispatch(t_cmd_buffer, x, y, z);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);
}


static void
inclusive_scan_iadd_compute(void)
{
    t_require_ext("VK_AMD_shader_ballot");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
    QO_EXTENSION GL_AMD_shader_ballot : enable
        layout(set = 0, binding = 0, std430) buffer Storage {
           uint ua[];
        } ssbo;

        layout (local_size_x = 64) in;

        void main()
        {
            ssbo.ua[gl_GlobalInvocationID.x] = addInvocationsInclusiveScanNonUniformAMD(1);
        }
    );

    const uint32_t ssbo_size = 4 * 64 * sizeof(uint32_t);
    VkDeviceMemory mem_out = common_init(cs, ssbo_size);

    dispatch_and_wait(4, 1, 1);

    uint32_t *map_out = qoMapMemory(t_device, mem_out, 0, ssbo_size, 0);
    for (unsigned i = 0; i < 4 * 64; i++) {
        t_assertf(map_out[i] == (i % 64) + 1,
                  "buffer mismatch at uint %d: found %u, "
                  "expected %u", i, map_out[i], (i % 64) + 1);
    }
    t_pass();
}

test_define {
    .name = "func.amd-shader-ballot.inclusive-scan-iadd-compute",
    .start = inclusive_scan_iadd_compute,
    .no_image = true,
};

static void
ballot_if_else(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    t_require_ext("VK_AMD_shader_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_shader_ballot : enable
    QO_EXTENSION GL_AMD_shader_ballot : enable
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
        layout(location = 0) out vec4 f_color;

        uint bitCount64(uint64_t val) {
            uvec2 split = unpackUint2x32(val);
            return bitCount(split.x) + bitCount(split.y);
        }

        void main() {
            uint sum = 0, active_chans = 0;
            if (gl_SubGroupInvocationARB % 2u == 0u) {
                sum = addInvocationsNonUniformAMD(1u);
                active_chans = bitCount64(ballotARB(true));
            } else {
                sum = addInvocationsNonUniformAMD(1u);
                active_chans = bitCount64(ballotARB(true));
            }

            if (sum != active_chans) {
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
            } else {
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            }
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.amd-shader-ballot.if-else",
    .start = ballot_if_else,
    .image_filename = "32x32-green.ref.png",
};


