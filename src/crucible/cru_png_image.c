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

#include <alloca.h>
#include <stdio.h>
#include <string.h>

#include <libpng16/png.h>

#include <crucible/cru_log.h>
#include <crucible/xalloc.h>

#include "cru_image.h"

typedef struct cru_png_image cru_png_image_t;

struct cru_png_image {
    cru_image_t image;

    char *filename;
    FILE *file;

    /// Value is one of PNG_COLOR_TYPE_*.
    uint8_t png_color_type;

    struct {
        /// When mapping the cru_png_image for reading, we decode the file into
        /// this pixel array.
        uint8_t *pixels;

        /// Concrete type is cru_pixel_image_t.  The image's backing storage is
        /// `cru_png_image::map.pixels`.
        cru_image_t *pixel_image;

        /// Bitmask of `CRU_IMAGE_MAP_ACCESS_*`.
        uint32_t access;
    } map;
};

static bool
cru_png_image_read_file_info(cru_png_image_t *kpng_image)
{
    bool result = false;
    png_structp png_reader = NULL;
    png_infop png_info = NULL;

    assert(kpng_image->file >= 0);
    rewind(kpng_image->file);

    // FINISHME: Error callbacks for libpng
    png_reader = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                       NULL, NULL, NULL);
    if (!png_reader)
        goto fail_create_png_reader;

    png_info = png_create_info_struct(png_reader);
    if (!png_info)
        goto fail_create_png_info;

    png_init_io(png_reader, kpng_image->file);
    png_read_info(png_reader, png_info);

    uint8_t bit_depth = png_get_bit_depth(png_reader, png_info);
    uint8_t color_type = png_get_color_type(png_reader, png_info);

    if (bit_depth != 8) {
        cru_loge("png file must have bit depth 8 but has bit depth %u (filename=%s)",
                bit_depth, kpng_image->filename);
        goto fail_format;
    }

    if (color_type != PNG_COLOR_TYPE_RGB_ALPHA &&
       color_type != PNG_COLOR_TYPE_RGB) {
        cru_loge("png file must have color type RGB or RGBA (filename=%s)",
                kpng_image->filename);
        goto fail_format;
    }

    kpng_image->png_color_type = color_type;
    kpng_image->image.width = png_get_image_width(png_reader, png_info);
    kpng_image->image.height = png_get_image_height(png_reader, png_info);

    result = true;

fail_create_png_info:
    png_destroy_read_struct(&png_reader, &png_info, NULL);
fail_create_png_reader:
fail_format:
    return result;
}

bool
cru_png_image_copy_to_pixels(cru_image_t *image, cru_image_t *dest)
{
    cru_png_image_t *kpng_image;

    bool result = false;
    png_structp png_reader = NULL;
    png_infop png_info = NULL;

    const uint32_t width = image->width;
    const uint32_t height = image->height;
    const uint32_t stride = width * cru_image_pixel_size;
    uint8_t *dest_pixels = NULL;
    uint8_t *dest_rows[height];

    assert(image->type == CRU_IMAGE_TYPE_PNG);
    kpng_image = (cru_png_image_t *) image;

    assert(!dest->read_only);
    dest_pixels = dest->map_pixels(dest, CRU_IMAGE_MAP_ACCESS_WRITE);
    if (!dest_pixels)
        return false;

    for (uint32_t y = 0; y < height; ++y) {
        dest_rows[y] = dest_pixels + y * stride;
    }

    // FINISHME: Error callbacks for libpng
    png_reader = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                       NULL, NULL, NULL);
    if (!png_reader) {
        cru_loge("failed to create png reader");
        goto fail_create_png_reader;
    }

    png_info = png_create_info_struct(png_reader);
    if (!png_info) {
        cru_loge("failed to create png reader info");
        goto fail_create_png_info;
    }

    rewind(kpng_image->file);
    png_init_io(png_reader, kpng_image->file);
    png_read_info(png_reader, png_info);

    // Transform the file's pixel format to the crucible image's pixel format.
    assert(kpng_image->image.format == VK_FORMAT_R8G8B8A8_UNORM);
    switch (kpng_image->png_color_type) {
    case PNG_COLOR_TYPE_RGB:
        png_set_add_alpha(png_reader, UINT32_MAX, PNG_FILLER_AFTER);
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        // No transformation needed.
        break;
    default:
        assert(!"internal error");
        break;
    }

    png_read_rows(png_reader, dest_rows, NULL, height);
    png_read_end(png_reader, NULL);

    result = true;

fail_create_png_info:

    png_destroy_read_struct(&png_reader, &png_info, NULL);
fail_create_png_reader:

    if (!dest->unmap_pixels(dest)) {
        cru_loge("failed to unmap pixel image");
        abort();
    }

    return result;
}

static uint8_t *
cru_png_image_map_pixels(cru_image_t *image, uint32_t access)
{
    cru_png_image_t *kpng_image = (cru_png_image_t *) image;

    const uint32_t width = image->width;
    const uint32_t height = image->height;
    cru_image_t *pixel_image = NULL;
    void *pixels = NULL;

    assert(kpng_image->map.access == 0);
    assert(access != 0);

    if (access & CRU_IMAGE_MAP_ACCESS_WRITE) {
        cru_loge("crucible png images are read-only; cannot image for writing");
        goto fail;
    }

    if (kpng_image->map.pixels) {
        // We've already mapped the image. Since cru_png_image is read-only, we
        // keep the map alive for the image's lifetime.
        return kpng_image->map.pixels;
    }

    pixels = xmalloc(width * height * cru_image_pixel_size);

    pixel_image = cru_image_from_pixels(pixels, image->format, width, height);
    if (!pixel_image)
        goto fail;

    if (!cru_png_image_copy_to_pixels(&kpng_image->image, pixel_image))
        goto fail;

    kpng_image->map.access = access;
    kpng_image->map.pixels = pixels;
    kpng_image->map.pixel_image = pixel_image;

    return pixels;

fail:
    cru_image_release(pixel_image);
    free(pixels);
    return NULL;
}

static bool
cru_png_image_unmap_pixels(cru_image_t *image)
{
    cru_png_image_t *kpng_image = (cru_png_image_t *) image;

    // PNG images are always read-only.
    assert(!(kpng_image->map.access & CRU_IMAGE_MAP_ACCESS_WRITE));
    assert(kpng_image->map.pixel_image != NULL);
    kpng_image->map.access = 0;

    return true;
}

static void
cru_png_image_destroy(cru_image_t *image)
{
    cru_png_image_t *kpng_image = (cru_png_image_t *) image;

    if (!kpng_image)
        return;

    if (kpng_image->map.pixel_image)
        cru_image_release(kpng_image->map.pixel_image);

    assert(kpng_image->file >= 0);
    fclose(kpng_image->file);

    free(kpng_image->map.pixels);
    free(kpng_image->filename);
    free(kpng_image);
}

cru_image_t *
cru_png_image_load_file(const char *filename)
{
    cru_png_image_t *kpng_image = xmalloc(sizeof(*kpng_image));

    if (!cru_image_init(&kpng_image->image,
                        CRU_IMAGE_TYPE_PNG,
                        VK_FORMAT_R8G8B8A8_UNORM,
                        /*width, height*/ 1, 1,
                        /*read_only*/ true)) {
        goto fail_image_init;
    }

    kpng_image->map.access = 0;
    kpng_image->map.pixels = NULL;
    kpng_image->map.pixel_image = NULL;

    kpng_image->filename = cru_image_get_abspath(filename);
    if (!kpng_image->filename)
        goto fail_filename;

    kpng_image->file = fopen(kpng_image->filename, "rb");
    if (!kpng_image->file) {
        cru_loge("failed to open file for reading: %s", filename);
        goto fail_fopen;
    }

    if (!cru_png_image_read_file_info(kpng_image))
        goto fail_read_file;

    kpng_image->image.destroy = cru_png_image_destroy;
    kpng_image->image.map_pixels = cru_png_image_map_pixels;
    kpng_image->image.unmap_pixels = cru_png_image_unmap_pixels;

    return &kpng_image->image;

fail_read_file:
    fclose(kpng_image->file);
fail_fopen:
    free(kpng_image->filename);
fail_filename:
fail_image_init:
    free(kpng_image);
    return NULL;
}
