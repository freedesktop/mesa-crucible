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

/* This is a test for https://bugs.freedesktop.org/show_bug.cgi?id=108911
 *
 * The wrong clear color was landing into the layer1+ of a multi-layered
 * multi-sampled image.
 */

static VkImage
create_image_and_bind_memory(const VkImageCreateInfo *info)
{
    VkImage img;
    VkDeviceMemory mem;

    vkCreateImage(t_device, info, NULL, &img);
    mem = qoAllocImageMemory(t_device, img,
                             .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    qoBindImageMemory(t_device, img, mem, 0);

    return img;
}

#define SIZE (8)

static void
test(void)
{
    VkRenderPass renderPass;
    VkImage fbImage, imageLayers[2];
    VkImageView viewLayers[2];
    VkFramebuffer fbLayers[2];
    VkBuffer bufferLayers[2];
    VkDeviceMemory bufferMem[2];

    renderPass = qoCreateRenderPass(t_device,
                                    .attachmentCount = 1,
                                    .pAttachments = &(VkAttachmentDescription) {
                                        .format = VK_FORMAT_R8G8B8A8_UNORM,
                                        .samples = VK_SAMPLE_COUNT_4_BIT,
                                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    },
                                    .subpassCount = 1,
                                    .pSubpasses = &(VkSubpassDescription) {
                                        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        .colorAttachmentCount = 1,
                                        .pColorAttachments = &(VkAttachmentReference) {
                                            .attachment = 0,
                                            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        },
                                    });

    fbImage = create_image_and_bind_memory(&(VkImageCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = (VkExtent3D) {
                .width = SIZE,
                .height = SIZE,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 2,
            .samples = VK_SAMPLE_COUNT_4_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = (VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                      VK_IMAGE_USAGE_SAMPLED_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
       });

    for (int i = 0; i < ARRAY_LENGTH(viewLayers); i++) {
        viewLayers[i] = qoCreateImageView(t_device,
                                          .image = fbImage,
                                          .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                                          .format = VK_FORMAT_R8G8B8A8_UNORM,
                                          .subresourceRange = (VkImageSubresourceRange) {
                                              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                              .baseMipLevel = 0,
                                              .levelCount = 1,
                                              .baseArrayLayer = i,
                                              .layerCount = 1,
                                           });
        fbLayers[i] = qoCreateFramebuffer(t_device,
                                          .renderPass = renderPass,
                                          .attachmentCount = 1,
                                          .pAttachments = &viewLayers[i],
                                          .width = SIZE, .height = SIZE, .layers = 1);

        imageLayers[i] = create_image_and_bind_memory(&(VkImageCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = (VkExtent3D) {
                .width = SIZE,
                .height = SIZE,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = (VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        });

        bufferLayers[i] = qoCreateBuffer(t_device, .size = 4 * SIZE * SIZE);
        bufferMem[i] = qoAllocBufferMemory(t_device, bufferLayers[i],
                                           .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkBindBufferMemory(t_device, bufferLayers[i], bufferMem[i], 0);
    }

    for (int i = 0; i < ARRAY_LENGTH(fbLayers); i++) {
        vkCmdBeginRenderPass(t_cmd_buffer,
                             &(VkRenderPassBeginInfo) {
                                 .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                 .renderPass = renderPass,
                                 .framebuffer = fbLayers[i],
                                 .renderArea = { { 0, 0 }, { SIZE, SIZE } },
                                 .clearValueCount = 1,
                                 .pClearValues = (VkClearValue[]) {
                                     { .color = { .float32 = { i == 0 ? 1.0 : 0.0, i == 1 ? 1.0 : 0.0, 0.0, 1.0 } } },
                                 },
                             }, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(t_cmd_buffer);
    }

    vkCmdPipelineBarrier(t_cmd_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, NULL,
                         0, NULL,
                         1, &(VkImageMemoryBarrier) {
                             .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                             .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                             .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                             .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             .image = fbImage,
                             .subresourceRange = (VkImageSubresourceRange) {
                                 .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 2,
                             },
                         });

    for (int i = 0; i < ARRAY_LENGTH(imageLayers); i++) {
        vkCmdPipelineBarrier(t_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, NULL,
                             0, NULL,
                             1, &(VkImageMemoryBarrier) {
                                 .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                 .srcAccessMask = 0,
                                 .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                 .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 .image = imageLayers[i],
                                 .subresourceRange = (VkImageSubresourceRange) {
                                     .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                 },
                             });

        vkCmdResolveImage(t_cmd_buffer,
                          fbImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          imageLayers[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          1, &(VkImageResolve) {
                              .srcSubresource = (VkImageSubresourceLayers) {
                                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .mipLevel = 0,
                                  .baseArrayLayer = i,
                                  .layerCount = 1,
                              },
                              .srcOffset = (VkOffset3D) { 0, 0, 0 },
                              .dstSubresource = (VkImageSubresourceLayers) {
                                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .mipLevel = 0,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1,
                              },
                              .dstOffset = (VkOffset3D) { 0, 0, 0 },
                              .extent = (VkExtent3D) { SIZE, SIZE, 1 } });
    }

    vkCmdPipelineBarrier(t_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, NULL,
                         0, NULL,
                         2, (VkImageMemoryBarrier[]) {
                             {
                                 .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                 .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                 .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                                 .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 .image = imageLayers[0],
                                 .subresourceRange = (VkImageSubresourceRange) {
                                     .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                 },
                             },
                             {
                                 .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                 .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                 .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                                 .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                 .image = imageLayers[1],
                                 .subresourceRange = (VkImageSubresourceRange) {
                                     .aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                 },
                             }
                         });

    for (int i = 0; i < ARRAY_LENGTH(imageLayers); i++) {
        vkCmdCopyImageToBuffer(t_cmd_buffer,
                               imageLayers[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, bufferLayers[i],
                               1, &(VkBufferImageCopy) {
                                   .bufferOffset = 0,
                                   .bufferRowLength = 0,
                                   .bufferImageHeight = 0,
                                   .imageSubresource = (VkImageSubresourceLayers) {
                                       .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                       .mipLevel = 0,
                                       .baseArrayLayer = 0,
                                       .layerCount = 1,
                                   },
                                   .imageOffset = (VkOffset3D) { 0, 0, 0 },
                                   .imageExtent = (VkExtent3D) { SIZE, SIZE, 1 } });
    }

    qoEndCommandBuffer(t_cmd_buffer);

    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    uint32_t *resolvedData[2] = {
        qoMapMemory(t_device, bufferMem[0], /*offset*/ 0,
                    SIZE * SIZE * 4, /*flags*/ 0),
        qoMapMemory(t_device, bufferMem[1], /*offset*/ 0,
                    SIZE * SIZE * 4, /*flags*/ 0)
    };

    for (int i = 0; i < SIZE * SIZE; i++) {
        if (resolvedData[0][i] != 0xff0000ff) {
            printf("unexpected clear color in layer0: got 0x%x expected 0xff0000ff\n",
                   resolvedData[0][i]);
            t_fail();
            return;
        }
        if (resolvedData[1][i] != 0xff00ff00) {
            printf("unexpected clear color in layer1: got 0x%x expected 0xff00ff00\n",
                   resolvedData[1][i]);
            t_fail();
            return;
        }
    }
    t_pass();
}

test_define {
    .name = "bug.108911",
    .start = test,
    .no_image = true,
};
