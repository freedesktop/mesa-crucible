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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include "tapi/t.h"

static uint64_t
get_timestamp(void)
{
    VkQueryPool pool;
    vkCreateQueryPool(t_device,
        &(VkQueryPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
            .queryType = VK_QUERY_TYPE_TIMESTAMP,
            .queryCount = 2,
        }, NULL, &pool);

    VkCommandBuffer cmdBuffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmdBuffer);
    vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 0);
    vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, pool, 1);
    qoEndCommandBuffer(cmdBuffer);

    qoQueueSubmit(t_queue, 1, &cmdBuffer, VK_NULL_HANDLE);

    vkQueueWaitIdle(t_queue);

    uint64_t results[2];
    vkGetQueryPoolResults(t_device, pool, 0, 2,
                          sizeof(results), results, sizeof *results,
                          VK_QUERY_RESULT_64_BIT);

    printf("top timestamp:       %20" PRId64 "  (%016" PRIx64 ")\n", results[0], results[0]);
    printf("bottom timestamp:    %20" PRId64 "  (%016" PRIx64")\n", results[1], results[1]);

    return results[0];
}

static void
test_timestamp(void)
{
    uint64_t a, b, freq, elapsed_ms;

    a = get_timestamp();
    t_assert(poll(NULL, 0, 100) == 0);
    b = get_timestamp();

    freq = 1 / (t_physical_dev_props->limits.timestampPeriod * 1000);
    elapsed_ms = (b - a) / freq;
    printf("difference: %" PRIu64 " - %" PRIu64 " = %" PRIu64 "\n", b / freq, a / freq, elapsed_ms);
    if (elapsed_ms < 90 || elapsed_ms > 110)
        t_fail();
    else
        t_pass();
}

test_define {
    .name = "func.query.timestamp",
    .start = test_timestamp,
    .no_image = true,
};
