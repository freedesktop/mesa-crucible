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
#include "tapi/t_thread.h"
#include "util/cru_image.h"

malloclike cru_image_t *
t_new_cru_image_from_filename(const char *filename)
{
    t_thread_yield();

    cru_image_t *cimg = cru_image_from_filename(filename);
    if (!cimg)
        t_failf("%s: failed to create image", __func__);

    t_cleanup_push_cru_image(cimg);

    return cimg;
}

malloclike cru_image_array_t *
t_new_cru_image_array_from_filename(const char *filename)
{
    t_thread_yield();

    cru_image_array_t *cia = cru_image_array_from_filename(filename);
    if (!cia)
        t_failf("%s: failed to create image array", __func__);

    t_cleanup_push_cru_image_array(cia);

    return cia;
}

malloclike cru_image_t *
t_new_cru_image_from_vk_image(VkDevice dev, VkQueue queue, VkImage image,
                              VkFormat format, VkImageAspectFlagBits aspect,
                              uint32_t level0_width, uint32_t level0_height,
                              uint32_t miplevel, uint32_t array_slice)
{
    t_thread_yield();

    cru_image_t *cimg = cru_image_from_vk_image(t_device, queue, image,
            format, aspect, level0_width, level0_height, miplevel,
            array_slice, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!cimg)
        t_failf("%s: failed to create image", __func__);

    t_cleanup_push_cru_image(cimg);

    return cimg;
}

malloclike cru_image_t *
t_new_cru_image_from_pixels(void *restrict pixels, VkFormat format,
                            uint32_t width, uint32_t height)
{
    t_thread_yield();

    cru_image_t *cimg = cru_image_from_pixels(pixels, format, width, height);
    if (!cimg)
        t_failf("%s: failed to create image", __func__);

    t_cleanup_push_cru_image(cimg);

    return cimg;
}
