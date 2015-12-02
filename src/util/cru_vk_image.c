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

#include <stdlib.h>

#include "qonos/qonos.h"
#include "tapi/t_data.h"
#include "util/xalloc.h"
#include "util/misc.h"
#include "util/log.h"

#include "cru_image.h"

typedef struct cru_vk_image cru_vk_image_t;

enum copy_direction {
    COPY_IMAGE_TO_BUFFER,
    COPY_BUFFER_TO_IMAGE,
};

struct cru_vk_image {
    cru_image_t cru_image;

    VkDevice vk_dev;
    VkQueue vk_queue;
    uint32_t vk_tmp_mem_type_index;

    struct {
        VkImage vk_image;
        VkImageAspectFlagBits vk_aspect;
        uint32_t miplevel;
        uint32_t array_slice;
    } target;


    struct {
        VkBuffer vk_buffer;
        VkDeviceMemory vk_mem;
        void *pixels;
        uint32_t access; ///< Mask of CRU_IMAGE_MAP_ACCESS_* .
    } map;
};

static void
cleanup_map(cru_vk_image_t *self)
{
    if (self->map.pixels)
        vkUnmapMemory(self->vk_dev, self->map.vk_mem);
    if (self->map.vk_mem != VK_NULL_HANDLE)
        vkFreeMemory(self->vk_dev, self->map.vk_mem, NULL);
    if (self->map.vk_buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(self->vk_dev, self->map.vk_buffer, NULL);

    self->map = (typeof(self->map)) {0};
}

/// Setup cru_vk_image::map.
static VkResult
setup_map(cru_vk_image_t *self)
{
    VkDevice dev = self->vk_dev;
    VkResult r = VK_SUCCESS;

    if (self->map.pixels)
        return VK_SUCCESS;

    const size_t buffer_size = self->cru_image.format_info->cpp *
                               self->cru_image.width *
                               self->cru_image.height;

    r = vkCreateBuffer(dev, &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = buffer_size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        },
        NULL,
        &self->map.vk_buffer);
    if (r != VK_SUCCESS)
        goto fail;

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(dev, self->map.vk_buffer, &mem_reqs);

    r = vkAllocateMemory(dev, &(VkMemoryAllocateInfo) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .memoryTypeIndex = self->vk_tmp_mem_type_index,
            .allocationSize = mem_reqs.size,
        },
        NULL,
        &self->map.vk_mem);
    if (r != VK_SUCCESS)
        goto fail;

    r = vkBindBufferMemory(dev, self->map.vk_buffer, self->map.vk_mem, 0);
    if (r != VK_SUCCESS)
        goto fail;

    r = vkMapMemory(dev, self->map.vk_mem, /*offset*/ 0, buffer_size,
                    /*flags*/ 0, &self->map.pixels);
    if (r != VK_SUCCESS)
        goto fail;

    return VK_SUCCESS;

fail:
    cleanup_map(self);
    return r;
}

static VkResult
copy(cru_vk_image_t *self, enum copy_direction dir)
{
    VkDevice dev = self->vk_dev;
    VkCommandPool cmd_pool = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkFence fence = {0};
    VkResult r = VK_SUCCESS;

    r = vkCreateCommandPool(dev, &(VkCommandPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        },
        NULL,
        &cmd_pool);

    r = vkCreateCommandBuffer(dev, &(VkCommandBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_CREATE_INFO,
            .commandPool = cmd_pool,
        },
        &cmd);
    if (r != VK_SUCCESS)
        goto cleanup;

    r = vkBeginCommandBuffer(cmd, &(VkCommandBufferBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        });
    if (r != VK_SUCCESS)
        goto cleanup;

    const VkBufferImageCopy region = {
        .bufferOffset = 0,
        .imageSubresource = {
            .aspectMask = self->target.vk_aspect,
            .mipLevel = self->target.miplevel,
            .baseArrayLayer = self->target.array_slice,
            .layerCount = 1,
        },
        .imageOffset = { .x = 0, .y = 0, .z = 0 },
        .imageExtent = {
            .width = self->cru_image.width,
            .height = self->cru_image.height,
            .depth = 1,
        },
    };

    switch (dir) {
    case COPY_IMAGE_TO_BUFFER:
        vkCmdCopyImageToBuffer(cmd, self->target.vk_image,
                               VK_IMAGE_LAYOUT_GENERAL, self->map.vk_buffer,
                               1, &region);
        break;
    case COPY_BUFFER_TO_IMAGE:
        vkCmdCopyBufferToImage(cmd, self->map.vk_buffer,
                               self->target.vk_image, VK_IMAGE_LAYOUT_GENERAL,
                               1, &region);
        break;
    }

    r = vkEndCommandBuffer(cmd);
    if (r != VK_SUCCESS)
        goto cleanup;

    r = vkCreateFence(dev, &(VkFenceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        },
        NULL,
        &fence);
    if (r != VK_SUCCESS)
        goto cleanup;

    r = vkQueueSubmit(self->vk_queue, 1, &cmd, fence);
    if (r != VK_SUCCESS)
        goto cleanup;

    r = vkWaitForFences(dev, 1, &fence, true, /*timeout*/ UINT64_MAX);
    if (r != VK_SUCCESS) {
        if (r == VK_TIMEOUT)
            logw("vkWaitForFences timed out!");
        goto cleanup;
    }

cleanup:
    if (fence != VK_NULL_HANDLE)
        vkDestroyFence(dev, fence, NULL);
    if (cmd)
        vkDestroyCommandBuffer(dev, cmd);
    if (cmd_pool)
        vkDestroyCommandPool(dev, cmd_pool, NULL);

    return r;
}

static uint8_t *
map_pixels(cru_image_t *_self, uint32_t access)
{
    cru_vk_image_t *self = (cru_vk_image_t *) _self;

    assert(!self->map.access);

    if (setup_map(self) != VK_SUCCESS)
        return NULL;

    if (access & CRU_IMAGE_MAP_ACCESS_READ) {
        if (copy(self, COPY_IMAGE_TO_BUFFER) != VK_SUCCESS)
            return NULL;
    }

    self->map.access = access;

    return self->map.pixels;
}

static bool
unmap_pixels(cru_image_t *_self)
{
    cru_vk_image_t *self = (cru_vk_image_t *) _self;
    bool r = false;

    assert(self->map.access);

    if (self->map.access & CRU_IMAGE_MAP_ACCESS_WRITE) {
        if (copy(self, COPY_BUFFER_TO_IMAGE) != VK_SUCCESS)
            goto done;
    }

    r = true;

done:
    self->map.access = 0;
    return r;
}

static void
destroy(cru_image_t *_self)
{
    if (!_self)
        return;

    cru_vk_image_t *self = (cru_vk_image_t *) _self;

    cleanup_map(self);
    free(self);
}

malloclike cru_image_t *
cru_image_from_vk_image(VkDevice dev, VkQueue queue, VkImage image,
                        VkFormat format, VkImageAspectFlagBits aspect,
                        uint32_t level0_width, uint32_t level0_height,
                        uint32_t miplevel, uint32_t array_slice,
                        uint32_t tmp_mem_type_index)
{
    cru_vk_image_t *self = xzalloc(sizeof(*self));

    if (!cru_image_init(&self->cru_image, CRU_IMAGE_TYPE_VULKAN, format,
                        cru_minify(level0_width, miplevel),
                        cru_minify(level0_height, miplevel),
                        /*read_only*/ false)) {
        goto fail;
    }

    self->vk_dev = dev;
    self->vk_queue = queue;
    self->vk_tmp_mem_type_index = tmp_mem_type_index;
    self->target.vk_image = image;
    self->target.vk_aspect = aspect;
    self->target.miplevel = miplevel;
    self->target.array_slice = array_slice;

    self->cru_image.destroy = destroy;
    self->cru_image.map_pixels = map_pixels;
    self->cru_image.unmap_pixels = unmap_pixels;

    return &self->cru_image;

fail:
    destroy(&self->cru_image);
    return NULL;
}
