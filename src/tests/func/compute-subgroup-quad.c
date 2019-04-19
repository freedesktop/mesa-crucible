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

#include "compute-subgroup-quad-spirv.h"

static bool
has_subgroup_quad_operations()
{
    VkPhysicalDeviceSubgroupProperties subgroup_props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES,
    };
    VkPhysicalDeviceProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &subgroup_props,
    };
    vkGetPhysicalDeviceProperties2(t_physical_dev, &props);
    t_assert(subgroup_props.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT);
    return subgroup_props.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT;
}

#include <stdio.h>

static void
subgroup_quad_swap_vertical_linear(void)
{
    // Tests the regular operation, which in a compute shader will
    // pick 4 elements linearly for each quad.

    if (!has_subgroup_quad_operations())
        t_skipf("subgroupQuad operations not supported");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_TARGET_ENV vulkan1.1
        QO_EXTENSION GL_KHR_shader_subgroup_quad: require

        layout(local_size_x = 4, local_size_y = 2) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint expected[8];
            uint fail;
        };

        void main() {
            uint index = gl_LocalInvocationIndex;
            uint swapped = subgroupQuadSwapVertical(index);
            if (expected[index] != swapped)
                atomicAdd(fail, 1);
        }
    );

    struct {
        uint32_t expected[8];
        uint32_t failed;
    } data = {
        {
            2, 3,
            0, 1,

            6, 7,
            4, 5
        },
    };

    simple_compute_pipeline_options_t opts = {
        .storage = &data,
        .storage_size = sizeof(data),
    };
    run_simple_compute_pipeline(cs, &opts);
    t_assert(!data.failed);

    t_pass();
}

test_define {
    .name = "func.compute.subgroup.swap_vertical.linear",
    .start = subgroup_quad_swap_vertical_linear,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
    .api_version = VK_MAKE_VERSION(1, 1, 0),
};

static bool
has_derivative_group_quads()
{
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV derivative_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV,
    };
    VkPhysicalDeviceFeatures2 pfeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &derivative_features
    };
    vkGetPhysicalDeviceFeatures2(t_physical_dev, &pfeatures);
    return derivative_features.computeDerivativeGroupQuads;
}

static void
subgroup_quad_swap_vertical_grid(void)
{
    // Use derivatives extension to specify the 4 elements will be an
    // actual 2x2 grid.

    if (!has_subgroup_quad_operations())
        t_skipf("subgroupQuad operations not supported");

    t_require_ext("VK_NV_compute_shader_derivatives");
    if (!has_derivative_group_quads())
        t_skipf("derivative_group_quadsNV not supported");

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_TARGET_ENV vulkan1.1
        QO_EXTENSION GL_NV_compute_shader_derivatives: require
        QO_EXTENSION GL_KHR_shader_subgroup_quad: require

        layout(local_size_x = 4, local_size_y = 2) in;
        layout(derivative_group_quadsNV) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint expected[8];
            uint fail;
        };

        void main() {
            uint index = gl_LocalInvocationIndex;
            uint swapped = subgroupQuadSwapVertical(index);
            if (expected[index] != swapped)
                atomicAdd(fail, 1);
        }
    );

    struct {
        uint32_t expected[8];
        uint32_t failed;
    } data = {
        {
            4, 5, 6, 7,
            0, 1, 2, 3,
        },
    };

    simple_compute_pipeline_options_t opts = {
        .storage = &data,
        .storage_size = sizeof(data),
    };
    run_simple_compute_pipeline(cs, &opts);
    t_assert(!data.failed);

    t_pass();
}

test_define {
    .name = "func.compute.subgroup.swap_vertical.grid",
    .start = subgroup_quad_swap_vertical_grid,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
    .api_version = VK_MAKE_VERSION(1, 1, 0),
};
