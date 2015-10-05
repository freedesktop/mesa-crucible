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

#define WIDTH 256
#define HEIGHT 256

struct src {
    VkImage vk_image;
};

struct dest {
    VkCmdBuffer cmd;
    VkBuffer buffer;
    cru_image_t *cru_image;
};

static const VkBufferImageCopy copy = {
    .bufferOffset = 0,
    .imageSubresource = {
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .mipLevel = 0,
        .arraySlice = 0,
    },
    .imageOffset = { .x = 0, .y = 0, .z = 0 },
    .imageExtent = {
        .width = WIDTH,
        .height = HEIGHT,
        .depth = 1,
    },
};

static void
setup_src(struct src *src)
{
    const uint32_t width = t_width;
    const uint32_t height = t_height;

    VkImage image = qoCreateImage(t_device,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = {
           .width = width,
           .height = height,
           .depth = 1,
       },
       .tiling = VK_IMAGE_TILING_LINEAR,
       .usage = 0);

    VkMemoryRequirements mem_reqs =
       qoGetImageMemoryRequirements(t_device, image);

    VkDeviceMemory mem = qoAllocMemoryFromRequirements(t_device, &mem_reqs,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    void *vk_map = qoMapMemory(t_device, mem, 0, mem_reqs.size, 0);

    // Copy refeference image into the Vulkan image.
    void *ref_map = cru_image_map(t_ref_image(), CRU_IMAGE_MAP_ACCESS_READ);
    t_assert(ref_map);
    memcpy(vk_map, ref_map, 4 * width * height);
    t_assert(cru_image_unmap(t_ref_image()));

    qoBindImageMemory(t_device, image, mem, /*offset*/ 0);

    src->vk_image = image;
}

static void
setup_dest(struct dest *dest)
{
    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device, t_cmd_pool);

    vkBeginCommandBuffer(cmd,
        &(VkCmdBufferBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO,
            .flags = 0,
        });

    size_t buffer_size = 4 * t_width * t_height;

    VkBuffer buffer = qoCreateBuffer(t_device, .size = buffer_size);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    void *map = qoMapMemory(t_device, mem, /*offset*/ 0,
                            buffer_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, buffer, mem, 0);

    cru_image_t *cru_image = t_new_cru_image_from_pixels(map,
            VK_FORMAT_R8G8B8A8_UNORM, t_width, t_height);

    dest->cmd = cmd;
    dest->buffer = buffer;
    dest->cru_image = cru_image;
}

static void
compare_images(struct src *src, struct dest *dest1, struct dest *dest2)
{
    t_dump_seq_image(dest1->cru_image);
    t_dump_seq_image(dest2->cru_image);

    t_assert(cru_image_compare(t_ref_image(), dest1->cru_image));
    t_assert(cru_image_compare(t_ref_image(), dest2->cru_image));

    t_pass();
}

static void
test_end1_submit1_end2_submit2(void)
{
    struct src src;
    struct dest dest1;
    struct dest dest2;

    setup_src(&src);
    setup_dest(&dest1);
    setup_dest(&dest2);

    vkCmdCopyImageToBuffer(dest1.cmd, src.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                           dest1.buffer, 1, &copy);
    qoEndCommandBuffer(dest1.cmd);
    qoQueueSubmit(t_queue, 1, &dest1.cmd, QO_NULL_FENCE);
    vkQueueWaitIdle(t_queue);

    vkCmdCopyImageToBuffer(dest2.cmd, src.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                           dest2.buffer, 1, &copy);
    qoEndCommandBuffer(dest2.cmd);
    qoQueueSubmit(t_queue, 1, &dest2.cmd, QO_NULL_FENCE);
    vkQueueWaitIdle(t_queue);

    compare_images(&src, &dest1, &dest2);
}

static void
test_end1_end2_submit1_submit2(void)
{
    struct src src;
    struct dest dest1;
    struct dest dest2;

    setup_src(&src);
    setup_dest(&dest1);
    setup_dest(&dest2);

    vkCmdCopyImageToBuffer(dest1.cmd, src.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                           dest1.buffer, 1, &copy);
    qoEndCommandBuffer(dest1.cmd);

    vkCmdCopyImageToBuffer(dest2.cmd, src.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                           dest2.buffer, 1, &copy);
    qoEndCommandBuffer(dest2.cmd);

    qoQueueSubmit(t_queue, 1, &dest1.cmd, QO_NULL_FENCE);
    qoQueueSubmit(t_queue, 1, &dest2.cmd, QO_NULL_FENCE);
    vkQueueWaitIdle(t_queue);

    compare_images(&src, &dest1, &dest2);
}

test_define {
    .name = "func.interleaved-cmd-buffers.end1-submit1-end2-submit2",
    .image_filename = "func.interleaved-cmd-buffers.ref.png",
    .start = test_end1_submit1_end2_submit2,
};

test_define {
    .name = "func.interleaved-cmd-buffers.end1-end2-submit1-submit2",
    .image_filename = "func.interleaved-cmd-buffers.ref.png",
    .start = test_end1_end2_submit1_submit2,
};
