// Copyright 2019 Valve Corporation
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

#include "atomic-spirv.h"

#define GET_DEVICE_FUNCTION_PTR(name) \
    PFN_vk##name name = (PFN_vk##name)vkGetDeviceProcAddr(t_device, "vk"#name)

static void
test_common(VkShaderModule cs, bool int64, unsigned num_values, void *init, void *ref)
{
    GET_DEVICE_FUNCTION_PTR(GetBufferDeviceAddressEXT);

    /* create pipeline */
    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &(VkPushConstantRange) {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .offset = 0,
            .size = sizeof(VkDeviceAddress) * 2 + 4,
        });

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

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    /* create and map buffers */
    unsigned buffer_size = num_values * (int64 ? 8 : 4);
    VkBuffer buffer = qoCreateBuffer(t_device, .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT, .size = buffer_size);
    VkDeviceMemory memory = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *map = qoMapMemory(t_device, memory, 0, buffer_size, 0);
    memcpy(map, init, buffer_size);

    qoBindBufferMemory(t_device, buffer, memory, 0);

    /* initialize push constants with buffer pointers */
    VkDeviceAddress addr = GetBufferDeviceAddressEXT(t_device,
        &(VkBufferDeviceAddressInfoEXT) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT,
            .pNext = NULL,
            .buffer = buffer
        });

    vkCmdPushConstants(t_cmd_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(addr), &addr);
    vkCmdPushConstants(t_cmd_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(addr), sizeof(addr), &addr);

    /* dispatch */
    uint32_t dispatch_index = 0;
    vkCmdPushConstants(t_cmd_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(addr) * 2, 4, &dispatch_index);
    vkCmdDispatch(t_cmd_buffer, 1, 1, 1);

    vkCmdPipelineBarrier(t_cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
        &(VkMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
        }, 0, NULL, 0, NULL);

    dispatch_index = 1;
    vkCmdPushConstants(t_cmd_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(addr) * 2, 4, &dispatch_index);
    vkCmdDispatch(t_cmd_buffer, 1, 1, 1);

    vkCmdPipelineBarrier(t_cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_HOST_BIT, 0, 1,
        &(VkMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = NULL,
            .srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_HOST_READ_BIT
        }, 0, NULL, 0, NULL);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    /* check results */
    if (int64) {
        uint64_t *mapi = (uint64_t*)map;
        uint64_t *refi = (uint64_t*)ref;
        for (unsigned i = 0; i < num_values; i++)
            t_assertf(mapi[i] == refi[i], "buffer mismatch at uint64_t %u: found %lu, expected %lu", i, mapi[i], refi[i]);
    } else {
        uint32_t *mapi = (uint32_t*)map;
        uint32_t *refi = (uint32_t*)ref;
        for (unsigned i = 0; i < num_values; i++)
            t_assertf(mapi[i] == refi[i], "buffer mismatch at uint32_t %u: found %u, expected %u", i, mapi[i], refi[i]);
    }
    t_pass();
}

static void
test32()
{
    t_require_ext("VK_EXT_buffer_device_address");
    t_require_ext("VK_KHR_shader_atomic_int64");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_EXT_buffer_reference : require

        layout(local_size_x = 2) in;

        layout(std430, buffer_reference) buffer UnsignedStorage {
           uint arr[];
        };
        layout(std430, buffer_reference) buffer SignedStorage {
           int arr[];
        };
        layout(std430, push_constant) uniform PushConst {
           UnsignedStorage ui;
           SignedStorage si;
           uint dispatch_index;
        };

        shared uint return_test[2];

        uint check_return(uint actual, uint start, uint expected0, uint expected1) {
            return_test[gl_LocalInvocationIndex] = actual;
            barrier();
            if (return_test[0] != start && return_test[1] != start)
                return 1;
            int other_idx = return_test[0] == start ? 1 : 0;
            if (return_test[other_idx] != expected0 && return_test[other_idx] != expected1)
                return 1;
            return 0;
        }

        void main()
        {
           uint fail = 0;
           if (dispatch_index == 0) {
              uint add_val = gl_LocalInvocationIndex > 0 ? 20 : 22;
              uint and_val = gl_LocalInvocationIndex > 0 ? 106 : 170;
              uint umax_val = gl_LocalInvocationIndex > 0 ? 10 : 42;
              uint umin_val = gl_LocalInvocationIndex > 0 ? 60 : 42;
              int smax_val = gl_LocalInvocationIndex > 0 ? -20 : 42;
              int smin_val = gl_LocalInvocationIndex > 0 ? 60 : -42;
              uint or_val = gl_LocalInvocationIndex > 0 ? 8 : 2;
              uint xor_val = gl_LocalInvocationIndex > 0 ? 64 : 128;

              /* no return */
              atomicAdd(ui.arr[0], add_val);
              atomicAnd(ui.arr[1], and_val);
              atomicCompSwap(ui.arr[2], 1, 42);
              atomicCompSwap(ui.arr[3], 1, 20);
              atomicExchange(ui.arr[4], 42);
              atomicMax(ui.arr[5], umax_val);
              atomicMin(ui.arr[6], umin_val);
              atomicMax(si.arr[7], smax_val);
              atomicMin(si.arr[8], smin_val);
              atomicOr(ui.arr[9], or_val);
              atomicXor(ui.arr[10], xor_val);

              /* return */
              fail |= check_return(atomicAdd(ui.arr[11], add_val), 0, 20, 22) << 1;
              fail |= check_return(atomicAnd(ui.arr[12], and_val), 0xffffffff, 106, 170) << 2;
              if (atomicCompSwap(ui.arr[13], 1, 20) != 1)
                 atomicCompSwap(ui.arr[13], 20, 42);
              if (atomicExchange(ui.arr[14], 20) == 20)
                atomicExchange(ui.arr[14], 42);
              fail |= check_return(atomicMax(ui.arr[15], umax_val), 0, 10, 42) << 3;
              fail |= check_return(atomicMin(ui.arr[16], umin_val), 0xffffffff, 60, 42) << 4;
              fail |= check_return(atomicMax(si.arr[17], smax_val), 0, 10, 42) << 5;
              fail |= check_return(atomicMin(si.arr[18], smin_val), 0x7fffffff, 60, -42) << 6;
              fail |= check_return(atomicOr(ui.arr[19], or_val), 32, 40, 34) << 7;
              fail |= check_return(atomicXor(ui.arr[20], xor_val), 234, 170, 106) << 8;
           } else if (dispatch_index == 1) {
              uint umax_val = gl_LocalInvocationIndex > 0 ? 20 : 30;
              uint umin_val = gl_LocalInvocationIndex > 0 ? 80 : 90;
              int smax_val = gl_LocalInvocationIndex > 0 ? -20 : 30;
              int smin_val = gl_LocalInvocationIndex > 0 ? 80 : 90;

              /* no return */
              atomicMax(ui.arr[5], umax_val);
              atomicMin(ui.arr[6], umin_val);
              atomicMax(si.arr[7], smax_val);
              atomicMin(si.arr[8], smin_val);

              /* return */
              fail |= check_return(atomicMax(ui.arr[15], umax_val), 42, 42, 42) << 9;
              fail |= check_return(atomicMin(ui.arr[16], umin_val), 42, 42, 42) << 10;
              fail |= check_return(atomicMax(si.arr[17], smax_val), 42, 42, 42) << 11;
              fail |= check_return(atomicMin(si.arr[18], smin_val), -42, -42, -42) << 12;
           }

           ui.arr[21] |= 1 | fail;
        }
    );

    uint32_t init[] = {
        0, /* add, no return */
        UINT32_MAX, /* and, no return */
        1, /* compare+swap, first case, no return */
        42, /* compare+swap, second case, no return */
        0, /* exchange, no return */
        0, /* unsigned max, no return */
        UINT32_MAX, /* unsigned min, no return */
        0, /* signed max, no return */
        INT32_MAX, /* signed min, no return */
        32, /* or, no return */
        234, /* xor, no return */

        0, /* add, return */
        UINT32_MAX, /* and, return */
        1, /* compare+swap, return */
        0, /* exchange, return */
        0, /* unsigned max, return */
        UINT32_MAX, /* unsigned min, return */
        0, /* signed max, return */
        INT32_MAX, /* signed min, return */
        32, /* or, return */
        234, /* xor, return */

        0, /* return fail state */
    };
    uint32_t values[] = {
        /* no return */
        42,
        42,
        42,
        42,
        42,
        42,
        42,
        42,
        -42,
        42,
        42,

        /* return */
        42,
        42,
        42,
        42,
        42,
        42,
        42,
        -42,
        42,
        42,

        /* return fail state */
        1,
    };
    assert(sizeof(values) == sizeof(init));
    test_common(cs, false, sizeof(values)/sizeof(values[0]), init, values);
}

test_define {
    .name = "func.buffer_reference.atomic32",
    .start = test32,
    .image_filename = "32x32-green.ref.png",
};

static void
test64()
{
    t_require_ext("VK_EXT_buffer_device_address");
    t_require_ext("VK_KHR_shader_atomic_int64");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_EXT_buffer_reference : require
        QO_EXTENSION GL_ARB_gpu_shader_int64 : require
        QO_EXTENSION GL_EXT_shader_atomic_int64 : require

        QO_DEFINE V(val) (int64_t((val)) << 29)

        layout(local_size_x = 2) in;

        layout(std430, buffer_reference) buffer UnsignedStorage {
           uint64_t arr[];
        };
        layout(std430, buffer_reference) buffer SignedStorage {
           int64_t arr[];
        };
        layout(std430, push_constant) uniform PushConst {
           UnsignedStorage ui;
           SignedStorage si;
           uint dispatch_index;
        };

        shared uint64_t return_test[2];

        uint check_return(uint64_t actual, uint64_t start, uint64_t expected0, uint64_t expected1) {
            start = V(start);
            expected0 = V(expected0);
            expected1 = V(expected1);

            return_test[gl_LocalInvocationIndex] = actual;
            barrier();
            if (return_test[0] != start && return_test[1] != start)
                return 1;
            int other_idx = return_test[0] == start ? 1 : 0;
            if (return_test[other_idx] != expected0 && return_test[other_idx] != expected1)
                return 1;
            return 0;
        }

        void main()
        {
           uint fail = 0;
           if (dispatch_index == 0) {
              uint64_t add_val = V(gl_LocalInvocationIndex > 0 ? 20 : 22);
              uint64_t and_val = V(gl_LocalInvocationIndex > 0 ? 106 : 170);
              uint64_t umax_val = V(gl_LocalInvocationIndex > 0 ? 10 : 42);
              uint64_t umin_val = V(gl_LocalInvocationIndex > 0 ? 60 : 42);
              int64_t smax_val = V(gl_LocalInvocationIndex > 0 ? -20 : 42);
              int64_t smin_val = V(gl_LocalInvocationIndex > 0 ? 60 : -42);
              uint64_t or_val = V(gl_LocalInvocationIndex > 0 ? 8 : 2);
              uint64_t xor_val = V(gl_LocalInvocationIndex > 0 ? 64 : 128);

              /* no return */
              atomicAdd(ui.arr[0], add_val);
              atomicAnd(ui.arr[1], and_val);
              atomicCompSwap(ui.arr[2], V(1), V(42));
              atomicCompSwap(ui.arr[3], V(1), V(20));
              atomicExchange(ui.arr[4], V(42));
              atomicMax(ui.arr[5], umax_val);
              atomicMin(ui.arr[6], umin_val);
              atomicMax(si.arr[7], smax_val);
              atomicMin(si.arr[8], smin_val);
              atomicOr(ui.arr[9], or_val);
              atomicXor(ui.arr[10], xor_val);

              /* return */
              fail |= check_return(atomicAdd(ui.arr[11], add_val), 0, 20, 22) << 1;
              fail |= check_return(atomicAnd(ui.arr[12], and_val), 0xfffffffffffffffful, 106, 170) << 2;
              if (atomicCompSwap(ui.arr[13], V(1), V(20)) != 1)
                 atomicCompSwap(ui.arr[13], V(20), V(42));
              if (atomicExchange(ui.arr[14], V(20)) == V(20))
                atomicExchange(ui.arr[14], V(42));
              fail |= check_return(atomicMax(ui.arr[15], umax_val), 0, 10, 42) << 3;
              fail |= check_return(atomicMin(ui.arr[16], umin_val), 0xfffffffffffffffful, 60, 42) << 4;
              fail |= check_return(atomicMax(si.arr[17], smax_val), 0, 10, 42) << 5;
              fail |= check_return(atomicMin(si.arr[18], smin_val), 0x7ffffffffffffffful, 60, -42) << 6;
              fail |= check_return(atomicOr(ui.arr[19], or_val), 32, 40, 34) << 7;
              fail |= check_return(atomicXor(ui.arr[20], xor_val), 234, 170, 106) << 8;
           } else if (dispatch_index == 1) {
              uint64_t umax_val = V(gl_LocalInvocationIndex > 0 ? 20 : 30);
              uint64_t umin_val = V(gl_LocalInvocationIndex > 0 ? 80 : 90);
              int64_t smax_val = V(gl_LocalInvocationIndex > 0 ? -20 : 30);
              int64_t smin_val = V(gl_LocalInvocationIndex > 0 ? 80 : 90);

              /* no return */
              atomicMax(ui.arr[5], umax_val);
              atomicMin(ui.arr[6], umin_val);
              atomicMax(si.arr[7], smax_val);
              atomicMin(si.arr[8], smin_val);

              /* return */
              fail |= check_return(atomicMax(ui.arr[15], umax_val), 42, 42, 42) << 9;
              fail |= check_return(atomicMin(ui.arr[16], umin_val), 42, 42, 42) << 10;
              fail |= check_return(atomicMax(si.arr[17], smax_val), 42, 42, 42) << 11;
              fail |= check_return(atomicMin(si.arr[18], smin_val), -42, -42, -42) << 12;
           }

           ui.arr[21] |= 1 | fail;
        }
    );

    uint64_t init[] = {
        0, /* add, no return */
        UINT64_MAX, /* and, no return */
        1, /* compare+swap, first case, no return */
        42, /* compare+swap, second case, no return */
        0, /* exchange, no return */
        0, /* unsigned max, no return */
        UINT64_MAX, /* unsigned min, no return */
        0, /* signed max, no return */
        INT64_MAX, /* signed min, no return */
        32, /* or, no return */
        234, /* xor, no return */

        0, /* add, return */
        UINT64_MAX, /* and, return */
        1, /* compare+swap, return */
        0, /* exchange, return */
        0, /* unsigned max, return */
        UINT64_MAX, /* unsigned min, return */
        0, /* signed max, return */
        INT64_MAX, /* signed min, return */
        32, /* or, return */
        234, /* xor, return */

        0, /* return fail state */
    };
    uint64_t values[] = {
        /* no return */
        42,
        42,
        42,
        42,
        42,
        42,
        42,
        42,
        -42,
        42,
        42,

        /* return */
        42,
        42,
        42,
        42,
        42,
        42,
        42,
        -42,
        42,
        42,

        /* return fail state */
        1,
    };
    assert(sizeof(values) == sizeof(init));
    for (unsigned i = 0; i < sizeof(values)/sizeof(values[0]) - 1; i++) {
        init[i] <<= 29;
        values[i] <<= 29;
    }
    test_common(cs, true, sizeof(values)/sizeof(values[0]), init, values);
}

test_define {
    .name = "func.buffer_reference.atomic64",
    .start = test64,
    .image_filename = "32x32-green.ref.png",
};

