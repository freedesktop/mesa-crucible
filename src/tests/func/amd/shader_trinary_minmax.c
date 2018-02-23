// Copyright 2018 Google LLC
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
#include <math.h>
#include "amd_common.h"

#include "shader_trinary_minmax-spirv.h"

static void
umin3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            uvec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = min3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        uint32_t params[3];
        uint32_t result;
    } cases[] = {
        {{0, 0, 0,}, 0},
        {{UINT32_MAX, UINT32_MAX, UINT32_MAX}, UINT32_MAX},
        {{1, 5, 7}, 1},
        {{UINT32_MAX, 5, 5}, 5},
        {{5, 0, UINT32_MAX}, 0},
        {{UINT32_MAX/2, UINT32_MAX/3, UINT32_MAX/4}, UINT32_MAX/4},
    };

    RUN_CASES(uint32_t, "%u");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.umin3",
    .start = umin3_compute,
    .no_image = true,
};


static void
smin3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            ivec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = min3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        int32_t params[3];
        int32_t result;
    } cases[] = {
        {{0, 0, 0,}, 0},
        {{INT32_MAX, INT32_MAX, INT32_MAX}, INT32_MAX},
        {{INT32_MIN, INT32_MIN, INT32_MIN}, INT32_MIN},
        {{1, 5, 7}, 1},
        {{INT32_MAX, 5, 5}, 5},
        {{5, 0, INT32_MAX}, 0},
        {{INT32_MAX/2, INT32_MAX/3, INT32_MAX/4}, INT32_MAX/4},
        {{INT32_MIN, INT32_MAX, 0}, INT32_MIN},
    };

    RUN_CASES(int32_t, "%d");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.smin3",
    .start = smin3_compute,
    .no_image = true,
};

static void
fmin3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            vec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = min3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        float params[3];
        float result;
    } cases[] = {
        /* No NaN testcases as those are undefined for this ext. */
        {{0, 0, 0,}, 0},
        {{INFINITY, INFINITY, INFINITY}, INFINITY},
        {{-INFINITY, -INFINITY, -INFINITY}, -INFINITY},
        {{-0, 0, 5}, -0},
        {{-1e10, 1e10, 5}, -1e10},
    };

    RUN_CASES(float, "%f");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.fmin3",
    .start = fmin3_compute,
    .no_image = true,
};


static void
umax3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            uvec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = max3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        uint32_t params[3];
        uint32_t result;
    } cases[] = {
        {{0, 0, 0,}, 0},
        {{UINT32_MAX, UINT32_MAX, UINT32_MAX}, UINT32_MAX},
        {{1, 5, 7}, 7},
        {{UINT32_MAX, 5, 5}, UINT32_MAX},
        {{5, 0, UINT32_MAX}, UINT32_MAX},
        {{UINT32_MAX/2, UINT32_MAX/3, UINT32_MAX/4}, UINT32_MAX/2},
    };

    RUN_CASES(uint32_t, "%u");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.umax3",
    .start = umax3_compute,
    .no_image = true,
};


static void
smax3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            ivec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = max3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        int32_t params[3];
        int32_t result;
    } cases[] = {
        {{0, 0, 0,}, 0},
        {{INT32_MAX, INT32_MAX, INT32_MAX}, INT32_MAX},
        {{INT32_MIN, INT32_MIN, INT32_MIN}, INT32_MIN},
        {{1, 5, 7}, 7},
        {{INT32_MAX, 5, 5}, INT32_MAX},
        {{5, 0, INT32_MAX}, INT32_MAX},
        {{INT32_MAX/2, INT32_MAX/3, INT32_MAX/4}, INT32_MAX/2},
        {{INT32_MIN, INT32_MAX, 0}, INT32_MAX},
    };

    RUN_CASES(int32_t, "%d");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.smax3",
    .start = smax3_compute,
    .no_image = true,
};

static void
fmax3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            vec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = max3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        float params[3];
        float result;
    } cases[] = {
        /* No NaN testcases as those are undefined for this ext. */
        {{0, 0, 0,}, 0},
        {{INFINITY, INFINITY, INFINITY}, INFINITY},
        {{-INFINITY, -INFINITY, -INFINITY}, -INFINITY},
        {{-0, 0, 5}, 5},
        {{-1e10, 1e10, 5}, 1e10},
    };

    RUN_CASES(float, "%f");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.fmax3",
    .start = fmax3_compute,
    .no_image = true,
};

static void
umid3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            uvec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = mid3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        uint32_t params[3];
        uint32_t result;
    } cases[] = {
        {{0, 0, 0,}, 0},
        {{UINT32_MAX, UINT32_MAX, UINT32_MAX}, UINT32_MAX},
        {{1, 5, 7}, 5},
        {{UINT32_MAX, 5, 5}, 5},
        {{5, 0, UINT32_MAX}, 5},
        {{UINT32_MAX/2, UINT32_MAX/3, UINT32_MAX/4}, UINT32_MAX/3},
    };

    RUN_CASES(uint32_t, "%u");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.umid3",
    .start = umid3_compute,
    .no_image = true,
};


static void
smid3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            ivec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = mid3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        int32_t params[3];
        int32_t result;
    } cases[] = {
        {{0, 0, 0,}, 0},
        {{INT32_MAX, INT32_MAX, INT32_MAX}, INT32_MAX},
        {{INT32_MIN, INT32_MIN, INT32_MIN}, INT32_MIN},
        {{1, 5, 7}, 5},
        {{INT32_MAX, 5, 5}, 5},
        {{5, 0, INT32_MAX}, 5},
        {{INT32_MAX/2, INT32_MAX/3, INT32_MAX/4}, INT32_MAX/3},
        {{INT32_MIN, INT32_MAX, 0}, 0},
    };

    RUN_CASES(int32_t, "%d");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.smid3",
    .start = smid3_compute,
    .no_image = true,
};

static void
fmid3_compute(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");

    VkShaderModule cs = qoCreateShaderModuleGLSL(
        t_device, COMPUTE,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(set = 0, binding = 0, std430) buffer Storage {
            vec4 v[];
        } ssbo;

        layout (local_size_x = 1) in;

        void main()
        {
            ssbo.v[gl_GlobalInvocationID.x].w = mid3(ssbo.v[gl_GlobalInvocationID.x].x,
                                                     ssbo.v[gl_GlobalInvocationID.x].y,
                                                     ssbo.v[gl_GlobalInvocationID.x].z);
        }
    );

    const struct {
        float params[3];
        float result;
    } cases[] = {
        /* No NaN testcases as those are undefined for this ext. */
        {{0, 0, 0,}, 0},
        {{INFINITY, INFINITY, INFINITY}, INFINITY},
        {{-INFINITY, -INFINITY, -INFINITY}, -INFINITY},
        {{-0.0, 1.0, -5.0}, -0.0},
        {{-1e10, 1e10, 5}, 5},
    };

    RUN_CASES(float, "%f");
}

test_define {
    .name = "func.amd.shader-trinary-minmax.fmid3",
    .start = fmid3_compute,
    .no_image = true,
};

static void
constant_min(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");
    VkShaderModule fs = qoCreateShaderModuleGLSL(
        t_device, FRAGMENT,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            vec4 f[3];
            uvec4 u[3];
            ivec4 i[2];
        };

        void main() {
            if (min3(f[0].x, f[0].y, f[0].z) == min3(1e10, 1.1e10, 1.2e10) &&
            min3(f[1].x, f[1].y, f[1].z) == min3(-9.0, -0.0, -5.0) &&
            min3(f[2].x, f[2].y, f[2].z) == min3(0.0, -0.0, -5.0) &&
            min3(u[0].x, u[0].y, u[0].z) == min3(0u, 0u, 1u) &&
            min3(u[1].x, u[1].y, u[1].z) == min3(4294967295u, 4294967295u, 4294967295u) &&
            min3(u[2].x, u[2].y, u[2].z) == min3(5u, 1u, 3u) &&
            min3(i[0].x, i[0].y, i[0].z) == min3(2147483647, 2147483647, 2147483647) &&
            min3(i[1].x, i[1].y, i[1].z) == min3(-2147483648, 2147483647, 5))
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );
    struct {
        float f[3][4];
        uint32_t u[3][4];
        int32_t i[2][4];
    } push = {
        {{1e10, 1.1e10, 1.2e10, 0},
        {-9.0, -0.0, -5.0, 0},
        {0.0, -0.0, -5.0, 0}},
        {{0u, 0u, 1u, 0u},
        {4294967295u, 4294967295u, 4294967295u, 0u},
        {5u, 1u, 3u, 0u}},
        {{2147483647, 2147483647, 2147483647, 0},
        {-2147483648, 2147483647, 5, 0}},
    };
    run_simple_pipeline(fs, &push, sizeof(push));
}
test_define {
    .name = "func.amd.shader-trinary-minmax.min3-constant",
    .start = constant_min,
    .image_filename = "32x32-green.ref.png",
};

static void
constant_max(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");
    VkShaderModule fs = qoCreateShaderModuleGLSL(
        t_device, FRAGMENT,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            vec4 f[3];
            uvec4 u[3];
            ivec4 i[2];
        };

        void main() {
            if (max3(f[0].x, f[0].y, f[0].z) == max3(1e10, 1.1e10, 1.2e10) &&
            max3(f[1].x, f[1].y, f[1].z) == max3(-9.0, -0.0, -5.0) &&
            max3(f[2].x, f[2].y, f[2].z) == max3(0.0, -0.0, -5.0) &&
            max3(u[0].x, u[0].y, u[0].z) == max3(5u, 4294967295u, 1u) &&
            max3(u[1].x, u[1].y, u[1].z) == max3(0u, 0u, 1u) &&
            max3(u[2].x, u[2].y, u[2].z) == max3(17u, 3u, 2u) &&
            max3(i[0].x, i[0].y, i[0].z) == max3(-2147483648, -2147483648, -2147483648) &&
            max3(i[1].x, i[1].y, i[1].z) == max3(-2147483648, 2147483647, 5))
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );
    struct {
        float f[3][4];
        uint32_t u[3][4];
        int32_t i[2][4];
    } push = {
        {{1e10, 1.1e10, 1.2e10, 0},
        {-9.0, -0.0, -5.0, 0},
        {0.0, -0.0, -5.0, 0}},
        {{5u, 4294967295u, 1u, 0u},
        {0u, 0u, 1u, 0u},
        {17u, 3u, 2u, 0u}},
        {{-2147483648, -2147483648, -2147483648, 0},
        {-2147483648, 2147483647, 5, 0}},
    };
    run_simple_pipeline(fs, &push, sizeof(push));
}
test_define {
    .name = "func.amd.shader-trinary-minmax.max3-constant",
    .start = constant_max,
    .image_filename = "32x32-green.ref.png",
};

static void
constant_mid(void)
{
    t_require_ext("VK_AMD_shader_trinary_minmax");
    VkShaderModule fs = qoCreateShaderModuleGLSL(
        t_device, FRAGMENT,
        QO_EXTENSION GL_AMD_shader_trinary_minmax: enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            vec4 f[3];
            uvec4 u[3];
            ivec4 i[2];
        };

        void main() {
            if (mid3(f[0].x, f[0].y, f[0].z) == mid3(-5.0, -0.0, 1.0) &&
            mid3(f[1].x, f[1].y, f[1].z) == mid3(-1e10, -1e10, 1e10) &&
            mid3(f[2].x, f[2].y, f[2].z) == mid3(-0.0, 0.0, -0.0) &&
            mid3(u[0].x, u[0].y, u[0].z) == mid3(0u, 4294967295u, 4294967295u) &&
            mid3(u[1].x, u[1].y, u[1].z) == mid3(0u, 0u, 5u) &&
            mid3(u[2].x, u[2].y, u[2].z) == mid3(7u, 1u, 3u) &&
            mid3(i[0].x, i[0].y, i[0].z) == mid3(0, -2147483648, -2147483648) &&
            mid3(i[1].x, i[1].y, i[1].z) == mid3(2147483646, 0, 2147483647))
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );
    struct {
        float f[3][4];
        uint32_t u[3][4];
        int32_t i[2][4];
    } push = {
        {{-5.0, -0.0, 1.0, 0.0},
        {-1e10, -1e10, 1e10, 0.0},
        {-0.0, 0.0, -0.0, 0.0}},
        {{0u, 4294967295u, 4294967295u, 0u},
        {0u, 0u, 5u, 0u},
        {7u, 1u, 3u, 0u}},
        {{0, -2147483648, -2147483648, 0},
        {2147483646, 0, 2147483647, 0}},
	};
    run_simple_pipeline(fs, &push, sizeof(push));
}
test_define {
    .name = "func.amd.shader-trinary-minmax.mid3-constant",
    .start = constant_mid,
    .image_filename = "32x32-green.ref.png",
};
