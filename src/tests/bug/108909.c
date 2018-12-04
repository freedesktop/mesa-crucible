// Copyright 2018 Intel Corporation
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

#include <inttypes.h>
#include <stdio.h>
#include "tapi/t.h"
#include "util/misc.h"

/* This is a test for https://bugs.freedesktop.org/show_bug.cgi?id=108909
 *
 * Ensure ordering of operations for between 3d pipeline and command
 * streamer on Intel HW.
 */

static void
test(void)
{
    const uint64_t initialData[] = {
        0xdeaddeadbeef,
        0xdeaddeadbeef,
        0xdeaddeadbeef,
        0xdeaddeadbeef,
    };
    VkBuffer dataBuffer = qoCreateBuffer(t_device,
                                          .size = sizeof(initialData),
                                          .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkDeviceMemory dataMem = qoAllocBufferMemory(t_device, dataBuffer,
                                                  .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    memcpy(qoMapMemory(t_device, dataMem, /*offset*/ 0,
                       sizeof(initialData), /*flags*/ 0),
           initialData,
           sizeof(initialData));
    qoBindBufferMemory(t_device, dataBuffer, dataMem, /*offset*/ 0);

    VkBuffer resultBuffer = qoCreateBuffer(t_device,
                                            .size = sizeof(initialData),
                                            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkDeviceMemory resultMem = qoAllocBufferMemory(t_device, resultBuffer,
                                                   .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    qoBindBufferMemory(t_device, resultBuffer, resultMem, /*offset*/ 0);

    VkQueryPool pool = qoCreateQueryPool(t_device,
                                               .queryType = VK_QUERY_TYPE_TIMESTAMP,
                                               .queryCount = ARRAY_LENGTH(initialData));


    /* vkCmdCopyQueryPoolResults should be ordered with regard to vkCmdCopyBuffer. */
    vkCmdWriteTimestamp(t_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 0);
    vkCmdWriteTimestamp(t_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 1);
    vkCmdCopyBuffer(t_cmd_buffer, dataBuffer, resultBuffer,
                    1, &(VkBufferCopy){ .srcOffset = 0, .dstOffset = 0, .size = sizeof(initialData) });
    vkCmdWriteTimestamp(t_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 2);
    vkCmdWriteTimestamp(t_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 3);

    vkCmdCopyQueryPoolResults(t_cmd_buffer, pool, /*firstQuery*/ 0, /*queryCount*/ 4,
                              resultBuffer, /*dstBufferOffset*/ 0, sizeof(uint64_t),
                              VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    qoEndCommandBuffer(t_cmd_buffer);

    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    uint64_t *copiedResults = qoMapMemory(t_device, resultMem, /*offset*/ 0,
                                          sizeof(initialData), /*flags*/ 0);

    for (unsigned i = 0; i < ARRAY_LENGTH(initialData); i++) {
        printf("timestamp%i = 0x%016" PRIx64 "\n",
               i, copiedResults[i]);
        if (copiedResults[i] == initialData[i]) {
            printf("Got unexpected timestamp (index=%i) 0x%016" PRIx64 "\n",
                   i, copiedResults[i]);

            t_fail();
            return;
        }
    }

    t_pass();
}

test_define {
    .name = "bug.108909",
    .start = test,
    .no_image = true,
};
