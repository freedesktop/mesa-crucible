// Copyright 2018 Red Hat
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

typedef struct test_params test_params_t;

struct copy_buffer_test {
    VkBuffer buffer1;
    VkBuffer buffer2;
    VkDeviceMemory mem;
    VkMemoryRequirements total_buffer_reqs;
};

#define DEF_TEST_MQ(tname)                                           \
    test_define {                                                    \
        .name = "func.buffer."#tname,                                \
        .start = test_transfer_##tname,                              \
        .no_image = true,                                            \
        .queue_setup = QUEUE_SETUP_TRANSFER,                         \
    };

static void
create_sized_buffers(struct copy_buffer_test *buf_test, int buffer_size)
{
    buf_test->buffer1 = qoCreateBuffer(t_device, .size = buffer_size);
    buf_test->buffer2 = qoCreateBuffer(t_device, .size = buffer_size);

    buf_test->total_buffer_reqs =
       qoGetBufferMemoryRequirements(t_device, buf_test->buffer1);

    buf_test->total_buffer_reqs.size *= 2;

    buf_test->mem = qoAllocMemoryFromRequirements(t_device,
                                                  &buf_test->total_buffer_reqs,
                                                  .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void *map;
    vkMapMemory(t_device, buf_test->mem, 0, buf_test->total_buffer_reqs.size, 0, &map);

    // Fill the first buffer_size of the memory with a pattern
    uint32_t *map32 = map;
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++)
        map32[i] = i;

    // Fill the rest with 0xdeadbeef
    uint32_t *map32_2 = map + (buf_test->total_buffer_reqs.size / 2);
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++)
        map32_2[i] = 0xdeadbeef;

    qoBindBufferMemory(t_device, buf_test->buffer1, buf_test->mem, 0);
    qoBindBufferMemory(t_device, buf_test->buffer2, buf_test->mem, buf_test->total_buffer_reqs.size / 2);
    vkUnmapMemory(t_device, buf_test->mem);
}

static void
check_copy_buffer_result(struct copy_buffer_test *buf_test, int buffer_size, uint32_t offset)
{
    void *map;
    vkMapMemory(t_device, buf_test->mem, 0, buf_test->total_buffer_reqs.size, 0, &map);
    uint32_t *map32 = map;
    uint32_t *map32_2 = map + (buf_test->total_buffer_reqs.size / 2);
    uint32_t offset_dw = offset / sizeof(*map32);
    for (unsigned i = 0; i < offset_dw; i++) {
        t_assertf(map32_2[i] == 0xdeadbeef,
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, map32_2[i], map32[i]);
    }
    for (unsigned i = offset_dw; i < buffer_size / sizeof(*map32); i++) {
        t_assertf(map32[i] == map32_2[i],
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, map32_2[i], map32[i]);
    }
    vkUnmapMemory(t_device, buf_test->mem);
}

static void
copy_buffers(struct copy_buffer_test *buf_test, int buffer_size, bool two_regions)
{
    VkCommandBuffer cmdBuffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmdBuffer);

    vkCmdPipelineBarrier(cmdBuffer, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, false,
                         0, NULL, 2,
        (VkBufferMemoryBarrier[]) {
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                .buffer = buf_test->buffer1,
                .offset = 0,
                .size = buffer_size,
            },
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .buffer = buf_test->buffer2,
                .offset = 0,
                .size = buffer_size,
            },
        }, 0, NULL);

    VkBufferCopy regions[2];
    regions[0].srcOffset = 0;
    regions[0].dstOffset = 0;
    regions[0].size = buffer_size;
    if (two_regions) {
        regions[0].size = buffer_size / 2;
        regions[1].srcOffset = regions[0].size;
        regions[1].dstOffset = regions[0].size;
        regions[1].size = regions[0].size;
    }
    vkCmdCopyBuffer(cmdBuffer, buf_test->buffer1, buf_test->buffer2, two_regions ? 2 : 1,
                    regions);

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, true,
                         0, NULL, 1,
        &(VkBufferMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
            .buffer = buf_test->buffer2,
            .offset = 0,
            .size = buffer_size,
        }, 0, NULL);
    qoEndCommandBuffer(cmdBuffer);

    qoQueueSubmit(t_queue, 1, &cmdBuffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);
}

static void
test_transfer_copy_buffer(void)
{
    struct copy_buffer_test test;

    // Start testing 1000k buffers
    const int buffer_size = 1024000;

    create_sized_buffers(&test, buffer_size);

    copy_buffers(&test, buffer_size, false);

    check_copy_buffer_result(&test, buffer_size, 0);
}

DEF_TEST_MQ(copy_buffer)

static void
test_transfer_copy_buffer_amd_limits(void)
{
    struct copy_buffer_test test;
    /* AMD has a limit on transfer size at 0x3fffe0 */
    const int buffer_size = 0x3fff00 + 10;
    create_sized_buffers(&test, buffer_size);

    copy_buffers(&test, buffer_size, false);

    check_copy_buffer_result(&test, buffer_size, 0);
}

DEF_TEST_MQ(copy_buffer_amd_limits)

static void
test_transfer_copy_buffer_two_regions(void)
{
    struct copy_buffer_test test;
    const int buffer_size = 1024000;
    create_sized_buffers(&test, buffer_size);

    copy_buffers(&test, buffer_size, true);

    check_copy_buffer_result(&test, buffer_size, 0);
}

DEF_TEST_MQ(copy_buffer_two_regions)

struct fill_buffer_test {
    VkBuffer buffer1;
    VkDeviceMemory mem;
    VkMemoryRequirements total_buffer_reqs;
};

static void
create_fill_buffer(struct fill_buffer_test *buf_test, int buffer_size)
{
    buf_test->buffer1 = qoCreateBuffer(t_device, .size = buffer_size);

    buf_test->total_buffer_reqs =
       qoGetBufferMemoryRequirements(t_device, buf_test->buffer1);

    buf_test->mem = qoAllocMemoryFromRequirements(t_device,
                                                  &buf_test->total_buffer_reqs,
                                                  .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void *map;
    vkMapMemory(t_device, buf_test->mem, 0, buf_test->total_buffer_reqs.size, 0, &map);

    // Fill the first buffer_size of the memory with a pattern
    uint32_t *map32 = map;
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++)
        map32[i] = i;

    qoBindBufferMemory(t_device, buf_test->buffer1, buf_test->mem, 0);
    vkUnmapMemory(t_device, buf_test->mem);
}

static void
fill_buffer(struct fill_buffer_test *buf_test, int buffer_size, uint32_t offset, uint32_t fill_val, bool whole_size)
{
    uint32_t size = buffer_size - offset;
    VkCommandBuffer cmdBuffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmdBuffer);

    vkCmdPipelineBarrier(cmdBuffer, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, false,
                         0, NULL, 1,
        (VkBufferMemoryBarrier[]) {
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .buffer = buf_test->buffer1,
                .offset = offset,
                .size = size,
            },
        }, 0, NULL);

    vkCmdFillBuffer(cmdBuffer, buf_test->buffer1, offset, whole_size ? VK_WHOLE_SIZE : buffer_size - offset, fill_val);

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, true,
                         0, NULL, 1,
        &(VkBufferMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
            .buffer = buf_test->buffer1,
            .offset = offset,
            .size = size,
        }, 0, NULL);
    qoEndCommandBuffer(cmdBuffer);

    qoQueueSubmit(t_queue, 1, &cmdBuffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);
}

static void
check_fill_buffer_result(struct fill_buffer_test *buf_test, int buffer_size, uint32_t offset, uint32_t fill_val)
{
    void *map;
    vkMapMemory(t_device, buf_test->mem, 0, buf_test->total_buffer_reqs.size, 0, &map);
    uint32_t *map32 = map;
    for (unsigned i = 0; i < offset / sizeof(*map32); i++) {
        t_assertf(map32[i] == i,
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, map32[i], i);
    }
    for (unsigned i = offset / sizeof(*map32); i < buffer_size / sizeof(*map32); i++) {
        t_assertf(map32[i] == fill_val,
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, map32[i], fill_val);
    }
    vkUnmapMemory(t_device, buf_test->mem);

}

static void
test_transfer_fill_buffer(void)
{
    struct fill_buffer_test test;
    const int buffer_size = 1024000;
    create_fill_buffer(&test, buffer_size);

    fill_buffer(&test, buffer_size, 0, 0xcafedead, false);
    check_fill_buffer_result(&test, buffer_size, 0, 0xcafedead);
}

DEF_TEST_MQ(fill_buffer)

static void
test_transfer_fill_buffer_with_small_offset(void)
{
    struct fill_buffer_test test;
    const int buffer_size = 1024000;
    create_fill_buffer(&test, buffer_size);

    fill_buffer(&test, buffer_size, 4, 0xcafedead, false);
    check_fill_buffer_result(&test, buffer_size, 4, 0xcafedead);
}

DEF_TEST_MQ(fill_buffer_with_small_offset)

static void
test_transfer_fill_buffer_with_small_offset_whole_size(void)
{
    struct fill_buffer_test test;
    const int buffer_size = 1024000;
    create_fill_buffer(&test, buffer_size);

    fill_buffer(&test, buffer_size, 4, 0xcafedead, true);
    check_fill_buffer_result(&test, buffer_size, 4, 0xcafedead);
}

DEF_TEST_MQ(fill_buffer_with_small_offset_whole_size)

static void
test_transfer_fill_buffer_with_large_offset(void)
{
    struct fill_buffer_test test;
    const int buffer_size = 1024000;
    create_fill_buffer(&test, buffer_size);

    fill_buffer(&test, buffer_size, buffer_size / 2, 0xcafedead, false);
    check_fill_buffer_result(&test, buffer_size, buffer_size / 2, 0xcafedead);
}

DEF_TEST_MQ(fill_buffer_with_large_offset)

static void
test_transfer_fill_buffer_amd_limits(void)
{
    struct fill_buffer_test test;
    const int buffer_size = (1ul << 22);
    create_fill_buffer(&test, buffer_size);

    fill_buffer(&test, buffer_size, 0, 0xcafedead, false);
    check_fill_buffer_result(&test, buffer_size, 0, 0xcafedead);
}

DEF_TEST_MQ(fill_buffer_amd_limits)

static void
update_buffer(struct copy_buffer_test *buf_test, int buffer_size, uint32_t offset)
{
    void *map;
    vkMapMemory(t_device, buf_test->mem, 0, buf_test->total_buffer_reqs.size, 0, &map);

    VkCommandBuffer cmdBuffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmdBuffer);

    vkCmdPipelineBarrier(cmdBuffer, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, false,
                         0, NULL, 2,
        (VkBufferMemoryBarrier[]) {
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .buffer = buf_test->buffer2,
                .offset = 0,
                .size = buffer_size,
            },
        }, 0, NULL);

    vkCmdUpdateBuffer(cmdBuffer, buf_test->buffer2, offset, buffer_size - offset, map + offset);

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, true,
                         0, NULL, 1,
        &(VkBufferMemoryBarrier) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
            .buffer = buf_test->buffer2,
            .offset = 0,
            .size = buffer_size,
        }, 0, NULL);
    qoEndCommandBuffer(cmdBuffer);

    qoQueueSubmit(t_queue, 1, &cmdBuffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    vkUnmapMemory(t_device, buf_test->mem);
}

static void
test_transfer_update_buffer_small(void)
{
    struct copy_buffer_test test;
    const int buffer_size = 1024000;
    create_sized_buffers(&test, buffer_size);

    update_buffer(&test, 4096, 0);
    check_copy_buffer_result(&test, 4096, 0);
}

DEF_TEST_MQ(update_buffer_small)

static void
test_transfer_update_buffer_max(void)
{
    struct copy_buffer_test test;
    const int buffer_size = 1024000;
    create_sized_buffers(&test, buffer_size);

    /* VK spec says 65536 is maximum update buffer size */
    update_buffer(&test, 65536, 0);
    check_copy_buffer_result(&test, 65536, 0);
}

DEF_TEST_MQ(update_buffer_max)

static void
test_transfer_update_buffer_offset(void)
{
    struct copy_buffer_test test;
    const int buffer_size = 1024000;
    create_sized_buffers(&test, buffer_size);

    /* VK spec says 65536 is maximum update buffer size */
    update_buffer(&test, 65536, 4096);
    check_copy_buffer_result(&test, 65536, 4096);
}

DEF_TEST_MQ(update_buffer_offset)
