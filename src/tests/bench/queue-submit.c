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

#include "tapi/t.h"
#include <time.h>

#define MIN_BUFFER_COUNT 8
#define MAX_BUFFER_COUNT 4096
#define BUFFER_SIZE 256
#define NUM_EXECS 1000

static uint64_t
gettime_ns()
{
    struct timespec current;
    int ret = clock_gettime(CLOCK_MONOTONIC, &current);
    t_assert (ret >= 0);
    if (ret < 0)
        return 0;

    return (uint64_t) current.tv_sec * 1000000000ULL + current.tv_nsec;
}

static void
test_queue_submit_variable(unsigned buffer_count, VkBuffer *buffers)
{
    /* It's fine to re-begin a command buffer */
    qoBeginCommandBuffer(t_cmd_buffer,
                         .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    for (unsigned i = 0; i < buffer_count; i++) {
        if (i > 0) {
            vkCmdCopyBuffer(t_cmd_buffer, buffers[0], buffers[i], 1,
                &(VkBufferCopy) {
                    .srcOffset = 0,
                    .dstOffset = 0,
                    .size = BUFFER_SIZE,
                });
        }
    }

    qoEndCommandBuffer(t_cmd_buffer);

    /* Warm up the kernel driver */
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    /* We do all NUM_EXECS submissions in one go so that we get the inner
     * most loop we can inside the driver.
     */
    VkCommandBuffer cmd_buffers[NUM_EXECS];
    for (unsigned i = 0; i < NUM_EXECS; i++)
        cmd_buffers[i] = t_cmd_buffer;

    uint64_t start = gettime_ns();

    qoQueueSubmit(t_queue, NUM_EXECS, cmd_buffers, VK_NULL_HANDLE);

    uint64_t end = gettime_ns();

    qoQueueWaitIdle(t_queue);

    logi("Called vkQueueSubmit with %u buffers %u times, took %uus (%uus each)",
         buffer_count, NUM_EXECS, (unsigned)((end - start) / 1000),
         (unsigned)((end - start) / (1000 * NUM_EXECS)));
}

static void
test_queue_submit(void)
{
    // Make some small buffers
    VkBuffer buffers[MAX_BUFFER_COUNT];
    unsigned buffer_count = 0;

    for (unsigned i = MIN_BUFFER_COUNT; i <= MAX_BUFFER_COUNT; i *= 2) {
        while (buffer_count < i) {
            VkBuffer buffer = qoCreateBuffer(t_device, .size = BUFFER_SIZE);
            VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer);
            vkBindBufferMemory(t_device, buffer, mem, 0);
            buffers[buffer_count++] = buffer;
        }
        test_queue_submit_variable(buffer_count, buffers);
    }
}

test_define {
    .name = "bench.queue-submit",
    .start = test_queue_submit,
    .no_image = true,
};
