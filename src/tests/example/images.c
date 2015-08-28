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

test_define {
    .name = "example.image.map-ref-image",
    .start = test_map_ref_image,
};

static void
test_copy_ref_image(void)
{
    const uint32_t width = t_width;
    const uint32_t height = t_height;

    uint32_t *copy_pixels = xmalloc(4 * width * height);
    t_cleanup_push_free(copy_pixels);

    cru_image_t *copy_image = t_new_cru_image_from_pixels(
            copy_pixels, VK_FORMAT_R8G8B8A8_UNORM, width, height);
    cru_image_copy(copy_image, t_ref_image());
    t_dump_image_f(copy_image, "actual.png");

    for (uint32_t i = 0; i < width * height; ++i) {
        t_assert(copy_pixels[i] == green);
    }

    t_pass();
}

test_define {
    .name = "example.image.copy-ref-image",
    .start = test_copy_ref_image,
};

static void
test_dump_seq_images(void)
{
    const uint32_t width = 16;
    const uint32_t height = 16;

    void *pixels = xmalloc(4 * width * height);
    t_cleanup_push_free(pixels);

    cru_image_t *img = t_new_cru_image_from_pixels(
        pixels, VK_FORMAT_R8G8B8A8_UNORM, width, height);

    // Dump a red, green, then blue image.
    for (int i = 0; i < 3; ++i) {
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t *rgba = pixels + 4 * (y * width + x);
                rgba[0] = i == 0 ? 0xff : 0x00;
                rgba[1] = i == 1 ? 0xff : 0x00;
                rgba[2] = i == 2 ? 0xff : 0x00;
                rgba[3] = 0xff;
            }
        }

        t_dump_seq_image(img);
    }

    t_pass();
}

test_define {
    .name = "example.image.dump-seq-images",
    .start = test_dump_seq_images,
    .no_image = true,
};
