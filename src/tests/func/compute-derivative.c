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

#include "util/simple_pipeline.h"
#include "tapi/t.h"

#include "src/tests/func/compute-derivative-spirv.h"

static void
group_none(void)
{
    t_require_ext("VK_NV_compute_shader_derivatives");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_NV_compute_shader_derivatives: require

        layout(local_size_x = 4, local_size_y = 2) in;
        // No derivative group declared.

        layout(push_constant, std430) uniform Push {
            uint expected_dx;
            uint expected_dy;
        };

        layout(set = 0, binding = 0) buffer Storage {
            uint fail;
        };

        void main() {
            uint index = gl_LocalInvocationIndex;
            uint dx = uint(dFdxFine(index));
            uint dy = uint(dFdyFine(index));
            if (expected_dx != dx || expected_dy != dy)
                atomicAdd(fail, 1);

            // Check mapping between gl_LocalInvocationIndex and
            // gl_LocalInvocationID is preserved.
            uvec3 size = gl_WorkGroupSize;
            uvec3 id = gl_LocalInvocationID;
            if (id.x != (index % size.x) ||
                id.y != ((index / size.x) % size.y) ||
                id.z != ((index / (size.x * size.y)) % size.z))
                atomicAdd(fail, 1);
        }
    );

    // When the extension is used but no derivative group is set, the
    // derivatives return zero.
    uint32_t expected[2] = {0, 0};
    uint32_t failed = 0;
    simple_compute_pipeline_options_t opts = {
        .push_constants = expected,
        .push_constants_size = sizeof(expected),

        .storage = &failed,
        .storage_size = sizeof(failed),
    };
    run_simple_compute_pipeline(cs, &opts);
    t_assert(!failed);

    t_pass();
}

test_define {
    .name = "func.compute.derivative.group-none",
    .start = group_none,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};

static VkPhysicalDeviceComputeShaderDerivativesFeaturesNV
get_compute_shader_derivatives_features(void)
{
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV,
    };
    VkPhysicalDeviceFeatures2 pfeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features,
    };
    vkGetPhysicalDeviceFeatures2(t_physical_dev, &pfeatures);
    return features;
}

static void
group_linear(void)
{
    t_require_ext("VK_NV_compute_shader_derivatives");

    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV features =
        get_compute_shader_derivatives_features();
    if (!features.computeDerivativeGroupLinear)
        t_skipf("derivative_group_linearNV not supported");
    
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_NV_compute_shader_derivatives: require

        layout(local_size_x = 4, local_size_y = 2) in;
        layout(derivative_group_linearNV) in;

        layout(push_constant, std430) uniform Push {
            uint expected_dx;
            uint expected_dy;
        };

        layout(set = 0, binding = 0) buffer Storage {
            uint fail;
        };

        void main() {
            uint index = gl_LocalInvocationIndex;
            uint dx = uint(dFdxFine(index));
            uint dy = uint(dFdyFine(index));
            if (expected_dx != dx || expected_dy != dy)
                atomicAdd(fail, 1);

            // Check mapping between gl_LocalInvocationIndex and
            // gl_LocalInvocationID is preserved.
            uvec3 size = gl_WorkGroupSize;
            uvec3 id = gl_LocalInvocationID;
            if (id.x != (index % size.x) ||
                id.y != ((index / size.x) % size.y) ||
                id.z != ((index / (size.x * size.y)) % size.z))
                atomicAdd(fail, 1);
        }
    );

    // Linear takes four elements in sequence each time to create a
    // 2x2 grid, so in this test we'll have two grids:
    //
    //       +---+---+       +---+---+
    //       | 0 | 1 |       | 4 | 5 |
    //       +---+---+       +---+---+
    //       | 2 | 3 |       | 6 | 7 |
    //       +---+---+       +---+---+
    //
    // All the horizontal derivatives are 1 and all vertical
    // derivatives are 2.
    uint32_t expected[2] = {1, 2};
    uint32_t failed = 0;
    simple_compute_pipeline_options_t opts = {
        .push_constants = expected,
        .push_constants_size = sizeof(expected),

        .storage = &failed,
        .storage_size = sizeof(failed),
    };
    run_simple_compute_pipeline(cs, &opts);
    t_assert(!failed);

    t_pass();
}

test_define {
    .name = "func.compute.derivative.group-linear",
    .start = group_linear,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};

static void
group_quads(void)
{
    t_require_ext("VK_NV_compute_shader_derivatives");

    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV features =
        get_compute_shader_derivatives_features();
    if (!features.computeDerivativeGroupQuads)
        t_skipf("derivative_group_quadsNV not supported");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_NV_compute_shader_derivatives: require

        layout(local_size_x = 4, local_size_y = 2) in;
        layout(derivative_group_quadsNV) in;

        layout(push_constant, std430) uniform Push {
            uint expected_dx;
            uint expected_dy;
        };

        layout(set = 0, binding = 0) buffer Storage {
            uint fail;
        };

        void main() {
            uint index = gl_LocalInvocationIndex;
            uint dx = uint(dFdxFine(index));
            uint dy = uint(dFdyFine(index));
            if (expected_dx != dx || expected_dy != dy)
                atomicAdd(fail, 1);

            // Check mapping between gl_LocalInvocationIndex and
            // gl_LocalInvocationID is preserved.
            uvec3 size = gl_WorkGroupSize;
            uvec3 id = gl_LocalInvocationID;
            if (id.x != (index % size.x) ||
                id.y != ((index / size.x) % size.y) ||
                id.z != ((index / (size.x * size.y)) % size.z))
                atomicAdd(fail, 1);
        }
    );

    // Quads takes 2x2 pieces from the grid, so the mapping to
    // local invocation *indices* ends up like this
    //
    //       +---+---+       +---+---+
    //       | 0 | 1 |       | 2 | 3 |
    //       +---+---+       +---+---+
    //       | 4 | 5 |       | 6 | 7 |
    //       +---+---+       +---+---+
    //
    // All the horizontal derivatives are 1 and all vertical
    // derivatives are 2.
    uint32_t expected[2] = {1, 4};
    uint32_t failed = 0;
    simple_compute_pipeline_options_t opts = {
        .push_constants = expected,
        .push_constants_size = sizeof(expected),

        .storage = &failed,
        .storage_size = sizeof(failed),
    };
    run_simple_compute_pipeline(cs, &opts);
    t_assert(!failed);

    t_pass();
}

test_define {
    .name = "func.compute.derivative.group-quads",
    .start = group_quads,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};

static void
group_quads_multiple_subgroups(void)
{
    t_require_ext("VK_NV_compute_shader_derivatives");

    VkPhysicalDeviceSubgroupProperties subgroup_props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES,
    };
    VkPhysicalDeviceProperties2 p = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &subgroup_props,
    };
    vkGetPhysicalDeviceProperties2(t_physical_dev, &p);

    enum {
        x_size = 8,
        y_size = 6,
        z_size = 5,
        total_size = x_size * y_size * z_size,
    };

    t_assert(x_size <= p.properties.limits.maxComputeWorkGroupSize[0]);
    t_assert(y_size <= p.properties.limits.maxComputeWorkGroupSize[1]);
    t_assert(z_size <= p.properties.limits.maxComputeWorkGroupSize[2]);
    t_assert(total_size <= p.properties.limits.maxComputeWorkGroupInvocations);

    // We want multiple subgroups, to ensure the indices are correct
    // in that case.
    t_assert(total_size > subgroup_props.subgroupSize);

    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV features =
        get_compute_shader_derivatives_features();
    if (!features.computeDerivativeGroupQuads)
        t_skipf("derivative_group_quadsNV not supported");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_NV_compute_shader_derivatives: require

        layout(local_size_x = 8, local_size_y = 6, local_size_z = 5) in;
        layout(derivative_group_quadsNV) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint dy[];
        };

        void main() {
            uint index = gl_LocalInvocationIndex;
            dy[index] = uint(dFdyFine(index));

            // Check mapping between gl_LocalInvocationIndex and
            // gl_LocalInvocationID is preserved.
            uvec3 size = gl_WorkGroupSize;
            uvec3 id = gl_LocalInvocationID;
            if (id.x != (index % size.x) ||
                id.y != ((index / size.x) % size.y) ||
                id.z != ((index / (size.x * size.y)) % size.z)) {
                // Force failure since no derivative will ever be that large.
                dy[index] = (size.x * size.y * size.z) + 1;
            }
        }
    );

    uint32_t dy[total_size] = {0};
    simple_compute_pipeline_options_t opts = {
        .storage = &dy,
        .storage_size = sizeof(dy),
    };
    run_simple_compute_pipeline(cs, &opts);

    for (int i = 0; i < total_size; i++)
       t_assert(dy[i] == x_size);

    t_pass();
}

test_define {
    .name = "func.compute.derivative.group-quads-multiple-subgroups",
    .start = group_quads_multiple_subgroups,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};

