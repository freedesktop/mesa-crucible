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
#include <endian.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util/log.h"
#include "util/xalloc.h"
#include "cru_image.h"

#include "vulkan/vulkan_core.h"

/* TODO add support for more compressed formats */
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

typedef struct cru_ktx_image cru_ktx_image_t;

struct ktx_image_info {
    FILE *file;
    const char *filename;
    size_t size;
    void *data;
    VkFormat vk_format;
    VkImageViewType target;
    uint32_t gl_type;
    uint32_t gl_type_size;
    uint32_t gl_format;
    uint32_t gl_internal_format;
    uint32_t gl_base_internal_format;
    uint32_t pixel_width;
    uint32_t pixel_height;
    uint32_t pixel_depth;
    uint32_t array_length;
    uint32_t num_faces;
    uint32_t num_miplevels;
    uint32_t num_images;

    cru_refcount_t refcount;
};

struct cru_ktx_image {
    cru_image_t image;

    void *data;
    struct ktx_image_info *info;

    struct {
        /// When mapping the cru_png_image for reading, we decode the file into
        /// this pixel array.
        uint8_t *pixels;

        /// Concrete type is cru_pixel_image_t.  The image's backing storage is
        /// `cru_ktx_image::map.pixels`.
        cru_image_t *pixel_image;

        /// Bitmask of `CRU_IMAGE_MAP_ACCESS_*`.
        uint32_t access;
    } map;
};

static const int cru_ktx_header_length = 64;
static const char cru_ktx_magic_number[12] =
        { 0xab, 'K', 'T', 'X', ' ', '1', '1', 0xbb, '\r', '\n', 0x1a, '\n' };

static void
cru_ktx_image_info_unref(struct ktx_image_info *image_info)
{
    if (cru_refcount_put(&image_info->refcount) == 0) {
        fclose(image_info->file);
        free(image_info);
    }
}

static void
cru_ktx_image_destroy(cru_image_t *image)
{
    cru_ktx_image_t *ktx_image = (cru_ktx_image_t *) image;
    if (!ktx_image)
        return;

    cru_ktx_image_info_unref(ktx_image->info);
    free(ktx_image);
}

static uint8_t *
cru_ktx_image_map_pixels(cru_image_t *image, uint32_t access)
{
    cru_ktx_image_t *ktx_image = (cru_ktx_image_t *)image;

    if (access & CRU_IMAGE_MAP_ACCESS_WRITE) {
        loge("crucible ktx images are read-only; cannot image for writing");
        goto fail;
    }
    if (ktx_image->map.pixels)
        return ktx_image->map.pixels;

    ktx_image->map.access = access;
    ktx_image->map.pixels = ktx_image->data;
    return ktx_image->data;
 fail:
    return NULL;
}

static bool
cru_ktx_image_unmap_pixels(cru_image_t *image)
{
    cru_ktx_image_t *ktx_image = (cru_ktx_image_t *)image;
    // PNG images are always read-only.
    assert(!(ktx_image->map.access & CRU_IMAGE_MAP_ACCESS_WRITE));

    return true;
}

static bool
cru_ktx_get_vk_format(struct ktx_image_info *image)
{
    /* add more format translation */
    if (image->gl_internal_format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
        image->vk_format = VK_FORMAT_BC3_UNORM_BLOCK;
        return true;
    }
    return false;
}

static bool
cru_ktx_calc_target(struct ktx_image_info *image)
{
    if (image->pixel_width == 0)
        goto bad_target;
    else if (image->pixel_height == 0) {
        if (image->pixel_depth != 0)
            goto bad_target;
        if (image->num_faces != 1)
            goto bad_target;
        if (image->array_length == 0)
            image->target = VK_IMAGE_VIEW_TYPE_1D;
        else
            image->target = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    } else if (image->pixel_depth == 0) {
        if (image->array_length == 0) {
            if (image->num_faces == 1)
                image->target = VK_IMAGE_VIEW_TYPE_2D;
            else if (image->num_faces == 6)
                image->target = VK_IMAGE_VIEW_TYPE_CUBE;
            else
                goto bad_target;
        } else {
            if (image->num_faces == 1)
                image->target = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            else if (image->num_faces == 6)
                image->target = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            else
                goto bad_target;
        }
    } else {
        if (image->array_length != 0)
            goto bad_target;
        if (image->num_faces != 1)
            goto bad_target;
        image->target = VK_IMAGE_VIEW_TYPE_3D;

    }
    return true;

 bad_target:
    loge("invalid image parameters");
    return false;
}

static void
cru_ktx_calc_base_image_size(struct ktx_image_info *image,
                             uint32_t *width,
                             uint32_t *height,
                             uint32_t *depth)
{
    switch (image->target) {
    case VK_IMAGE_VIEW_TYPE_1D:
        *width = image->pixel_width;
        *height = 0;
        *depth = 0;
        break;
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        *width = image->pixel_width;
        *height = image->array_length;
        *depth = 0;
        break;
    case VK_IMAGE_VIEW_TYPE_2D:
        *width = image->pixel_width;
        *height = image->pixel_height;
        *depth = 0;
        break;
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        *width = image->pixel_width;
        *height = image->pixel_height;
        *depth = image->array_length;
        break;
    case VK_IMAGE_VIEW_TYPE_CUBE:
        *width = image->pixel_width;
        *height = image->pixel_height;
        *depth = 0;
        break;
    case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
        *width = image->pixel_width;
        *height = image->pixel_height;
        *depth = 6 * image->array_length;
        break;
    case VK_IMAGE_VIEW_TYPE_3D:
        *width = image->pixel_width;
        *height = image->pixel_height;
        *depth = image->pixel_depth;
        break;
    default:
        assert(0);
        break;
    }
}

static bool
cru_ktx_parse_header(struct ktx_image_info *image)
{
    const uint32_t *u32 = image->data;

    if (image->size < cru_ktx_header_length) {
        loge("data size must be at least 64 bytes");
        return false;
    }

    if (memcmp(u32, cru_ktx_magic_number, sizeof(cru_ktx_magic_number)) != 0) {
        loge("KTX header missing");
        return false;
    }

    switch (u32[3]) {
    case 0x04030201:
        break;
    case 0x01020304:
        loge("big endian ktx not supported");
        return false;
    default:
        loge("ktx header is bad");
        return false;
    }

    image->gl_type = u32[4];
    image->gl_type_size = u32[5];
    image->gl_format = u32[6];
    image->gl_internal_format = u32[7];
    image->gl_base_internal_format = u32[8];
    image->pixel_width = u32[9];
    image->pixel_height = u32[10];
    image->pixel_depth = u32[11];
    image->array_length = u32[12];
    image->num_faces = u32[13];
    image->num_miplevels = u32[14];

    if (image->num_miplevels == 0) {
        loge("KTX header requests automatic mipmap generation, which crucible does not support");
        return false;
    }

    bool ok = cru_ktx_get_vk_format(image);
    if (!ok)
        return false;
    ok = cru_ktx_calc_target(image);
    if (!ok)
        return false;

    if (image->target == VK_IMAGE_VIEW_TYPE_CUBE)
        image->num_images = 6 * image->num_miplevels;
    else
        image->num_images = image->num_miplevels;

    return true;
}

static void
minify(uint32_t *n)
{
        assert(*n != 0);

        if (*n > 1)
                *n >>= 1;
}

static bool
cru_ktx_parse_images(struct ktx_image_info *image_info,
                     cru_image_array_t *ia)
{
    /*
     * Current byte being parsed. Used to traverse data with a step of
     * either 8 or 32 bits.
     */
    union piglit_ktx_position {
        uint8_t *u8;
        uint32_t *u32;
    } p;

    uint32_t pixel_width;
    uint32_t pixel_height;
    uint32_t pixel_depth;

    cru_ktx_calc_base_image_size(image_info, &pixel_width,
                                 &pixel_height, &pixel_depth);
    /* Loop counters */
    int miplevel;
    int face;

    /* Skip header. */
    p.u8 = image_info->data;
    p.u8 += cru_ktx_header_length;

    int idx = 0;
    struct cru_ktx_image *ktx_image = (struct cru_ktx_image *)ia->images[0];

#define CUR_SIZE (p.u8 - (uint8_t *) image_info->data)

    for (miplevel = 0; miplevel < image_info->num_miplevels; ++miplevel) {
        uint32_t image_size;

        if (image_info->size < CUR_SIZE + 1) {
            return false;
        }

        image_size = *p.u32;
        ++p.u32;

        for (face = 0; face < 6; ++face) {
            cru_image_init(&ktx_image->image, CRU_IMAGE_TYPE_KTX,
                           image_info->vk_format, pixel_width, pixel_height, true);

            ktx_image->image.destroy = cru_ktx_image_destroy;
            ktx_image->image.map_pixels = cru_ktx_image_map_pixels;
            ktx_image->image.unmap_pixels = cru_ktx_image_unmap_pixels;
            ktx_image->data = p.u8;
            p.u8 += image_size;
            idx++;
            if (idx >= image_info->num_images)
                break;
            ktx_image = (struct cru_ktx_image *)ia->images[idx];

            /* Padding */
            while (CUR_SIZE % 4 != 0)
                ++p.u8;

            if (image_info->target != VK_IMAGE_VIEW_TYPE_CUBE)
                break;
        }

        switch (image_info->target) {
        case VK_IMAGE_VIEW_TYPE_3D:
            minify(&pixel_width);
            minify(&pixel_height);
            minify(&pixel_depth);
            break;
        case VK_IMAGE_VIEW_TYPE_2D:
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_CUBE:
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
            minify(&pixel_width);
            minify(&pixel_height);
            break;
        case VK_IMAGE_VIEW_TYPE_1D:
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
            minify(&pixel_width);
            break;
        default:
            assert(0);
            break;
        }
    }

    if (image_info->size < CUR_SIZE) {
        /*
         * The last image's data lies, at least partially, in
         * out-of-bounds memory.
         */
        loge("size of data stream incorrect");
        return false;
    }

    image_info->size = CUR_SIZE;
    return true;
}

cru_image_array_t *
cru_ktx_image_array_load_file(const char *filename)
{
    char *abs_filename = NULL;
    FILE *file;
    int error = 0;
    bool ok;
    uint32_t size;

    abs_filename = cru_image_get_abspath(filename);
    if (!abs_filename)
        goto fail_filename;

    file = fopen(abs_filename, "rb");
    if (!file) {
        loge("failed to open file for reading: %s", abs_filename);
        goto fail_fopen;
    }

    error = fseek(file, 0, SEEK_END);
    if (error)
        goto bad_read;
    size = ftell(file);
    error = fseek(file, 0, SEEK_SET);
    if (error)
        goto bad_read;

    struct ktx_image_info *image_info = calloc(1, sizeof(struct ktx_image_info) + size);
    if (!image_info)
        goto fail_alloc;

    cru_refcount_init(&image_info->refcount);
    image_info->file = file;
    image_info->size = size;
    image_info->filename = abs_filename;
    image_info->data = image_info + 1;

    size_t size_read = fread(image_info->data, 1, image_info->size, file);
    if (size_read < image_info->size)
        goto fail_read_file;

    ok = cru_ktx_parse_header(image_info);
    if (!ok)
        goto fail_read_file;

    cru_image_array_t *ia = calloc(1, sizeof(*ia));
    if (!ia)
        goto bad_ia_alloc;
    cru_refcount_init(&ia->refcount);
    ia->num_images = image_info->num_images;
    /* allocate an array of images for each image in the ktx file. */
    ia->images = calloc(image_info->num_images, sizeof(struct cru_ktx_image *));
    if (!ia->images)
        goto bad_ia_image_ptrs_alloc;
    for (unsigned i = 0; i < image_info->num_images; i++) {
        ia->images[i] = calloc(1, sizeof(struct cru_ktx_image));
        struct cru_ktx_image *ktx_image = (struct cru_ktx_image *)ia->images[i];
        ktx_image->info = image_info;
        cru_refcount_get(&image_info->refcount);

        if (!ia->images[i])
            goto bad_ia_images_alloc;
    }
    ok = cru_ktx_parse_images(image_info, ia);
    if (!ok)
        goto bad_read;

    cru_ktx_image_info_unref(image_info);
    return ia;

bad_ia_images_alloc:
    for (unsigned i = 0; i < ia->num_images; i++)
        cru_ktx_image_destroy((struct cru_image *)ia->images[i]);
bad_ia_image_ptrs_alloc:
    free(ia->images);
bad_ia_alloc:
fail_read_file:
    free(image_info);
fail_alloc:
bad_read:
    fclose(file);
fail_fopen:
fail_filename:
    return NULL;
}
