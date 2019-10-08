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

#include "ext_shader_subgroup_vote-spirv.h"

static void
basic(void)
{
    t_require_ext("VK_EXT_shader_subgroup_vote");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_shader_group_vote : enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            uint arg;
        };
        void main()
        {
            bool cond = bool(arg);
            if (allInvocationsARB(cond) != true) {
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
                return;
            }
            if (anyInvocationARB(cond) != true) {
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
                return;
            }
            if (allInvocationsEqualARB(cond) != true) {
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
                return;
            }
            f_color = vec4(0.0, 1.0, 0.0, 1.0);
        }
    );
    uint32_t arg = 1;
    run_simple_pipeline(fs, &arg, sizeof(arg));
}

test_define {
    .name = "func.shader-subgroup-vote.basic",
    .start = basic,
    .image_filename = "32x32-green.ref.png",
};

static void
advanced(void)
{
    t_require_ext("VK_EXT_shader_subgroup_vote");
    t_require_ext("VK_EXT_shader_subgroup_ballot");

    VkPhysicalDeviceFeatures features = {};
    vkGetPhysicalDeviceFeatures(t_physical_dev, &features);
    if (!features.shaderInt64)
        t_skipf("shaderInt64 not supported");

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_shader_group_vote : enable
    QO_EXTENSION GL_ARB_shader_ballot : enable
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
        layout(location = 0) out vec4 f_color;

        int findLSB64(uint64_t v)
        {
            uvec2 split = unpackUint2x32(v);
            int lsb = findLSB(split.x);
            if (lsb >= 0)
                return lsb;

            lsb = findLSB(split.y);
            if (lsb >= 0)
                return 32 + lsb;

            return -1;
        }

        void main()
        {
            uint64_t active_set = ballotARB(true);
            if (active_set == 1) {
                f_color = vec4(0.0, 1.0, 0.0, 0.0);
                return;
            }
            bool id_zero = gl_SubGroupInvocationARB == findLSB64(active_set);
            if (anyInvocationARB(id_zero) != true) {
                f_color = vec4(1.0, 0.1, 0.0, 1.0);
                return;
            }
            if (allInvocationsARB(id_zero) != false) {
                f_color = vec4(1.0, 0.2, 0.0, 1.0);
                return;
            }
            if (allInvocationsEqualARB(id_zero) != false) {
                f_color = vec4(1.0, 0.3, 0.0, 1.0);
                return;
            }
            f_color = vec4(0.0, 1.0, 0.0, 1.0);
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.shader-subgroup-vote.advanced",
    .start = advanced,
    .image_filename = "32x32-green.ref.png",
};
