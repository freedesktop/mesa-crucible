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

#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpng16/png.h>

#include <crucible/cru_log.h>
#include <crucible/cru_misc.h>
#include <crucible/string.h>
#include <crucible/xalloc.h>

#include "cru_image.h"

static regex_t filename_ext_regex;

static void
filename_regex_setup(void)
{
    int err;

    err = regcomp(&filename_ext_regex, "\\.[^.]\\+$", 0);
    if (err) {
        cru_loge("failed to compiled filename extension regex");
        abort();
    }
}

static const char *
filename_get_ext(const char *filename)
{
    static pthread_once_t filename_regex_once = PTHREAD_ONCE_INIT;
    regmatch_t match[1];
    int err;

    err = pthread_once(&filename_regex_once, filename_regex_setup);
    if (err) {
        cru_loge("failed to compile filename extension regex");
        abort();
    }

    err = regexec(&filename_ext_regex, filename, 1, match, 0);
    if (err)
        return NULL;

    if (match[0].rm_so == -1)
        return NULL;

    return &filename[match[0].rm_so + 1];
}

static bool
filename_is_png(const char *filename)
{
    const char *ext = filename_get_ext(filename);

    if (!ext) {
        cru_loge("image filename lacks extension: %s", filename);
        return false;
    } else if (cru_streq(ext, "png")) {
        return true;
    } else {
        cru_loge("image filename has unsupported extension: %s", filename);
        return false;
    }
}

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

    return abspath.buf;
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

VkFormat
cru_image_get_format(cru_image_t *image)
{
    return image->format;
}

bool
cru_image_init(cru_image_t *image, enum cru_image_type type,
                VkFormat format, uint32_t width, uint32_t height,
                bool read_only)
{
    cru_refcount_init(&image->refcount);

    if (format != VK_FORMAT_R8G8B8A8_UNORM) {
        // FINISHME: Maybe one day we'll want to support more formats.
        cru_loge("cannot create crucible image with unsupported "
                "format=0x%x", format);
        return false;
    }

    if (width == 0) {
        cru_loge("cannot create crucible image with zero width");
        return false;
    }

    if (height == 0) {
        cru_loge("cannot create crucible image with zero width");
        return false;
    }

    image->format = format;
    image->cpp = 4; // RGBA8
    image->width = width;
    image->height = height;
    image->type = type;
    image->read_only = read_only;

    return true;
}

static bool
cru_image_check_compatible(const char *func,
                            cru_image_t *a, cru_image_t *b)
{
    if (a == b) {
        cru_loge("%s: images are same", func);
        return false;
    }

    if (a->format != b->format) {
        // Maybe one day we'll want to support more formats.
        cru_loge("%s: image formats differ", func);
        return false;
    } else {
        assert(a->cpp == b->cpp);
    }

    if (a->width != b->width) {
        cru_loge("%s: image widths differ", func);
        return false;
    }

    if (a->height != b->height) {
        cru_loge("%s: image heights differ", func);
        return false;
    }

    if (a->format != b->format) {
        cru_loge("%s: image formats differ", func);
        return false;
    }

    return true;
}

cru_image_t *
cru_image_load_file(const char *filename)
{
    if (!filename_is_png(filename))
        return false;

    return cru_png_image_load_file(filename);
}

bool
cru_image_write_file(cru_image_t *image, const char *filename)
{
    bool result = false;
    char *abspath = NULL;
    const uint32_t src_width = image->width;
    const uint32_t src_height = image->height;
    const uint32_t src_stride = src_width * cru_image_pixel_size;
    uint8_t *src_pixels = NULL;
    uint8_t *src_rows[src_height];

    FILE *f = NULL;
    png_structp png_writer = NULL;
    png_infop png_info = NULL;

    if (!filename_is_png(filename))
        return false;

    abspath = cru_image_get_abspath(filename);
    if (!abspath)
        goto fail_get_abspath;

    src_pixels = image->map_pixels(image, CRU_IMAGE_MAP_ACCESS_READ);
    if (!src_pixels)
        goto fail_map_pixels;

    for (uint32_t y = 0; y < src_height; ++y) {
        src_rows[y] = src_pixels + y * src_stride;
    }

    f = fopen(abspath, "wb");
    if (!f) {
        cru_loge("failed to open file for writing: %s", abspath);
        goto fail_fopen;
    }

    // FINISHME: Error callbacks for libpng
    png_writer = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                        NULL, NULL, NULL);
    if (!png_writer) {
        cru_loge("failed to create png writer");
        goto fail_create_png_writer;
    }

    png_info = png_create_info_struct(png_writer);
    if (!png_info) {
        cru_loge("failed to create png writer info");
        goto fail_create_png_info;
    }

    png_init_io(png_writer, f);
    png_set_IHDR(png_writer, png_info,
                 src_width, src_height,
                 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_writer, png_info);
    png_set_rows(png_writer, png_info, src_rows);
    png_write_png(png_writer, png_info, PNG_TRANSFORM_IDENTITY, NULL);

    result = true;

fail_create_png_info:
    png_destroy_write_struct(&png_writer, &png_info);
fail_create_png_writer:
    fclose(f);
fail_fopen:
    // Ignore the result of unmap because no write-back occurs when unmapping
    // a read-only map.
    image->unmap_pixels(image);
fail_map_pixels:
    free(abspath);
fail_get_abspath:
    return result;
}

static bool
cru_image_copy_pixels_to_pixels(cru_image_t *dest, cru_image_t *src)
{
    bool result = false;
    uint8_t *src_pixels = NULL;
    uint8_t *dest_pixels = NULL;
    size_t image_size;

    assert(!dest->read_only);

    src_pixels = src->map_pixels(src, CRU_IMAGE_MAP_ACCESS_READ);
    if (!src_pixels)
        goto fail_map_src_pixels;

    dest_pixels = dest->map_pixels(dest, CRU_IMAGE_MAP_ACCESS_WRITE);
    if (!dest_pixels)
        goto fail_map_dest_pixels;

    // src and dest must have the same size. Should be checked earlier with
    // cru_image_check_compatible().
    image_size = src->width * src->height * cru_image_pixel_size;

    memcpy(dest, src, image_size);
    result = true;

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
        cru_loge("%s: dest is read only", __func__);
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
        cru_loge("%s: image dimensions differ", __func__);
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

    if (a->format != b->format) {
        // Maybe one day we'll want to support more formats.
        cru_loge("%s: image formats differ", __func__);
        goto cleanup;
    } else {
        assert(a->cpp == b->cpp);
    }

    if (a_x + width > a->width || a_y + height > a->height ||
        b_x + width > b->width || b_y + height > b->height) {
        cru_loge("%s: rect exceeds image dimensions", __func__);
        goto cleanup;
    }

    const uint32_t cpp = cru_image_pixel_size;
    const uint32_t row_size = cpp * width;
    const uint32_t a_stride = cpp * a->width;
    const uint32_t b_stride = cpp * b->width;

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
            cru_loge("%s: diff found in row %u of rect", __func__, y);
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
