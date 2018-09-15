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

#include "tapi/t.h"

static unsigned
bytes_to_unit_div(uint64_t val)
{
    if (val >= (1ull << 30))
        return (1ull << 30);
    if (val >= (1ull << 20))
        return (1ull << 20);
    if (val >= (1ull << 10))
        return (1ull << 10);
    return 1;
}

static const char *
bytes_to_unit_str(uint64_t val)
{
    if (val >= (1ull << 30))
        return "GiB";
    if (val >= (1ull << 20))
        return "MiB";
    if (val >= (1ull << 10))
        return "KiB";
    return "B";
}

static double
second_to_unit_mul(double value)
{
    if (value <= 1.0 / 1000000000.0)
        return 1000000000.0;
    if (value <= 1.0 / 1000000.0)
        return 1000000.0;
    if (value <= 1.0 / 1000.0)
        return 1000.0;
    return 1.0;
}

static const char *
second_to_unit_str(double value)
{
    if (value <= 1.0 / 1000000000.0)
        return "ns";
    if (value <= 1.0 / 1000000.0)
        return "us";
    if (value <= 1.0 / 1000.0)
        return "ms";
    return "s";
}

static void
test_large_copy(void)
{
    // Make 256MiB buffers to ensure we easily blow caches
    const unsigned buffer_size_log2 = 28;
    const uint64_t buffer_size = 1ull << buffer_size_log2;
    unsigned runs_per_size = 16;

    VkBuffer buffer1 = qoCreateBuffer(t_device, .size = buffer_size);
    VkBuffer buffer2 = qoCreateBuffer(t_device, .size = buffer_size);

    VkMemoryRequirements total_buffer_reqs =
       qoGetBufferMemoryRequirements(t_device, buffer1);

    total_buffer_reqs.size *= 2;

    VkDeviceMemory mem = qoAllocMemoryFromRequirements(t_device,
        &total_buffer_reqs,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *map = qoMapMemory(t_device, mem, 0, total_buffer_reqs.size, 0);

    // Fill the first buffer_size of the memory with a pattern
    uint32_t *map32 = map;
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++)
        map32[i] = i;

    // Fill the rest with 0xdeadbeef
    uint32_t *map32_2 = map + buffer_size;
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++)
        map32_2[i] = 0xdeadbeef;

    qoBindBufferMemory(t_device, buffer1, mem, 0);
    qoBindBufferMemory(t_device, buffer2, mem, total_buffer_reqs.size / 2);

    VkCommandBuffer cmd_buffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd_buffer);
    vkCmdPipelineBarrier(cmd_buffer, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, NULL, 2,
        (VkBufferMemoryBarrier[]) {
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                .buffer = buffer1,
                .offset = 0,
                .size = buffer_size,
            },
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .buffer = buffer2,
                .offset = 0,
                .size = buffer_size,
            },
        }, 0, NULL);
    qoEndCommandBuffer(cmd_buffer);
    qoQueueSubmit(t_queue, 1, &cmd_buffer, VK_NULL_HANDLE);

    VkQueryPool query = qoCreateQueryPool(t_device,
                                          .queryType = VK_QUERY_TYPE_TIMESTAMP,
                                          .queryCount = 2);

    for (unsigned s = 2; s <= buffer_size_log2; s++) {
        /* For smaller copies, we don't want to blow out our command
         * buffer, so take an average of the log2s of the sizes.
         */
        unsigned bytes_to_copy_log2 = (s + buffer_size_log2) / 2;
        uint64_t cmd_buffer_copy_size = 1ull << bytes_to_copy_log2;
        uint64_t single_copy_size = 1ull << s;

        cmd_buffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
        qoBeginCommandBuffer(cmd_buffer);

        vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            query, 0);

        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, 0, NULL, 1,
            &(VkBufferMemoryBarrier) {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_READ_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .buffer = buffer2,
                .offset = 0,
                .size = buffer_size,
            }, 0, NULL);

        for (uint64_t i = 0; i < cmd_buffer_copy_size; i += single_copy_size) {
            assert(buffer_size % single_copy_size == 0);
            uint64_t offset = i % buffer_size;
            vkCmdCopyBuffer(cmd_buffer, buffer1, buffer2, 1,
                &(VkBufferCopy) {
                    .srcOffset = offset,
                    .dstOffset = offset,
                    .size = single_copy_size,
                });
        }

        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, 0, NULL, 1,
            &(VkBufferMemoryBarrier) {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
                .buffer = buffer2,
                .offset = 0,
                .size = buffer_size,
            }, 0, NULL);

        vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                            query, 1);

        qoEndCommandBuffer(cmd_buffer);

        uint64_t bytes_copied = 0, time = 0;
        for (unsigned run = 0; run < runs_per_size; run++) {
            qoQueueSubmit(t_queue, 1, &cmd_buffer, VK_NULL_HANDLE);
            qoQueueWaitIdle(t_queue);

            uint64_t query_results[2];
            vkGetQueryPoolResults(t_device, query, 0, 2,
                                  sizeof(query_results), query_results,
                                  sizeof(query_results[0]),
                                  VK_QUERY_RESULT_64_BIT);

            bytes_copied += cmd_buffer_copy_size;
            time += query_results[1] - query_results[0];
        }

        double seconds =
            (time * (double)t_physical_dev_props->limits.timestampPeriod) /
            1000000000.0;
        double gbps = (bytes_copied / seconds) / (1ull << 30);

        logi("Copied %u%s of data in %u%s chunks, took %f%s (%f GiB/s)",
             (unsigned)(bytes_copied / bytes_to_unit_div(bytes_copied)),
             bytes_to_unit_str(bytes_copied),
             (unsigned)(single_copy_size / bytes_to_unit_div(single_copy_size)),
             bytes_to_unit_str(single_copy_size),
             (float)(seconds * second_to_unit_mul(seconds)),
             second_to_unit_str(seconds), gbps);
    }
}

test_define {
    .name = "bench.copy-buffer",
    .start = test_large_copy,
    .no_image = true,
};
