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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/misc.h"
#include "util/string.h"
#include "util/xalloc.h"

#include "cru_image.h"

typedef bool (*pixel_copy_func_t)(
                uint32_t width, uint32_t height,
                void *src, uint32_t src_x, uint32_t src_y, uint32_t src_stride,
                void *dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_stride);

/// Caller must free the returned string.
char *
cru_image_get_abspath(const char *filename)
{
    string_t abspath = STRING_INIT;
    const char *env = getenv("CRU_DATA_DIR");

    if (env && env[0]) {
        path_append_cstr(&abspath, env);
        path_append_cstr(&abspath, filename);
    } else {
        path_append(&abspath, cru_prefix_path());
        path_append_cstr(&abspath, "data");
        path_append_cstr(&abspath, filename);
    }

    return string_detach(&abspath);
}

void
cru_image_reference(cru_image_t *image)
{
    cru_refcount_get(&image->refcount);
}

void
cru_image_release(cru_image_t *image)
{
    if (cru_refcount_put(&image->refcount) > 0)
        return;

    image->destroy(image);
}

uint32_t
cru_image_get_width(cru_image_t *image)
{
    return image->width;
}

uint32_t
cru_image_get_height(cru_image_t *image)
{
    return image->height;
}

uint32_t
cru_image_get_pitch_bytes(cru_image_t *image)
{
    return image->pitch_bytes ? image->pitch_bytes : image->width * image->format_info->cpp;
}

VkFormat
cru_image_get_format(cru_image_t *image)
{
    return image->format_info->format;
}

bool
cru_image_init(cru_image_t *image, enum cru_image_type type,
               VkFormat format, uint32_t width, uint32_t height,
               bool read_only)
{
    cru_refcount_init(&image->refcount);

    image->format_info = cru_format_get_info(format);
    if (!image->format_info) {
        loge("cannot crucible image with VkFormat %d", format);
        return false;
    }

    if (width == 0) {
        loge("cannot create crucible image with zero width");
        return false;
    }

    if (height == 0) {
        loge("cannot create crucible image with zero width");
        return false;
    }

    image->width = width;
    image->height = height;
    image->type = type;
    image->read_only = read_only;
    image->pitch_bytes = 0;

    return true;
}

void
cru_image_set_pitch_bytes(cru_image_t *image, uint32_t pitch_bytes)
{
    image->pitch_bytes = pitch_bytes;
}

static bool
cru_image_check_compatible(const char *func,
                            cru_image_t *a, cru_image_t *b)
{
    if (a == b) {
        loge("%s: images are same", func);
        return false;
    }

    if (a->format_info->num_channels != b->format_info->num_channels) {
        loge("%s: image formats differ in number of channels", func);
        return false;
    }

    // FIXME: Reject images whose channel order differs.

    if (a->width != b->width) {
        loge("%s: image widths differ", func);
        return false;
    }

    if (a->height != b->height) {
        loge("%s: image heights differ", func);
        return false;
    }

    return true;
}

cru_image_t *
cru_image_from_filename(const char *_filename)
{
    string_t filename = STRING_INIT;
    cru_image_t *image = NULL;

    string_copy_cstr(&filename, _filename);

    if (string_endswith_cstr(&filename, ".png")) {
        image = cru_png_image_load_file(_filename);
    } else if (string_endswith_cstr(&filename, ".ktx")) {
        loge("loading ktx requires array in %s", _filename);
    } else {
        loge("unknown file extension in %s", _filename);
    }

    string_finish(&filename);

    return image;
}

bool
cru_image_write_file(cru_image_t *image, const char *_filename)
{
    string_t filename = STRING_INIT;
    bool res;

    string_copy_cstr(&filename, _filename);

    if (string_endswith_cstr(&filename, ".png")) {
        res = cru_png_image_write_file(image, &filename);
    } else {
        loge("unknown file extension in %s", _filename);
        res = false;
    }

    string_finish(&filename);

    return res;
}

static bool
copy_unorm8_to_uint8(
    uint32_t width, uint32_t height,
    void *src, uint32_t src_x, uint32_t src_y, uint32_t src_stride,
    void *dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_stride)
{
    const uint32_t cpp = 1;

    for (uint32_t y = 0; y < height; ++y) {
        void *src_row = src + ((src_y + y) * src_stride + src_x * cpp);
        void *dest_row = dest + ((dest_y + y) * dest_stride + dest_x * cpp);
        memcpy(dest_row, src_row, width * cpp);
    }

    return true;
}

static bool
copy_uint8_to_unorm8(
    uint32_t width, uint32_t height,
    void *src, uint32_t src_x, uint32_t src_y, uint32_t src_stride,
    void *dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_stride)
{
    return copy_unorm8_to_uint8(width, height,
                                src, src_x, src_y, src_stride,
                                dest, dest_x, dest_y, dest_stride);
}

static bool
copy_unorm8_to_f32(
    uint32_t width, uint32_t height,
    void *src, uint32_t src_x, uint32_t src_y, uint32_t src_stride,
    void *dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_stride)
{
    const uint32_t src_cpp = 1;
    const uint32_t dest_cpp = 4;

    for (uint32_t y = 0; y < height; ++y) {
        void *src_row = src + ((src_y + y) * src_stride +
                               src_x * src_cpp);

        void *dest_row = dest + ((dest_y + y) * dest_stride +
                                 dest_x * dest_cpp);

        for (uint32_t x = 0; x < width; ++x) {
            uint8_t *src_pix = src_row + (x * src_cpp);
            float *dest_pix = dest_row + (x * dest_cpp);

            dest_pix[0] = (float) src_pix[0] / UINT8_MAX;
        }
    }

    return true;
}

static bool
copy_f32_to_unorm8(
    uint32_t width, uint32_t height,
    void *src, uint32_t src_x, uint32_t src_y, uint32_t src_stride,
    void *dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_stride)
{
    const uint32_t src_cpp = 4;
    const uint32_t dest_cpp = 1;

    for (uint32_t y = 0; y < height; ++y) {
        void *src_row = src + ((src_y + y) * src_stride +
                               src_x * src_cpp);

        void *dest_row = dest + ((dest_y + y) * dest_stride +
                                 dest_x * dest_cpp);

        for (uint32_t x = 0; x < width; ++x) {
            float *src_pix = src_row + (x * src_cpp);
            uint8_t *dest_pix = dest_row + (x * dest_cpp);

            dest_pix[0] = UINT8_MAX * src_pix[0];
        }
    }

    return true;
}

static bool
copy_oneshot_memcpy(
    uint32_t width, uint32_t height,
    void *src, uint32_t src_x, uint32_t src_y, uint32_t src_stride,
    void *dest, uint32_t dest_x, uint32_t dest_y, uint32_t dest_stride)
{
    assert(src_x == 0);
    assert(src_y == 0);
    assert(dest_x == 0);
    assert(dest_y == 0);
    assert(src_stride == dest_stride);

    memcpy(dest, src, height * src_stride);

    return true;
}

static bool
cru_image_copy_pixels_to_pixels(cru_image_t *dest, cru_image_t *src)
{
    bool result = false;
    uint8_t *src_pixels = NULL;
    uint8_t *dest_pixels = NULL;
    pixel_copy_func_t copy_func;

    const VkFormat src_format = src->format_info->format;
    const VkFormat dest_format = dest->format_info->format;

    const uint32_t width = src->width;
    const uint32_t height = src->height;

    const uint32_t src_stride = cru_image_get_pitch_bytes(src);

    const uint32_t dest_stride = cru_image_get_pitch_bytes(dest);

    assert(!dest->read_only);

    // Extent equality is enforced by cru_image_check_compatible().
    assert(src->width == dest->width);
    assert(src->height == dest->height);

    src_pixels = src->map_pixels(src, CRU_IMAGE_MAP_ACCESS_READ);
    if (!src_pixels)
        goto fail_map_src_pixels;

    dest_pixels = dest->map_pixels(dest, CRU_IMAGE_MAP_ACCESS_WRITE);
    if (!dest_pixels)
        goto fail_map_dest_pixels;

    if (src->format_info == dest->format_info
        && src_stride == dest_stride) {
        copy_func = copy_oneshot_memcpy;
    } else if (src_format == VK_FORMAT_R8_UNORM &&
               dest_format == VK_FORMAT_D32_SFLOAT) {
        copy_func = copy_unorm8_to_f32;
    } else if (src_format == VK_FORMAT_D32_SFLOAT &&
               dest_format == VK_FORMAT_R8_UNORM) {
        copy_func = copy_f32_to_unorm8;
    } else if (src_format == VK_FORMAT_R8_UNORM &&
               dest_format == VK_FORMAT_S8_UINT) {
        copy_func = copy_unorm8_to_uint8;
    } else if (src_format == VK_FORMAT_S8_UINT &&
               dest_format == VK_FORMAT_R8_UNORM) {
        copy_func = copy_uint8_to_unorm8;
    } else {
        loge("%s: unsupported format combination", __func__);
        goto fail_no_copy_func;
    }

    result = copy_func(width, height,
                       src_pixels, 0, 0, src_stride,
                       dest_pixels, 0, 0, dest_stride);

fail_no_copy_func:

    // Check the result of unmapping the destination image because writeback
    // can fail during unmap.
    result &= dest->unmap_pixels(dest);
fail_map_dest_pixels:

    // Ignore the result of unmapping the source image because no writeback
    // occurs when unmapping a read-only map.
    src->unmap_pixels(src);
fail_map_src_pixels:

    return result;
}

bool
cru_image_copy(cru_image_t *dest, cru_image_t *src)
{
    if (!cru_image_check_compatible(__func__, dest, src))
        return false;

    if (dest->read_only) {
        loge("%s: dest is read only", __func__);
        return false;
    }

    // PNG images are always read-only.
    assert(dest->type != CRU_IMAGE_TYPE_PNG);

    if (src->type == CRU_IMAGE_TYPE_PNG)
        return cru_png_image_copy_to_pixels(src, dest);
    else
        return cru_image_copy_pixels_to_pixels(dest, src);
}

bool
cru_image_compare(cru_image_t *a, cru_image_t *b)
{
    if (a->width != b->width || a->height != b->height) {
        loge("%s: image dimensions differ", __func__);
        return false;
    }

    return cru_image_compare_rect(a, 0, 0, b, 0, 0, a->width, a->height);
}

bool
cru_image_compare_rect(cru_image_t *a, uint32_t a_x, uint32_t a_y,
                       cru_image_t *b, uint32_t b_x, uint32_t b_y,
                       uint32_t width, uint32_t height)
{
    bool result = false;
    void *a_map = NULL;
    void *b_map = NULL;

    if (a == b)
        return true;

    if (a->format_info != b->format_info &&

        !((a->format_info->format == VK_FORMAT_S8_UINT &&
           b->format_info->format == VK_FORMAT_R8_UNORM) ||

          (a->format_info->format == VK_FORMAT_R8_UNORM &&
           b->format_info->format == VK_FORMAT_S8_UINT))) {

        // Maybe one day we'll want to support more formats.
        loge("%s: image formats are incompatible", __func__);
        goto cleanup;
    }

    if (a_x + width > a->width || a_y + height > a->height ||
        b_x + width > b->width || b_y + height > b->height) {
        loge("%s: rect exceeds image dimensions", __func__);
        goto cleanup;
    }

    const uint32_t cpp = a->format_info->cpp;
    const uint32_t row_size = cpp * width;
    const uint32_t a_stride = cru_image_get_pitch_bytes(a);
    const uint32_t b_stride = cru_image_get_pitch_bytes(b);

    a_map = a->map_pixels(a, CRU_IMAGE_MAP_ACCESS_READ);
    if (!a_map)
        goto cleanup;

    b_map = b->map_pixels(b, CRU_IMAGE_MAP_ACCESS_READ);
    if (!b_map)
        goto cleanup;

    // FINISHME: Support a configurable tolerance.
    // FINISHME: Support dumping the diff to file.
    for (uint32_t y = 0; y < height; ++y) {
        const void *a_row = a_map + ((a_y + y) * a_stride + a_x * cpp);
        const void *b_row = b_map + ((b_y + y) * b_stride + b_x * cpp);

        if (memcmp(a_row, b_row, row_size) != 0) {
            loge("%s: diff found in row %u of rect", __func__, y);
            result = false;
            goto cleanup;
        }
    }

    result = true;

cleanup:
    if (a_map)
        a->unmap_pixels(a);

    if (b_map)
        b->unmap_pixels(b);

    return result;
}

void *
cru_image_map(cru_image_t *image, uint32_t access_mask)
{
    return image->map_pixels(image, access_mask);
}

bool
cru_image_unmap(cru_image_t *image)
{
    return image->unmap_pixels(image);
}

void
cru_image_array_reference(cru_image_array_t *ia)
{
    cru_refcount_get(&ia->refcount);
}

static void
cru_image_array_destroy(cru_image_array_t *ia)
{
    for (int i = 0; i < ia->num_images; i++)
        cru_image_release(ia->images[i]);
    free(ia->images);
    free(ia);
}

void
cru_image_array_release(cru_image_array_t *ia)
{
    if (cru_refcount_put(&ia->refcount) > 0)
        return;

    cru_image_array_destroy(ia);
}

cru_image_t *
cru_image_array_get_image(cru_image_array_t *ia, int index)
{
    return ia->images[index];
}

cru_image_array_t *
cru_image_array_from_filename(const char *_filename)
{
    string_t filename = STRING_INIT;
    cru_image_array_t *ia = NULL;

    string_copy_cstr(&filename, _filename);

    if (string_endswith_cstr(&filename, ".png")) {
        ia = calloc(1, sizeof(*ia));
        if (!ia)
            return NULL;
        cru_refcount_init(&ia->refcount);
        ia->num_images = 1;
        ia->images = calloc(1, sizeof(struct cru_image *));
        if (!ia->images) {
            free(ia);
            return NULL;
        }
        ia->images[0] = cru_png_image_load_file(_filename);
        if (!ia->images[0]) {
            free(ia->images);
            free(ia);
            return NULL;
        }
    } else if (string_endswith_cstr(&filename, ".ktx")) {
        ia = cru_ktx_image_array_load_file(_filename);
    }

    string_finish(&filename);
    return ia;
}
