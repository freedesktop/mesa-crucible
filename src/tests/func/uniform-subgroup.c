// Copyright 2020 Valve Corporation
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

// ACO has optimized implementations for several reductions for when the source
// is uniform.

#include "uniform-subgroup-spirv.h"

static VkDeviceMemory
common_init(VkShaderModule cs, const uint32_t ssbo_size,
            unsigned spec_count, const uint32_t *spec)
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

    VkSpecializationMapEntry entries[spec_count];
    for (unsigned i = 0; i < spec_count; i++)
        entries[i] = (VkSpecializationMapEntry){i, i * 4, 4};
    VkSpecializationInfo spec_info = {
       spec_count, entries, spec_count * 4, spec
    };

    VkPipeline pipeline;
    vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .stage = {
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = cs,
                .pName = "main",
                .pSpecializationInfo = &spec_info,
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

typedef struct test_params {
    unsigned reduce;
    unsigned bit_size;
    unsigned func;
} test_params_t;

static void
test(void)
{
    const test_params_t *params = t_user_data;
    bool is_float = params->func >= 8;

    if (t_physical_dev_props->apiVersion < VK_API_VERSION_1_1)
        t_skipf("Vulkan 1.1 required");

    if (params->bit_size == 8 || (params->bit_size == 16 && is_float))
        t_require_ext("VK_KHR_shader_float16_int8");
    if (params->bit_size != 32)
        t_require_ext("VK_KHR_shader_subgroup_extended_types");

    VkPhysicalDeviceSubgroupProperties subgroup_props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES,
    };
    VkPhysicalDeviceProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &subgroup_props,
    };
    vkGetPhysicalDeviceProperties2(t_physical_dev, &props);

    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR fp16_int8_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR
    };
    VkPhysicalDeviceFeatures2 features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    };

    if (!(subgroup_props.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT))
        t_skipf("VK_SUBGROUP_FEATURE_ARITHMETIC_BIT unsupported");

    if (params->bit_size == 8 || (params->bit_size == 16 && is_float))
        features.pNext = &fp16_int8_features;
    vkGetPhysicalDeviceFeatures2(t_physical_dev, &features);

    if (params->bit_size == 8 && !fp16_int8_features.shaderInt8)
        t_skipf("missing shaderInt8");
    if (params->bit_size == 16 && !is_float && !features.features.shaderInt16)
        t_skipf("missing shaderInt16");
    if (params->bit_size == 64 && !is_float && !features.features.shaderInt64)
        t_skipf("missing shaderInt64");

    if (params->bit_size == 16 && is_float && !fp16_int8_features.shaderFloat16)
        t_skipf("missing shaderFloat16");
    if (params->bit_size == 64 && is_float && !features.features.shaderFloat64)
        t_skipf("missing shaderFloat64");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_TARGET_ENV vulkan1.1
        QO_EXTENSION GL_EXT_shader_explicit_arithmetic_types_int8: enable
        QO_EXTENSION GL_EXT_shader_explicit_arithmetic_types_int16: enable
        QO_EXTENSION GL_EXT_shader_explicit_arithmetic_types_int64: enable
        QO_EXTENSION GL_EXT_shader_explicit_arithmetic_types_float16: enable
        QO_EXTENSION GL_KHR_shader_subgroup_arithmetic: enable
        QO_EXTENSION GL_EXT_shader_subgroup_extended_types_int8: enable
        QO_EXTENSION GL_EXT_shader_subgroup_extended_types_int16: enable
        QO_EXTENSION GL_EXT_shader_subgroup_extended_types_int64: enable
        QO_EXTENSION GL_EXT_shader_subgroup_extended_types_float16: enable

        layout (constant_id = 0) const int REDUCE_TYPE = 0;
        layout (constant_id = 1) const int BIT_SIZE = 0;
        layout (constant_id = 2) const int FUNC = 0;

        layout(set = 0, binding = 0, std430) buffer Storage {
            uint v[];
        } ssbo;

        layout (local_size_x = 64) in;

        //Convince the compiler that a uniform value is divergent.
        uint make_divergent(uint a)
        {
            //gl_WorkGroupID.x is always 0
            uint zero = uint(gl_WorkGroupID.x > gl_LocalInvocationIndex);
            return a + zero;
        }

        // Use floating point operations to convince the compiler to place a
        // uniform value into a register (VGPR) typically used for divergent
        // values.
        uint make_vgpr(uint a)
        {
            //0x3f800000 = 1.0
            //0x7fc00000 = NaN
            return floatBitsToUint(max(uintBitsToFloat(0x3f800000 | a),
                                       uintBitsToFloat(gl_WorkGroupID.x | 0x7fc00000))) & 0xffff;
        }

        QO_DEFINE _test(func, bit_size, val, val_div) \
        if (BIT_SIZE == bit_size && REDUCE_TYPE == 0) \
            fail = subgroup##func(val) != subgroup##func(val_div) || fail; \
        if (BIT_SIZE == bit_size && REDUCE_TYPE == 1) \
            fail = subgroupInclusive##func(val) != subgroupInclusive##func(val_div) || fail; \
        if (BIT_SIZE == bit_size && REDUCE_TYPE == 2) \
            fail = subgroupExclusive##func(val) != subgroupExclusive##func(val_div) || fail;

        QO_DEFINE _testi(func_idx, func, val) \
        if (FUNC == func_idx) { \
            _test(func, 8, int8_t(val), int8_t(make_divergent(val))) \
            _test(func, 16, int16_t(val), int16_t(make_divergent(val))) \
            _test(func, 32, int(val), int(make_divergent(val))) \
            _test(func, 64, int64_t(val), int64_t(make_divergent(val))) \
        }

        QO_DEFINE _testu(func_idx, func, val) \
        if (FUNC == func_idx) { \
            _test(func, 8, uint8_t(val), uint8_t(make_divergent(val))) \
            _test(func, 16, uint16_t(val), uint16_t(make_divergent(val))) \
            _test(func, 32, uint(val), uint(make_divergent(val))) \
            _test(func, 64, uint64_t(val), uint64_t(make_divergent(val))) \
        }

        QO_DEFINE _testf(func_idx, func, val) \
        if (FUNC == func_idx) { \
            _test(func, 16, float16_t(val), float16_t(make_divergent(val))) \
            _test(func, 32, float(val), float(make_divergent(val))) \
            _test(func, 64, double(val), double(make_divergent(val))) \
        }

        QO_DEFINE test(val) \
       _testi(0, Add, val) \
       _testi(1, Min, val) \
       _testi(2, Max, val) \
       _testu(3, Min, val) \
       _testu(4, Max, val) \
       _testi(5, And, val) \
       _testi(6, Or, val) \
       _testi(7, Xor, val) \
       _testf(8, Add, val) \
       _testf(9, Min, val) \
       _testf(10, Max, val)

        void main()
        {
            bool fail = false;
            for (uint i = 0; i < 4; i++) {
                switch (i) {
                case 0:
                    break;
                case 1:
                    if (gl_SubgroupInvocationID == 0)
                        continue;
                    break;
                case 2:
                    if ((gl_SubgroupInvocationID & 0x1) == 0)
                        continue;
                    break;
                case 3:
                    if ((gl_SubgroupInvocationID & 0x1) != 0)
                        continue;
                    break;
                }

                test(0);
                test(1);
                test(45);
                test(make_vgpr(45));
            }
            ssbo.v[gl_LocalInvocationIndex] = fail ? 1 : 0;
        }
    );

    uint32_t spec[3] = {params->reduce, params->bit_size, params->func};
    VkDeviceMemory mem = common_init(cs, 64 * 16, 3, spec);
    dispatch_and_wait(1, 1, 1);

    uint32_t *map = qoMapMemory(t_device, mem, 0, 64 * 16, 0);
    for (unsigned i = 0; i < 64; i++)
        t_assertf(map[i * 4] == 0, "invocation %u failed", i);
    t_pass();
}

#define _TEST_DEF(bit_size, reduce, reduce_name, func, func_name) \
test_define { \
    .name = "func.uniform-subgroup." #reduce_name "." #func_name #bit_size, \
    .start = test, \
    .user_data = &(test_params_t) {reduce, bit_size, func}, \
    .no_image = true, \
};

#define _TEST(bit_size, func, func_name) \
_TEST_DEF(bit_size, 0, reduce, func, func_name) \
_TEST_DEF(bit_size, 1, inclusive, func, func_name) \
_TEST_DEF(bit_size, 2, exclusive, func, func_name)

#define TEST_INT(func, func_name) \
_TEST(8, func, func_name) \
_TEST(16, func, func_name) \
_TEST(32, func, func_name) \
_TEST(64, func, func_name)

#define TEST_FLOAT(func, func_name) \
_TEST(16, func, func_name) \
_TEST(32, func, func_name) \
_TEST(64, func, func_name)

TEST_INT(0, iadd)
TEST_INT(1, imin)
TEST_INT(2, imax)
TEST_INT(3, umin)
TEST_INT(4, umax)
TEST_INT(5, and)
TEST_INT(6, or)
TEST_INT(7, xor)
TEST_FLOAT(8, fadd)
TEST_FLOAT(9, fmin)
TEST_FLOAT(10, fmax)
