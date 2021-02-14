// Copyright 2017 Valve Corporation
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

#include "src/tests/func/shader/pack_unpack-spirv.h"

static void
pack_double(void)
{
    t_require_feature(shaderFloat64);

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            uvec2 arg;
            double expected;
        };

        void main()
        {
            if (packDouble2x32(arg) == expected)
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    struct {
        uint32_t low;
        uint32_t high;
        double expected;
    } push;
    push.low = 0x0;
    push.high = 0x3ff00000;
    push.expected = 1.0;
    run_simple_pipeline(fs, &push, sizeof(push));
}

test_define {
    .name = "func.shader.packDouble2x32.basic",
    .start = pack_double,
    .image_filename = "32x32-green.ref.png",
};

static void
unpack_double(void)
{
    t_require_feature(shaderFloat64);

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            double arg;
            uvec2 expected;
        };

        void main()
        {
            if (unpackDouble2x32(arg) == expected)
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    struct {
        double arg;
        uint32_t low;
        uint32_t high;
    } push;
    push.low = 0x0;
    push.high = 0x3ff00000;
    push.arg = 1.0;
    run_simple_pipeline(fs, &push, sizeof(push));
}

test_define {
    .name = "func.shader.unpackDouble2x32.basic",
    .start = unpack_double,
    .image_filename = "32x32-green.ref.png",
};

static void
pack_int64(void)
{
    t_require_feature(shaderInt64);

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            uvec2 arg;
            uint64_t expected;
        };

        void main()
        {
            if (packUint2x32(arg) == expected)
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    struct {
        uint32_t low;
        uint32_t high;
        uint64_t expected;
    } push;
    push.low = 0x42;
    push.high = 0x43;
    push.expected = 0x0000004300000042;
    run_simple_pipeline(fs, &push, sizeof(push));
}

test_define {
    .name = "func.shader.packUint2x32.basic",
    .start = pack_int64,
    .image_filename = "32x32-green.ref.png",
};

static void
unpack_int64(void)
{
    t_require_feature(shaderInt64);

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
    QO_EXTENSION GL_ARB_gpu_shader_int64 : enable
        layout(location = 0) out vec4 f_color;
        layout(push_constant) uniform push_consts {
            uint64_t arg;
            uvec2 expected;
        };

        void main()
        {
            if (unpackUint2x32(arg) == expected)
                f_color = vec4(0.0, 1.0, 0.0, 1.0);
            else
                f_color = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    struct {
        uint64_t arg;
        uint32_t low;
        uint32_t high;
    } push;
    push.low = 0x42;
    push.high = 0x43;
    push.arg = 0x0000004300000042;
    run_simple_pipeline(fs, &push, sizeof(push));
}

test_define {
    .name = "func.shader.unpackUint2x32.basic",
    .start = unpack_int64,
    .image_filename = "32x32-green.ref.png",
};

