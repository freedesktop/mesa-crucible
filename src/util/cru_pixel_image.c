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
#include <string.h>

#include "util/xalloc.h"

#include "cru_image.h"

typedef struct cru_pixel_image cru_pixel_image_t;

struct cru_pixel_image {
    cru_image_t image;

    uint8_t *restrict pixels;

    ///< Bitmask of `CRU_IMAGE_MAP_ACCESS_*`.
    uint32_t map_access;
};

static void
cru_pixel_image_destroy(cru_image_t *image)
{
    cru_pixel_image_t *kpix_image = (cru_pixel_image_t *) image;

    if (!kpix_image)
        return;

    free(kpix_image);
}

static uint8_t *
cru_pixel_image_map_pixels(cru_image_t *image, uint32_t access)
{
    cru_pixel_image_t *kpix_image = (cru_pixel_image_t *) image;

    assert(kpix_image->map_access == 0);
    assert(access != 0);

    kpix_image->map_access = access;
    return kpix_image->pixels;
}

static bool
cru_pixel_image_unmap_pixels(cru_image_t *image)
{
    cru_pixel_image_t *kpix_image = (cru_pixel_image_t *) image;

    assert(kpix_image->map_access != 0);
    kpix_image->map_access = 0;
    return true;
}

cru_image_t *
cru_image_from_pixels(void *restrict pixels,
                       VkFormat format,
                       uint32_t width, uint32_t height)
{
    cru_pixel_image_t *kpix_image = xmalloc(sizeof(*kpix_image));

    if (!cru_image_init(&kpix_image->image, CRU_IMAGE_TYPE_PIXELS,
                        format, width, height,
                        /*read_only*/ false)) {
        goto fail;
    }

    kpix_image->pixels = pixels;
    kpix_image->map_access = 0;

    kpix_image->image.destroy = cru_pixel_image_destroy;
    kpix_image->image.map_pixels = cru_pixel_image_map_pixels;
    kpix_image->image.unmap_pixels = cru_pixel_image_unmap_pixels;

    return &kpix_image->image;

fail:
    cru_pixel_image_destroy((cru_image_t *) kpix_image);
    return NULL;
}
