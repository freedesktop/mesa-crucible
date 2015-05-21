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

const uint32_t green = 0xff00ff00; // little endian RGBA8

static void
test_map_ref_image(void)
{
    const uint32_t width = t_width;
    const uint32_t height = t_height;

    uint32_t *rgba = cru_image_map(t_ref_image(), CRU_IMAGE_MAP_ACCESS_READ);
    t_assert(rgba);

    for (uint32_t i = 0; i < width * height; ++i) {
        t_assert(rgba[i] == green);
    }

    t_assert(cru_image_unmap(t_ref_image()));
    t_pass();
}

cru_define_test {
    .name = "example.image.map-ref-image",
    .start = test_map_ref_image,
};

static void
test_copy_ref_image(void)
{
    const uint32_t width = t_width;
    const uint32_t height = t_height;

    uint32_t *copy_pixels = xmalloc(4 * width * height);
    t_cleanup_push(free, copy_pixels);

    cru_image_t *copy_image = cru_image_from_pixels(copy_pixels,
                                                    VK_FORMAT_R8G8B8A8_UNORM,
                                                    width, height);
    t_cleanup_push(copy_image);

    cru_image_copy(copy_image, t_ref_image());

    for (uint32_t i = 0; i < width * height; ++i) {
        t_assert(copy_pixels[i] == green);
    }

    t_pass();
}

cru_define_test {
    .name = "example.image.copy-ref-image",
    .start = test_copy_ref_image,
};
