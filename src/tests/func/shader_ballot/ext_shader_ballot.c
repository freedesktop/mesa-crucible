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

#include "ext_shader_ballot-spirv.h"

static void
ballot_basic(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_shader_ballot : enable
        layout(location = 0) out vec4 f_color;
        void main()
        {
            if (ballotARB(true) == 0)
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
            else if (ballotARB(false) != 0)
                f_color = vec4(1.0, 0.1, 0.0, 1.0);
            else
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.shader-ballot.basic",
    .start = ballot_basic,
    .image_filename = "32x32-green.ref.png",
};

static void
ballot_if_else(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
    QO_EXTENSION GL_ARB_shader_ballot : enable
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

        void main() {
            uint64_t active_set = ballotARB(true);
            bool ok = true;

            uint64_t even_set = active_set & 0x5555555555555555ul;
            uint64_t odd_set = active_set & 0xaaaaaaaaaaaaaaaaul;
            uint64_t even_inner = 0ul;
            uint64_t odd_inner = 0ul;


            f_color = vec4(0.0, 1.0, 0.0, 1.0);

            if (gl_SubGroupInvocationARB % 2u == 0u) {
                even_inner = ballotARB(true);
                if (ok && even_inner != even_set) {
                    f_color = vec4(1.0, 0.1, 0.0, gl_SubGroupInvocationARB / 255.0);
                    ok = false;
                }
            } else {
                odd_inner = ballotARB(true);
                if (ok && odd_inner != odd_set) {
                    f_color = vec4(1.0, 0.2, 0.0, gl_SubGroupInvocationARB / 255.0);
                    ok = false;
                }
            }

            int first_even = findLSB64(even_set);
            int first_odd = findLSB64(odd_set);

            if (first_even >= 0) {
                even_inner = packUint2x32(readInvocationARB(unpackUint2x32(even_inner), first_even));
            }
            if (first_odd >= 0) {
                odd_inner = packUint2x32(readInvocationARB(unpackUint2x32(odd_inner), first_odd));
            }

            if (ok && even_inner != even_set) {
                f_color = vec4(1.0, 0.3, first_even / 255.0, gl_SubGroupInvocationARB / 255.0);
            }
            if (ok && odd_inner != odd_set) {
                f_color = vec4(1.0, 0.4, first_odd / 255.0, gl_SubGroupInvocationARB / 255.0);
            }
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.shader-ballot.if-else",
    .start = ballot_if_else,
    .image_filename = "32x32-green.ref.png",
};


static void
builtins(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
    QO_EXTENSION GL_ARB_shader_ballot : enable
        layout(location = 0) out vec4 f_color;

        void main() {
            uint64_t active_set = ballotARB(true);

            uint id = gl_SubGroupInvocationARB;

            if (id >= gl_SubGroupSizeARB) {
                f_color = vec4(1.0, 0 / 255.0, id / 255.0, gl_SubGroupSizeARB / 255.0);
                return;
            }

            if (gl_SubGroupEqMaskARB != (1ul << id)) {
                f_color = vec4(1.0, 1 / 255.0, id / 255.0, 0.0);
                return;
            }

            if (gl_SubGroupGeMaskARB != (~0ul << id)) {
                f_color = vec4(1.0, 2 / 255.0, id / 255.0, 0.0);
                return;
            }

            if (gl_SubGroupGtMaskARB != (~1ul << id)) {
                f_color = vec4(1.0, 2 / 255.0, id / 255.0, 0.0);
                return;
            }

            if (gl_SubGroupLeMaskARB != ~(~1ul << id)) {
                f_color = vec4(1.0, 3 / 255.0, id / 255.0, 0.0);
                return;
            }

            if (gl_SubGroupLtMaskARB != ~(~0ul << id)) {
                f_color = vec4(1.0, 4 / 255.0, id / 255.0, 0.0);
                return;
            }
            f_color = vec4(0.0, 1.0, 0.0, 1.0);
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.shader-ballot.builtins",
    .start = builtins,
    .image_filename = "32x32-green.ref.png",
};

static void
read_first_invocation(void)
{
    t_require_ext("VK_EXT_shader_subgroup_ballot");
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
    QO_EXTENSION GL_ARB_shader_ballot : enable
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

        void main() {
            uint64_t active_set = ballotARB(true);
            int first_invocation = findLSB64(active_set);

            uint expected = uint(42 + first_invocation);
            uint read = readFirstInvocationARB(gl_SubGroupInvocationARB + 42u);

            if (read == expected)
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, read / 255.0, expected / 255.0, gl_SubGroupInvocationARB / 255.0);
        }
    );
    run_simple_pipeline(fs, NULL, 0);
}

test_define {
    .name = "func.shader-ballot.readFirstInvocation",
    .start = read_first_invocation,
    .image_filename = "32x32-green.ref.png",
};

