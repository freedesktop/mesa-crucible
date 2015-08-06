/*
 * Copyright 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include "tapi/t.h"

/* This file implements a test that pokes at our internal allocators bo
 * allocating and destroying thousands of VkDynamicViewportState objects on
 * different threads.  I chose VkDynamicViewportState because, in the
 * current driver implementaiton, that's guaranteed to trigger three state
 * pool allocations.
 */

#define NUM_THREADS 16
#define ALLOCS_PER_THREAD_LOG2 12

struct anv_state {
   uint32_t offset;
   uint32_t alloc_size;
   void *map;
};

static void
do_test_alloc_multithread(void *_id)
{
    unsigned id = (uintptr_t)_id;
    VkDevice device = t_device;
    VkResult result;

    const VkDynamicViewportStateCreateInfo vp_info = {
        QO_DYNAMIC_VIEWPORT_STATE_CREATE_INFO_DEFAULTS,
        .viewportAndScissorCount = 1,
        .pViewports = (VkViewport[]) {
            {
                .originX = 0,
                .originY = 0,
                .width = 32,
                .height = 32,
                .minDepth = 0,
                .maxDepth = 1
            },
        },
        .pScissors = (VkRect2D[]) {
            {{ 0, 0 }, { 32, 32 }},
        }
    };

    const unsigned chunk_size = 1 << (id % ALLOCS_PER_THREAD_LOG2);
    const unsigned allocs = 1 << ALLOCS_PER_THREAD_LOG2;

    VkDynamicViewportState states[chunk_size];

    for (unsigned i = 0; i < allocs / chunk_size; i++) {
        for (unsigned j = 0; j < chunk_size; j++) {
            result = vkCreateDynamicViewportState(device, &vp_info, &states[j]);
            t_assert(result == VK_SUCCESS);
        }
        for (unsigned j = 0; j < chunk_size; j++) {
            result = vkDestroyDynamicViewportState(device, states[j]);
            t_assert(result == VK_SUCCESS);
        }
    }
}

static void
test_alloc_multithread(void)
{
    for (unsigned i = 0; i < NUM_THREADS; i++)
        t_create_thread(do_test_alloc_multithread, (void *)(uintptr_t)i);
    sleep(1);
}

cru_define_test {
    .name = "stress.alloc-multithread",
    .start = test_alloc_multithread,
    .no_image = true,
};
