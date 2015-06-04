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

    VkImage image;
    vkCreateImage(t_device,
        &(VkImageCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = {
               .width = width,
               .height = height,
               .depth = 1,
           },
           .mipLevels = 1,
           .arraySize = 1,
           .samples = 1,
           .tiling = VK_IMAGE_TILING_LINEAR,
           .usage = VK_IMAGE_USAGE_GENERAL,
           .flags = 0,
       },
       &image);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_IMAGE, image);

    VkMemoryRequirements mem_reqs =
       qoImageGetMemoryRequirements(t_device, image);

    VkDeviceMemory mem = qoAllocMemory(t_device,
                                       .allocationSize = mem_reqs.size);
    void *vk_map = qoMapMemory(t_device, mem, 0, mem_reqs.size, 0);

    // Copy refeference image into the Vulkan image.
    void *ref_map = cru_image_map(t_ref_image(), CRU_IMAGE_MAP_ACCESS_READ);
    t_assert(ref_map);
    memcpy(vk_map, ref_map, 4 * width * height);
    t_assert(cru_image_unmap(t_ref_image()));

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_IMAGE, image,
                            /*allocationIndex*/ 0, mem,
                            /*offset*/ 0);

    src->vk_image = image;
}

static void
setup_dest(struct dest *dest)
{
    VkCmdBuffer cmd;
    vkCreateCommandBuffer(t_device,
        &(VkCmdBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO,
            .queueNodeIndex = 0,
            .flags = 0,
         },
        &cmd);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_COMMAND_BUFFER, cmd);

    vkBeginCommandBuffer(cmd,
        &(VkCmdBufferBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO,
            .flags = 0,
        });

    VkBuffer buffer = qoCreateBuffer(t_device, .size = 4 * t_width * t_height,
                                     .usage = VK_BUFFER_USAGE_GENERAL);

    VkMemoryRequirements mem_reqs =
       qoBufferGetMemoryRequirements(t_device, buffer);

    VkDeviceMemory mem = qoAllocMemory(t_device,
                                       .allocationSize = mem_reqs.size);
    void *map = qoMapMemory(t_device, mem, 0, mem_reqs.size, 0);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER, buffer,
                            /*index*/ 0, mem, 0);

    cru_image_t *cru_image = cru_image_from_pixels(map, VK_FORMAT_R8G8B8A8_UNORM,
                                                   t_width, t_height);
    t_assert(cru_image);
    t_cleanup_push(cru_image);

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
    vkEndCommandBuffer(dest1.cmd);
    vkQueueSubmit(t_queue, 1, &dest1.cmd, 0);
    vkQueueWaitIdle(t_queue);

    vkCmdCopyImageToBuffer(dest2.cmd, src.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                           dest2.buffer, 1, &copy);
    vkEndCommandBuffer(dest2.cmd);
    vkQueueSubmit(t_queue, 1, &dest2.cmd, 0);
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
    vkEndCommandBuffer(dest1.cmd);

    vkCmdCopyImageToBuffer(dest2.cmd, src.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                           dest2.buffer, 1, &copy);
    vkEndCommandBuffer(dest2.cmd);

    vkQueueSubmit(t_queue, 1, &dest1.cmd, 0);
    vkQueueSubmit(t_queue, 1, &dest2.cmd, 0);
    vkQueueWaitIdle(t_queue);

    compare_images(&src, &dest1, &dest2);
}

cru_define_test {
    .name = "func.interleaved-cmd-buffers.end1-submit1-end2-submit2",
    .image_filename = "func.interleaved-cmd-buffers.ref.png",
    .start = test_end1_submit1_end2_submit2,
};

cru_define_test {
    .name = "func.interleaved-cmd-buffers.end1-end2-submit1-submit2",
    .image_filename = "func.interleaved-cmd-buffers.ref.png",
    .start = test_end1_end2_submit1_submit2,
};
