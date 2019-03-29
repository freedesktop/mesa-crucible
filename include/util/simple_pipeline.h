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

#include <stddef.h>
#include "vk_wrapper.h"

void run_simple_pipeline(VkShaderModule fs, void *push_constants,
                         size_t push_constants_size);

typedef struct simple_compute_pipeline_options simple_compute_pipeline_options_t;

struct simple_compute_pipeline_options {
    void *push_constants;
    size_t push_constants_size;

    // Bound to set 0, descriptor 0.
    void *storage;
    size_t storage_size;

    // Defaults to 1 if not specified.
    uint32_t x_count;
    uint32_t y_count;
    uint32_t z_count;
};

void run_simple_compute_pipeline(VkShaderModule cs,
                                 const struct simple_compute_pipeline_options *opts);
