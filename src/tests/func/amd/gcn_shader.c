// Copyright 2018 Valve Corporation
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
#include "amd_common.h"

#include "gcn_shader-spirv.h"

static void
time(void)
{
    t_require_ext("VK_AMD_gcn_shader");
    VkShaderModule fs = qoCreateShaderModuleGLSL(
        t_device, FRAGMENT,
        QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
        QO_EXTENSION GL_AMD_gcn_shader : enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            int nothing;
        };
        /* We cannot really test the time output.
         * Thus, we only check for monotony. */
        void main() {
            float res = 0.0;
            uint64_t time1 = timeAMD();
            if (time1 == 0) res = 1.0;
            uint64_t time2 = timeAMD();
            if (time2 < time1) res = 1.0;

            f_color = vec4(res, 1.0 - res, 0.0, 1.0);
        }
    );
    struct {
        uint32_t nothing;
    } push;
    run_simple_pipeline(fs, &push, sizeof(push));
}
test_define {
    .name = "func.amd.gcn-shader.time",
    .start = time,
    .image_filename = "32x32-green.ref.png",
};

static void
cubeFaceCoordTC(void)
{
    t_require_ext("VK_AMD_gcn_shader");
    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_gcn_shader : enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            vec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            uint idx = gl_GlobalInvocationID.x;
            ssbo.v[idx].w = cubeFaceCoordAMD(ssbo.v[idx].xyz).x;
        }
    );

    const struct {
        float params[3];
        float result;
    } cases[] = {
        {{1.0, -0.1, 0.8,},  0.1},  // -y
        {{-1.0, 0.5, 0.3,},  0.35}, // -y
        {{-0.2, 1.0, 0.7,},  0.4},  // +z
        {{0.9, -1.0, 0.4,},  0.05}, // -z
        {{-0.1, 0.0, 1.0,},  0.45}, // -y
        {{0.3, 0.6, -1.0,},  0.65}, // -y
        /* corner cases */
        {{1.0, 1.0, -0.0,},  1.0},  // +z -> posY
        {{1.0, -1.0, 0.0,}, -0.0},  // -z -> negY
        {{0.0, 0.0,  0.0,}, -NAN},  // -y -> posZ
        {{1.0, 1.0, -1.0,},  1.0},  // -y -> negZ
    };

    RUN_CASES_CLOSE(float, "%f");
}
test_define {
    .name = "func.amd.gcn-shader.cube-face-coord-tc",
    .start = cubeFaceCoordTC,
    .no_image = true,
};

static void
cubeFaceCoordSC(void)
{
    t_require_ext("VK_AMD_gcn_shader");
    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_gcn_shader : enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            vec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            uint idx = gl_GlobalInvocationID.x;
            ssbo.v[idx].w = cubeFaceCoordAMD(ssbo.v[idx].xyz).y;
        }
    );

    const struct {
        float params[3];
        float result;
    } cases[] = {
        {{1.0, -0.1, 0.8,},  0.55}, // -z
        {{-1.0, 0.5, 0.3,},  0.75}, // +z
        {{-0.2, 1.0, 0.7,},  0.85}, // +x
        {{0.9, -1.0, 0.4,},  0.7},  // +x
        {{-0.1, 0.0, 1.0,},  0.5},  // +x
        {{0.3, 0.6, -1.0,},  0.8},  // -x
        /* corner cases */
        {{1.0, 1.0, -0.0,},  0.5}, // +x -> posY
        {{1.0, -1.0, 0.0,},  0.5}, // +x -> negY
        {{0.0, 0.0,  0.0,}, -NAN}, // +x -> posZ
        {{1.0, 1.0, -1.0,},  1.0}, // -x -> negZ
    };

    RUN_CASES_CLOSE(float, "%f");
}
test_define {
    .name = "func.amd.gcn-shader.cube-face-coord-sc",
    .start = cubeFaceCoordSC,
    .no_image = true,
};

static void
cubeFaceIndex(void)
{
    t_require_ext("VK_AMD_gcn_shader");
    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_gcn_shader : enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            vec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            uint idx = gl_GlobalInvocationID.x;
            ssbo.v[idx].w = cubeFaceIndexAMD(ssbo.v[idx].xyz);
        }
    );

    const struct {
        float params[3];
        float result;
    } cases[] = {
        {{1.0, -0.1, 0.8,}, 0.0},
        {{-1.0, 0.5, 0.3,}, 1.0},
        {{-0.2, 1.0, 0.7,}, 2.0},
        {{0.9, -1.0, 0.4,}, 3.0},
        {{-0.1, 0.0, 1.0,}, 4.0},
        {{0.3, 0.6, -1.0,}, 5.0},
        /* corner cases */
        {{1.0, 1.0, -0.0,}, 2.0},
        {{1.0, -1.0, 0.0,}, 3.0},
        {{0.0, 0.0,  0.0,}, 4.0},
        {{1.0, 1.0, -1.0,}, 5.0},
    };

    RUN_CASES(float, "%f");
}
test_define {
    .name = "func.amd.gcn-shader.cube-face-index",
    .start = cubeFaceIndex,
    .no_image = true,
};


static void
constant_folding(void)
{
    t_require_ext("VK_AMD_gcn_shader");
    VkShaderModule fs = qoCreateShaderModuleGLSL(
        t_device, FRAGMENT,
        QO_EXTENSION GL_AMD_gcn_shader : enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            vec3 i[8];
        };

        /* compare integer representations to handle NaNs */
        bool test(float a, float b) {
            return floatBitsToInt(a) == floatBitsToInt(b);
        }

        bool test(vec2 a, vec2 b) {
            return floatBitsToInt(a) == floatBitsToInt(b);
        }

        void main() {
            if (test(cubeFaceCoordAMD(vec3(1.0, -0.1, 0.8)), cubeFaceCoordAMD(i[0])) &&
            test(cubeFaceCoordAMD(vec3(-1.0, 0.5, 0.3)), cubeFaceCoordAMD(i[1])) &&
            test(cubeFaceCoordAMD(vec3(-0.2, 1.0, 0.7)), cubeFaceCoordAMD(i[2])) &&
            test(cubeFaceCoordAMD(vec3(0.9, -1.0, 0.4)), cubeFaceCoordAMD(i[3])) &&
            test(cubeFaceCoordAMD(vec3(-0.1, 0.0, 1.0)), cubeFaceCoordAMD(i[4])) &&
            test(cubeFaceCoordAMD(vec3(0.3, 0.6, -1.0)), cubeFaceCoordAMD(i[5])) &&
            test(cubeFaceCoordAMD(vec3(1.0, 1.0, -0.0)), cubeFaceCoordAMD(i[6])) &&
            test(cubeFaceCoordAMD(vec3(0.0, 0.0,  0.0)), cubeFaceCoordAMD(i[7])) &&
            test(cubeFaceIndexAMD(vec3(1.0, -0.1, 0.8)), cubeFaceIndexAMD(i[0])) &&
            test(cubeFaceIndexAMD(vec3(-1.0, 0.5, 0.3)), cubeFaceIndexAMD(i[1])) &&
            test(cubeFaceIndexAMD(vec3(-0.2, 1.0, 0.7)), cubeFaceIndexAMD(i[2])) &&
            test(cubeFaceIndexAMD(vec3(0.9, -1.0, 0.4)), cubeFaceIndexAMD(i[3])) &&
            test(cubeFaceIndexAMD(vec3(-0.1, 0.0, 1.0)), cubeFaceIndexAMD(i[4])) &&
            test(cubeFaceIndexAMD(vec3(0.3, 0.6, -1.0)), cubeFaceIndexAMD(i[5])) &&
            test(cubeFaceIndexAMD(vec3(1.0, 1.0, -0.0)), cubeFaceIndexAMD(i[6])) &&
            test(cubeFaceIndexAMD(vec3(0.0, 0.0,  0.0)), cubeFaceIndexAMD(i[7])))
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );
    struct {
        float i[4];
    } push[8] = {
        {{1.0, -0.1, 0.8, 0.0}},
        {{-1.0, 0.5, 0.3, 1.0}},
        {{-0.2, 1.0, 0.7, 2.0}},
        {{0.9, -1.0, 0.4, 3.0}},
        {{-0.1, 0.0, 1.0, 4.0}},
        {{0.3, 0.6, -1.0, 5.0}},
        /* corner cases */
        {{1.0, 1.0, -0.0, 6.0}},
        {{0.0, 0.0,  0.0, 7.0}},
    };
    run_simple_pipeline(fs, &push, sizeof(push));
}
test_define {
    .name = "func.amd.gcn-shader.constant",
    .start = constant_folding,
    .image_filename = "32x32-green.ref.png",
};
