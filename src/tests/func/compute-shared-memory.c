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

#include "src/tests/func/compute-shared-memory-spirv.h"

static void
shared_memory_bool_scalar(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(local_size_x = 2) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint result[2];
        };

        shared bool c;

        void main() {
            if (gl_LocalInvocationID.x == 0) {
                c = true;
            }

            barrier();

            result[gl_LocalInvocationID.x] = c ? 22 : 0;
        }
    );

    uint32_t result[2] = {0, 0};
    simple_compute_pipeline_options_t opts = {
        .storage = &result,
        .storage_size = sizeof(result),
    };
    run_simple_compute_pipeline(cs, &opts);

    t_assert(result[0] == 22);
    t_assert(result[1] == 22);

    t_pass();
}

test_define {
    .name = "func.compute.shared-memory.bool_scalar",
    .start = shared_memory_bool_scalar,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};


static void
shared_memory_bool_two_scalars(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(local_size_x = 2) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint result[2];
        };

        shared bool c;
        shared bool d;

        void main() {
            switch (gl_LocalInvocationID.x) {
            case 0: c = true; break;
            case 1: d = true; break;
            }

            barrier();

            result[gl_LocalInvocationID.x] = c && d ? 22 : 0;
        }
    );

    uint32_t result[2] = {0, 0};
    simple_compute_pipeline_options_t opts = {
        .storage = &result,
        .storage_size = sizeof(result),
    };
    run_simple_compute_pipeline(cs, &opts);

    t_assert(result[0] == 22);
    t_assert(result[1] == 22);

    t_pass();
}

test_define {
    .name = "func.compute.shared-memory.bool_two_scalars",
    .start = shared_memory_bool_two_scalars,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};


static void
shared_memory_bool_mixed_scalars(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(local_size_x = 4) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint result[4];
        };

        shared bool c;
        shared int d;
        shared bool e;

        void main() {
            switch (gl_LocalInvocationID.x) {
            case 0: c = true; break;
            case 1: d = 1; break;
            case 2: e = true; break;
            }

            barrier();

            result[gl_LocalInvocationID.x] = c && (d == 1) && e ? 22 : 0;
        }
    );

    uint32_t result[4] = {0, 0, 0, 0};
    simple_compute_pipeline_options_t opts = {
        .storage = &result,
        .storage_size = sizeof(result),
    };
    run_simple_compute_pipeline(cs, &opts);

    for (int i = 0; i < 4; i++)
        t_assert(result[i] == 22);

    t_pass();
}

test_define {
    .name = "func.compute.shared-memory.bool_mixed_scalars",
    .start = shared_memory_bool_mixed_scalars,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};


static void
shared_memory_bool_array(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(local_size_x = 4) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint result[4];
        };

        shared bool c[4];

        void main() {
            c[gl_LocalInvocationID.x] = true;

            barrier();

            uint next = (gl_LocalInvocationID.x + 1) % 4;
            result[gl_LocalInvocationID.x] = c[next] ? 22 : 0;
        }
    );

    uint32_t result[4] = {0, 0, 0, 0};
    simple_compute_pipeline_options_t opts = {
        .storage = &result,
        .storage_size = sizeof(result),
    };
    run_simple_compute_pipeline(cs, &opts);

    for (int i = 0; i < 4; i++)
        t_assert(result[i] == 22);

    t_pass();
}

test_define {
    .name = "func.compute.shared-memory.bool_array",
    .start = shared_memory_bool_array,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};


static void
shared_memory_bool_vector(void)
{
    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        layout(local_size_x = 4) in;

        layout(set = 0, binding = 0) buffer Storage {
            uint result[4];
        };

        shared bvec4 c;

        void main() {
            switch (gl_LocalInvocationID.x) {
            case 0: c.x = true; break;
            case 1: c.y = true; break;
            case 2: c.w = true; break;
            case 3: c.z = true; break;
            }

            barrier();

            bool next = false;
            switch (gl_LocalInvocationID.x) {
            case 0: next = c.y; break;
            case 1: next = c.z; break;
            case 2: next = c.w; break;
            case 3: next = c.x; break;
            }
            
            result[gl_LocalInvocationID.x] = next ? 22 : 0;
        }
    );

    uint32_t result[4] = {0, 0, 0, 0};
    simple_compute_pipeline_options_t opts = {
        .storage = &result,
        .storage_size = sizeof(result),
    };
    run_simple_compute_pipeline(cs, &opts);

    for (int i = 0; i < 4; i++)
        t_assert(result[i] == 22);

    t_pass();
}

test_define {
    .name = "func.compute.shared-memory.bool_vector",
    .start = shared_memory_bool_vector,
    .no_image = true,
    .queue_setup = QUEUE_SETUP_COMPUTE,
};
