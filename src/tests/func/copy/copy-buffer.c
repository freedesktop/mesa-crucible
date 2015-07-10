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

#include <crucible/cru.h>

static void
test_large_copy(void)
{
    //  We'll test copying 1000k buffers
    const int buffer_size = 1024000;

    VkBuffer buffer1 = qoCreateBuffer(t_device, .size = buffer_size,
                                      .usage = VK_BUFFER_USAGE_GENERAL);
    VkBuffer buffer2 = qoCreateBuffer(t_device, .size = buffer_size,
                                      .usage = VK_BUFFER_USAGE_GENERAL);

    VkMemoryRequirements buffer_requirements =
       qoGetBufferMemoryRequirements(t_device, buffer1);

    const int memory_size = buffer_requirements.size * 2;

    VkDeviceMemory mem = qoAllocMemory(t_device,
        .allocationSize = memory_size,
        .memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *map = qoMapMemory(t_device, mem, 0, buffer_requirements.size * 2, 0);

    // Fill the first buffer_size of the memory with a pattern
    uint32_t *map32 = map;
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++)
        map32[i] = i;

    // Fill the rest with 0
    memset((char *)map + buffer_size, 0, memory_size - buffer_size);

    qoBindBufferMemory(t_device, buffer1, mem, 0);
    qoBindBufferMemory(t_device, buffer2, mem, buffer_requirements.size);

    VkCmdBuffer cmdBuffer = qoCreateCommandBuffer(t_device);
    qoBeginCommandBuffer(cmdBuffer);

    vkCmdPipelineBarrier(cmdBuffer, VK_WAIT_EVENT_TOP_OF_PIPE, 0, 2,
        (const void * []) {
            &(VkBufferMemoryBarrier) {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .outputMask = VK_MEMORY_OUTPUT_HOST_WRITE_BIT,
                .inputMask = VK_MEMORY_INPUT_TRANSFER_BIT,
                .buffer = buffer1,
                .offset = 0,
                .size = buffer_size,
            },
            &(VkBufferMemoryBarrier) {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .outputMask = VK_MEMORY_OUTPUT_HOST_WRITE_BIT,
                .buffer = buffer2,
                .offset = 0,
                .size = buffer_size,
            },
        });

    vkCmdCopyBuffer(cmdBuffer, buffer1, buffer2, 1,
        &(VkBufferCopy) {
            .srcOffset = 0,
            .destOffset = 0,
            .copySize = buffer_size,
        });

    vkCmdPipelineBarrier(cmdBuffer, VK_WAIT_EVENT_TOP_OF_PIPE, 0, 1,
        (const void * []) {
            &(VkBufferMemoryBarrier) {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .outputMask = VK_MEMORY_OUTPUT_TRANSFER_BIT,
                .inputMask = VK_MEMORY_INPUT_HOST_READ_BIT,
                .buffer = buffer2,
                .offset = 0,
                .size = buffer_size,
            },
        });

    qoEndCommandBuffer(cmdBuffer);
    qoQueueSubmit(t_queue, 1, &cmdBuffer, 0);
    vkQueueWaitIdle(t_queue);

    uint32_t *map32_2 = map + buffer_requirements.size;
    for (unsigned i = 0; i < buffer_size / sizeof(*map32); i++) {
        t_assertf(map32[i] == map32_2[i],
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, map32_2[i], map32[i]);
    }
}

cru_define_test {
    .name = "func.copy.copy-buffer.large",
    .start = test_large_copy,
    .no_image = true,
};
