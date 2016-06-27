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

/// \file
/// \brief Test texturing from and rendering to image subresources in a mipmap
///        tree.
///
/// Three objects are central to the test: the VkImage that contains the mipmap
/// tree; and two memory-mapped VkBuffers, called the "source buffer" and the
/// "destination buffer", that contain pixel data for each level and layer of
/// the VkImage.
//
/// Each test has 5 stages:
///
///     1. *Populeate the source buffer*. For each level and layer in the
///        VkImage, open a data file and copy its pixels into the source
///        buffer. The pixels will serve as the reference image for this
///        level/layer of the VkImage.
///
///     2. *Upload*. For each level and layer, upload the reference pixels from
///        the source buffer to the VkImage.
///
///     3. *Download*. For each level and layer, download the pixels from the
///        VkImage into the destination buffer.
///
///     4. *Compare*. For each level and layer, compare the pixels in the
///        destination buffer against those in the source buffer.
///
/// TODO: Test multisampled images.
/// TODO: Test non-square, non-power-of-two image sizes.

#include <math.h>
#include <stdnoreturn.h>

#include "util/cru_format.h"
#include "util/cru_vec.h"
#include "util/misc.h"
#include "util/string.h"
#include "tapi/t.h"

#include "miptree-spirv.h"

typedef struct test_params test_params_t;
typedef struct test_data test_data_t;
typedef struct test_draw_data test_draw_data_t;
typedef struct miptree miptree_t;
typedef struct mipslice mipslice_t;
typedef struct mipslice_vec mipslice_vec_t;

enum miptree_upload_method {
    MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
    MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
    MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
};

enum miptree_download_method {
    MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
};

struct test_params {
    VkFormat format;
    VkImageAspectFlagBits aspect;
    VkImageViewType view_type;
    uint32_t levels;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t array_length;
    enum miptree_upload_method upload_method;
    enum miptree_download_method download_method;
};

struct test_data {
    const miptree_t *mt;

    /// Used only by upload/download methods that use vkCmdDraw*.
    struct test_draw_data {
        uint32_t num_vertices;
        VkBuffer vertex_buffer;
        VkDeviceSize vertex_buffer_offset;
        VkRenderPass render_pass;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        VkDescriptorSet desc_sets[1];
    } draw;
};

struct mipslice {
    uint32_t level;
    uint32_t array_slice;
    uint32_t z_offset;

    uint32_t width;
    uint32_t height;
    uint32_t depth;

    uint32_t buffer_offset;

    VkImage src_vk_image;
    VkImage dest_vk_image;

    cru_image_t *src_cru_image;
    cru_image_t *dest_cru_image;
};

CRU_VEC_DEFINE(struct mipslice_vec, struct mipslice);

struct miptree {
    VkImage image;

    VkBuffer src_buffer;
    VkBuffer dest_buffer;

    uint32_t width;
    uint32_t height;
    uint32_t levels;
    uint32_t array_length;

    mipslice_vec_t mipslices;
};

static const VkImageType
image_type_from_image_view_type(VkImageViewType view_type)
{
    switch (view_type) {
    case VK_IMAGE_VIEW_TYPE_1D:
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        return VK_IMAGE_TYPE_1D;
    case VK_IMAGE_VIEW_TYPE_2D:
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
    case VK_IMAGE_VIEW_TYPE_CUBE:
    case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
        return VK_IMAGE_TYPE_2D;
    case VK_IMAGE_VIEW_TYPE_3D:
        return VK_IMAGE_TYPE_3D;
    default:
        t_failf("bad VkImageViewType %d", view_type);
    }
}

// Fill the pixels with a canary color.
static void
fill_rect_with_canary(void *pixels,
                      const cru_format_info_t *format_info,
                      uint32_t width, uint32_t height)
{
    static const float peach[] = {1.0, 0.4, 0.2, 1.0};

    if (format_info->num_type == CRU_NUM_TYPE_UNORM &&
        format_info->num_channels == 4) {
        for (uint32_t i = 0; i < width * height; ++i) {
            uint8_t *rgba = pixels + (4 * i);
            rgba[0] = 255 * peach[0];
            rgba[1] = 255 * peach[1];
            rgba[2] = 255 * peach[2];
            rgba[3] = 255 * peach[3];
        }
    } else if (format_info->num_type == CRU_NUM_TYPE_SFLOAT &&
               format_info->num_channels == 1) {
        for (uint32_t i = 0; i < width * height; ++i) {
            float *f = pixels + (sizeof(float) * i);
            f[0] = M_1_PI;
        }
    } else if (format_info->format == VK_FORMAT_S8_UINT) {
        memset(pixels, 0x19, width * height);
    } else {
        t_failf("unsupported cru_format_info");
    }
}

static void
mipslice_get_description(const mipslice_t *slice, string_t *desc)
{
    const test_params_t *params = t_user_data;

    switch (params->view_type) {
    case VK_IMAGE_VIEW_TYPE_1D:
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
    case VK_IMAGE_VIEW_TYPE_2D:
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        if (params->array_length == 0) {
            string_printf(desc, "level%02u", slice->level);
        } else {
            string_printf(desc, "level%02u.array%02u",
                          slice->level, slice->array_slice);
        }
        break;
    case VK_IMAGE_VIEW_TYPE_3D:
        string_printf(desc, "level%02u.z%02u",
                      slice->level, slice->z_offset);
        break;
    default:
        t_failf("FINISHME: VkImageViewType %d", params->view_type);
        break;
    }
}

/// Ensure that each mipslice's pixels is unique, and that each pair of
/// mipslices is easily distinguishable visually.
///
/// To aid the debugging of failing tests, the perturbed pixels of each
/// mipslice must resemble the original image.  Perturb the mipslice too much,
/// and it will resemble noise, making debugging failing tests difficult.
/// Perturb too little, and it will resemble too closely adjacent mipslices,
/// allowing the test to pass in the presence of driver bugs.
static void
mipslice_perturb_pixels(void *pixels,
                       const cru_format_info_t *format_info,
                       uint32_t width, uint32_t height,
                       uint32_t level, uint32_t num_levels,
                       uint32_t layer, uint32_t num_layers)
{
    float red_scale = 1.0f - (float) level / num_levels;
    float blue_scale = 1.0f - (float) layer / num_layers;

    if (format_info->num_type == CRU_NUM_TYPE_UNORM &&
        format_info->num_channels == 4) {
        for (uint32_t i = 0; i < width * height; ++i) {
            uint8_t *rgba = pixels + 4 * i;
            rgba[0] *= red_scale;
            rgba[2] *= blue_scale;
        }
    } else if (format_info->num_type == CRU_NUM_TYPE_SFLOAT &&
               format_info->num_channels == 1) {
        for (uint32_t i = 0; i < width * height; ++i) {
            float *f = pixels + (sizeof(float) * i);
            f[0] *= red_scale;
        }
    } else if (format_info->format == VK_FORMAT_S8_UINT) {
        for (uint32_t i = 0; i < width * height; ++i) {
            // Stencil values have a small range, so it's dificult to guarantee
            // uniqueness of each mipslice while also preserving the mipslice's
            // resemblance to the original image. A good compromise is to
            // invert the pixels of every odd mipslice and also apply a small
            // shift to each pixel. The alternating inversion guarantees that
            // adjacent mipslices are easily distinguishable, yet they still
            // strongly resemble the original image.
            bool odd = (level + layer) % 2;
            uint8_t *u = pixels + i;
            u[0] = CLAMP((1 - 2 * odd) * (u[0] - 3), 0, UINT8_MAX);
        }
    } else {
        t_failf("unsupported cru_format_info");
     }
}

static string_t
mipslice_get_template_filename(const cru_format_info_t *format_info,
                               uint32_t image_width, uint32_t image_height,
                               uint32_t level, uint32_t num_levels,
                               uint32_t layer, uint32_t num_layers)
{
    const test_params_t *params = t_user_data;

    string_t filename = STRING_INIT;
    const uint32_t level_width = cru_minify(image_width, level);
    const uint32_t level_height = cru_minify(image_height, level);

    // The test attempts to make each pair of adjact mipslices visually
    // distinct to (1) reduce the probability of the test falsely passing and
    // to (2) aid the debugging of failing tests.  For most formats,
    // mipslice_perturb_pixels() provides the visual distinction.
    //
    // However, for single-channel formats, the perturbation may not provide
    // sufficient visual distinction. The perturbation acts on only one
    // dimension (the single channel) but mipslices vary along two dimensions
    // (level and layer).  To work around insufficient perturbation in the
    // single-channel case, the test selects distint source images for each
    // pair of adjacent mipslices.
    switch (format_info->format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
        string_appendf(&filename, "mandrill");
        break;
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_S8_UINT:
        if (layer % 2 == 0) {
            string_appendf(&filename, "grass-grayscale");
        } else {
            string_appendf(&filename, "pink-leaves-grayscale");
        }
        break;
    default:
        t_failf("unsuppported %s", format_info->name);
        break;
    }

    switch (params->view_type) {
    case VK_IMAGE_VIEW_TYPE_1D:
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        // Reuse 2d image files for 1d images.
        string_appendf(&filename, "-%ux%u.png", level_width, level_width);
        break;
    case VK_IMAGE_VIEW_TYPE_2D:
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
    case VK_IMAGE_VIEW_TYPE_3D:
        string_appendf(&filename, "-%ux%u.png", level_width, level_height);
        break;
    default:
        t_failf("FINISHME: VkImageViewType %d", params->view_type);
    }

    return filename;
}

/// Calculate a buffer size that can hold all subimages of the miptree.
static size_t
miptree_calc_buffer_size(void)
{
    const test_params_t *p = t_user_data;

    size_t buffer_size = 0;
    const uint32_t cpp = 4;
    const uint32_t width = p->width;
    const uint32_t height = p->height;
    const uint32_t depth = p->depth;

    switch (p->view_type) {
        case VK_IMAGE_VIEW_TYPE_1D:
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_2D:
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_3D:
            break;
        default:
            t_failf("FINISHME: VkImageViewType %d", p->view_type);
            break;
    }

    for (uint32_t l = 0; l < p->levels; ++l) {
        buffer_size += cpp * cru_minify(width, l) *
                             cru_minify(height, l) *
                             cru_minify(depth, l);
    }

    buffer_size *= p->array_length;

    return buffer_size;
}

static cru_image_t *
mipslice_make_template_image(const struct cru_format_info *format_info,
                             uint32_t image_width, uint32_t image_height,
                             uint32_t level, uint32_t num_levels,
                             uint32_t layer, uint32_t num_layers)
{
    const test_params_t *params = t_user_data;

    string_t filename = mipslice_get_template_filename(
            format_info, image_width, image_height,
            level, num_levels, layer, num_layers);

    // FIXME: Don't load the same file multiple times. It slows down the test
    // run.
    cru_image_t *file_img =
        t_new_cru_image_from_filename(string_data(&filename));

    switch (params->view_type) {
    case VK_IMAGE_VIEW_TYPE_1D:
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY: {
        // Reuse 2d image files for 1d images.
        void *pixels = cru_image_map(file_img, CRU_IMAGE_MAP_ACCESS_READ);
        t_assert(pixels);

        uint32_t level_width = cru_minify(image_width, level);
        uint32_t stride = level_width * format_info->cpp;

        t_assert(level_width == cru_image_get_width(file_img));
        t_assert(layer < cru_image_get_height(file_img));

        return t_new_cru_image_from_pixels(pixels + layer * stride,
                                           cru_image_get_format(file_img),
                                           level_width, /*height*/ 1);
    }
    case VK_IMAGE_VIEW_TYPE_2D:
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
    case VK_IMAGE_VIEW_TYPE_3D:
        return file_img;
    default:
        t_failf("FINISHME: VkImageViewType %d", params->view_type);
    }
}

static void
miptree_destroy(miptree_t *mt)
{
    if (!mt)
        return;

    cru_vec_finish(&mt->mipslices);
    free(mt);
}

static void
can_create_image(VkImageType type, VkImageTiling tiling,
                 uint32_t usage, VkFormat format)
{
    VkImageFormatProperties fmt_properties;
    VkResult result =
              vkGetPhysicalDeviceImageFormatProperties(t_physical_dev, format,
                                                       type, tiling, usage,
                                                       0, &fmt_properties);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
       t_end(TEST_RESULT_SKIP);
}

static const miptree_t *
miptree_create(void)
{
    const test_params_t *params = t_user_data;

    const VkFormat format = params->format;
    const cru_format_info_t *format_info = t_format_info(format);
    const uint32_t cpp = format_info->cpp;
    const uint32_t levels = params->levels;
    const uint32_t width = params->width;
    const uint32_t height = params->height;
    const uint32_t depth = params->depth;
    const uint32_t array_length = params->array_length;
    const size_t buffer_size = miptree_calc_buffer_size();
    const uint32_t usage_bits = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageType image_type = image_type_from_image_view_type(params->view_type);

    // Determine if an image can be created with this combination
    can_create_image(image_type, VK_IMAGE_TILING_OPTIMAL,
                     usage_bits, format);

    // Create the image that will contain the real miptree.
    VkImage image = qoCreateImage(t_device,
        .imageType = image_type,
        .format = format,
        .mipLevels = levels,
        .arrayLayers = array_length,
        .extent = {
            .width = width,
            .height = height,
            .depth = depth,
        },
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage_bits);
    VkBuffer src_buffer = qoCreateBuffer(t_device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkBuffer dest_buffer = qoCreateBuffer(t_device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkDeviceMemory image_mem = qoAllocImageMemory(t_device, image,
        .memoryTypeIndex = t_mem_type_index_for_device_access);
    VkDeviceMemory src_buffer_mem = qoAllocBufferMemory(t_device, src_buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);
    VkDeviceMemory dest_buffer_mem = qoAllocBufferMemory(t_device, dest_buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    void *src_buffer_map = qoMapMemory(t_device, src_buffer_mem,
                                       /*offset*/ 0, buffer_size, 0);
    void *dest_buffer_map = qoMapMemory(t_device, dest_buffer_mem,
                                        /*offset*/ 0, buffer_size, 0);

    qoBindImageMemory(t_device, image, image_mem, /*offset*/ 0);
    qoBindBufferMemory(t_device, src_buffer, src_buffer_mem, /*offset*/ 0);
    qoBindBufferMemory(t_device, dest_buffer, dest_buffer_mem, /*offset*/ 0);

    miptree_t *mt = xzalloc(sizeof(*mt));
    t_cleanup_push_callback((cru_cleanup_callback_func_t) miptree_destroy, mt);

    mt->image = image;
    mt->src_buffer = src_buffer;
    mt->dest_buffer = dest_buffer;
    mt->width = width;
    mt->height = height;
    mt->levels = levels;
    mt->array_length = array_length;
    cru_vec_init(&mt->mipslices);

    uint32_t buffer_offset = 0;

    for (uint32_t l = 0; l < levels; ++l) {
        const uint32_t level_width = cru_minify(width, l);
        const uint32_t level_height = cru_minify(height, l);
        const uint32_t level_depth = cru_minify(depth, l);

        // 3D array textures are illegal.
        t_assert(level_depth == 1 || array_length == 1);

        const uint32_t num_layers = MAX(level_depth, array_length);

        for (uint32_t layer = 0; layer < num_layers; ++layer) {
            void *src_pixels = src_buffer_map + buffer_offset;
            void *dest_pixels = dest_buffer_map + buffer_offset;

            VkImage src_vk_image;
            VkImage dest_vk_image;

            switch (params->upload_method) {
            case MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER:
                src_vk_image = (VkImage) {0}; // unused
                break;
            case MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE:
            case MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW:

                // Determine if an image can be created with this combination
                can_create_image(VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT, format);

                src_vk_image = qoCreateImage(t_device,
                    .format = format,
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .extent = {
                        .width = level_width,
                        .height = level_height,
                        .depth = 1,
                    },
                    .tiling = VK_IMAGE_TILING_LINEAR,
                    .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
                qoBindImageMemory(t_device, src_vk_image, src_buffer_mem,
                                  buffer_offset);
                break;
            }

            switch (params->download_method) {
            case MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER:
                dest_vk_image = (VkImage) {0}; // unused
                break;
            case MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE:
            case MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW:

                // Determine if an image can be created with this combination
                can_create_image(VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT, format);

                dest_vk_image = qoCreateImage(t_device,
                    .format = format,
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .extent = {
                        .width = level_width,
                        .height = level_height,
                        .depth = 1,
                    },
                    .tiling = VK_IMAGE_TILING_LINEAR,
                    .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT);
                qoBindImageMemory(t_device, dest_vk_image, dest_buffer_mem,
                                  buffer_offset);
                break;
            }

            cru_image_t *templ_image;
            cru_image_t *src_image;
            cru_image_t *dest_image;

            templ_image = mipslice_make_template_image(format_info,
                                                       width, height,
                                                       l, levels,
                                                       layer, num_layers);
            t_assert(level_width == cru_image_get_width(templ_image));
            t_assert(level_height == cru_image_get_height(templ_image));

            src_image = t_new_cru_image_from_pixels(src_pixels,
                    format, level_width, level_height);
            t_assert(cru_image_copy(src_image, templ_image));
            mipslice_perturb_pixels(src_pixels, format_info,
                                    level_width, level_height,
                                    l, levels,
                                    layer, num_layers);

            dest_image = t_new_cru_image_from_pixels(dest_pixels,
                    format, level_width, level_height);
            fill_rect_with_canary(dest_pixels, format_info,
                                  level_width, level_height);

            uint32_t z_offset = 0;
            if (depth > 1)
                z_offset = layer;

            uint32_t array_slice = 0;
            if (array_length > 1)
                array_slice = layer;

            mipslice_t mipslice = {
                .level = l,
                .array_slice = array_slice,
                .z_offset = z_offset,

                .width = level_width,
                .height = level_height,
                .depth = level_depth,

                .buffer_offset = buffer_offset,

                .src_vk_image = src_vk_image,
                .dest_vk_image = dest_vk_image,

                .src_cru_image = src_image,
                .dest_cru_image = dest_image,
            };

            cru_vec_push_memcpy(&mt->mipslices, &mipslice, 1);

            buffer_offset += cpp * level_width * level_height;
        }
    }

    return mt;
}

static void
miptree_upload_copy_from_buffer(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;
    const mipslice_t *slice;

    VkCommandBuffer cmd = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    cru_vec_foreach(slice, &mt->mipslices) {
        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspectMask = params->aspect,
                .mipLevel = slice->level,
                .baseArrayLayer = slice->array_slice,
                .layerCount = 1,
            },
            .imageOffset = {
                .x = 0,
                .y = 0,
                .z = slice->z_offset,
            },
            .imageExtent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
        };

        vkCmdCopyBufferToImage(cmd, mt->src_buffer, mt->image,
                               VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, VK_NULL_HANDLE);
}

static void
miptree_download_copy_to_buffer(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;
    const mipslice_t *slice;

    VkCommandBuffer cmd = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    cru_vec_foreach(slice, &mt->mipslices) {
        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspectMask = params->aspect,
                .mipLevel = slice->level,
                .baseArrayLayer = slice->array_slice,
                .layerCount = 1,
            },
            .imageOffset = {
                .x = 0,
                .y = 0,
                .z = slice->z_offset,
            },
            .imageExtent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
        };

        vkCmdCopyImageToBuffer(cmd, mt->image, VK_IMAGE_LAYOUT_GENERAL,
                               mt->dest_buffer, 1, &copy);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, VK_NULL_HANDLE);
}


static void
miptree_upload_copy_from_linear_image(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;
    const mipslice_t *slice;

    VkCommandBuffer cmd = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    cru_vec_foreach(slice, &mt->mipslices) {
        VkImageCopy copy = {
            .srcSubresource = {
                .aspectMask = params->aspect,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffset = { .x = 0, .y = 0, .z = 0 },

            .dstSubresource = {
                .aspectMask = params->aspect,
                .mipLevel = slice->level,
                .baseArrayLayer = slice->array_slice,
                .layerCount = 1,
            },
            .dstOffset = {
                .x = 0,
                .y = 0,
                .z = slice->z_offset,
            },

            .extent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
        };

        vkCmdCopyImage(cmd, slice->src_vk_image, VK_IMAGE_LAYOUT_GENERAL,
                       mt->image, VK_IMAGE_LAYOUT_GENERAL,
                       1, &copy);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, VK_NULL_HANDLE);
}

static void
miptree_download_copy_to_linear_image(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;
    const mipslice_t *slice;

    VkCommandBuffer cmd = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    cru_vec_foreach(slice, &mt->mipslices) {
        VkImageCopy copy = {
            .srcSubresource = {
                .aspectMask = params->aspect,
                .mipLevel = slice->level,
                .baseArrayLayer = slice->array_slice,
                .layerCount = 1,
            },
            .srcOffset = {
                .x = 0,
                .y = 0,
                .z = slice->z_offset,
            },

            .dstSubresource = {
                .aspectMask = params->aspect,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstOffset = { .x = 0, .y = 0, .z = 0 },

            .extent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
        };

        vkCmdCopyImage(cmd, mt->image, VK_IMAGE_LAYOUT_GENERAL,
                       slice->dest_vk_image, VK_IMAGE_LAYOUT_GENERAL,
                       1, &copy);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, VK_NULL_HANDLE);
}

static void
copy_color_images_with_draw(const test_data_t *data,
                            VkExtent2D extents[],
                            VkImageView tex_views[],
                            VkImageView attachment_views[],
                            uint32_t count)
{
    VkCommandBuffer cmd = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);
    vkCmdBindVertexBuffers(cmd, /*startBinding*/ 0, /*bindingCount*/ 1,
                           (VkBuffer[]) { data->draw.vertex_buffer},
                           (VkDeviceSize[]) { data->draw.vertex_buffer_offset });
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, data->draw.pipeline);

    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t width = extents[i].width;
        const uint32_t height = extents[i].height;

        vkCmdSetViewport(cmd, 0, 1,
            &(VkViewport) {
                .x = 0,
                .y = 0,
                .width = width,
                .height = height,
                .minDepth = 0,
                .maxDepth = 1
            });

        VkFramebuffer fb = qoCreateFramebuffer(t_device,
            .attachmentCount = 1,
            .pAttachments = (VkImageView[]) {
                attachment_views[i],
            },
            .width = width,
            .height = height,
            .layers = 1);

        vkUpdateDescriptorSets(t_device,
            1, /* writeCount */
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = data->draw.desc_sets[0],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .pImageInfo = (VkDescriptorImageInfo[]) {
                        {
                            .imageView = tex_views[i],
                            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                        },
                    },
                },
            }, 0, NULL);

        vkCmdBeginRenderPass(cmd,
            &(VkRenderPassBeginInfo) {
                .renderPass = data->draw.render_pass,
                .framebuffer = fb,
                .renderArea = { {0, 0}, {width, height} },
                .clearValueCount = 1,
                .pClearValues = NULL
            },
            VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                data->draw.pipeline_layout,
                                /*firstSet*/ 0,
                                ARRAY_LENGTH(data->draw.desc_sets),
                                data->draw.desc_sets,
                                /*dynamicOffsetCount*/ 0,
                                /*dynamicOffsets*/ NULL);
        vkCmdDraw(cmd, data->draw.num_vertices, /*instanceCount*/ 1,
                  /*firstVertex*/ 0, /*firstInstance*/ 0);
        vkCmdEndRenderPass(cmd);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, VK_NULL_HANDLE);
}

static void
miptree_upload_copy_with_draw(const test_data_t *data)
{
    const test_params_t *params = t_user_data;

    const miptree_t *mt = data->mt;
    const uint32_t num_views = mt->mipslices.len;
    VkImageView tex_views[num_views];
    VkImageView att_views[num_views];
    VkExtent2D extents[num_views];

    for (uint32_t i = 0; i < mt->mipslices.len; ++i) {
        const mipslice_t *slice = &mt->mipslices.data[i];

        extents[i].width = slice->width;
        extents[i].height = slice->height;

        tex_views[i] = qoCreateImageView(t_device,
            .image = slice->src_vk_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = params->format,
            .subresourceRange = {
                .aspectMask = params->aspect,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            });

        att_views[i] = qoCreateImageView(t_device,
            .image = mt->image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = params->format,
            .subresourceRange = {
                .aspectMask = params->aspect,
                .baseMipLevel = slice->level,
                .levelCount = 1,
                .baseArrayLayer = slice->array_slice,
                .layerCount = 1,
            });
    }

    copy_color_images_with_draw(data, extents, tex_views, att_views,
                                num_views);
}

static void
miptree_download_copy_with_draw(const test_data_t *data)
{
    const test_params_t *params = t_user_data;

    const miptree_t *mt = data->mt;
    const uint32_t num_views = mt->mipslices.len;
    VkImageView tex_views[num_views];
    VkImageView att_views[num_views];
    VkExtent2D extents[num_views];

    if (params->view_type != VK_IMAGE_VIEW_TYPE_2D) {
        // Different view types require pipelines with different sampler
        // types.
        t_failf("FINISHME: VkImageViewType %d", params->view_type);
    }

    for (uint32_t i = 0; i < mt->mipslices.len; ++i) {
        const mipslice_t *slice = &mt->mipslices.data[i];
        extents[i].width = slice->width;
        extents[i].height = slice->height;

        tex_views[i] = qoCreateImageView(t_device,
            .image = mt->image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = params->format,
            .subresourceRange = {
                .aspectMask = params->aspect,
                .baseMipLevel = slice->level,
                .levelCount = 1,
                .baseArrayLayer = slice->array_slice,
                .layerCount = 1,
            });

        att_views[i] = qoCreateImageView(t_device,
            .image = slice->dest_vk_image,
            .viewType = params->view_type,
            .format = params->format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            });
    }

    copy_color_images_with_draw(data, extents, tex_views, att_views,
                                num_views);
}

static void
miptree_upload(const test_data_t *data)
{
    const test_params_t *params = t_user_data;

    switch (params->upload_method) {
    case MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER:
        miptree_upload_copy_from_buffer(data);
        break;
    case MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE:
        miptree_upload_copy_from_linear_image(data);
        break;
    case MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW:
        miptree_upload_copy_with_draw(data);
        break;
    }
}

static void
miptree_download(const test_data_t *data)
{
    const test_params_t *params = t_user_data;

    switch (params->download_method) {
    case MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER:
        miptree_download_copy_to_buffer(data);
        break;
    case MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE:
        miptree_download_copy_to_linear_image(data);
        break;
    case MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW:
        miptree_download_copy_with_draw(data);
        break;
    }
}

static noreturn void
miptree_compare_images(const miptree_t *mt)
{
    test_result_t result = TEST_RESULT_PASS;
    const mipslice_t *slice;
    string_t slice_desc = STRING_INIT;

    vkQueueWaitIdle(t_queue);

    cru_vec_foreach(slice, &mt->mipslices) {
        mipslice_get_description(slice, &slice_desc);

        t_dump_image_f(slice->src_cru_image, "%s.ref.png", string_data(&slice_desc));
        t_dump_image_f(slice->dest_cru_image, "%s.actual.png", string_data(&slice_desc));

        if (!cru_image_compare(slice->src_cru_image, slice->dest_cru_image)) {
            loge("image incorrect at %s", string_data(&slice_desc));
            result = TEST_RESULT_FAIL;
        }
    }

    t_end(result);
}

static void
init_draw_data(test_draw_data_t *draw_data)
{
    const test_params_t *params = t_user_data;

    if (!(params->upload_method == MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW ||
          params->download_method == MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW)) {
        return;
    }

    const float position_data[] = {
        -1, -1,
         1, -1,
         1,  1,
        -1,  1,
    };
    const uint32_t num_position_components = 2;
    const uint32_t num_vertices =
        ARRAY_LENGTH(position_data) / num_position_components;
    const size_t vb_size = sizeof(position_data);

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .attachmentCount = 1,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = params->format,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorAttachmentCount = 1,
                .pColorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_GENERAL,
                    },
                },
            }
        });

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec2 a_position;

        void main()
        {
            gl_Position = vec4(a_position, 0, 1);
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(set = 0, binding = 0) uniform sampler2D u_tex;
        layout(location = 0) out vec4 f_color;

        // glslang doesn't get the Vulkan default right so we have to
        // manually re-declare gl_FragCoord
        layout(origin_upper_left) in vec4 gl_FragCoord;

        void main()
        {
            f_color = texelFetch(u_tex, ivec2(gl_FragCoord), 0);
        }
    );

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 1,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
        });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            .vertexShader = vs,
            .fragmentShader = fs,
            .dynamicStates = (1 << VK_DYNAMIC_STATE_VIEWPORT) |
                             (1 << VK_DYNAMIC_STATE_SCISSOR),
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .layout = pipeline_layout,
            .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
                    {
                        .binding = 0,
                        .stride = num_position_components * sizeof(float),
                        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                    },
                },
                .vertexAttributeDescriptionCount = 1,
                .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
                    {
                        .location = 0,
                        .binding = 0,
                        .format = VK_FORMAT_R32G32_SFLOAT,
                        .offset = 0,
                    },
                },
            },
            .renderPass = pass,
            .subpass = 0,
        }});

    VkBuffer vb = qoCreateBuffer(t_device, .size = vb_size,
                                 .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vb_mem = qoAllocBufferMemory(t_device, vb,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    qoBindBufferMemory(t_device, vb, vb_mem, /*offset*/ 0);

    memcpy(qoMapMemory(t_device, vb_mem, /*offset*/ 0, vb_size, /*flags*/ 0),
           position_data, sizeof(position_data));

    VkDescriptorSet desc_set =
        qoAllocateDescriptorSet(t_device,
                                .descriptorPool = t_descriptor_pool,
                                .pSetLayouts = &set_layout);

    // Prevent dumb bugs by initializing the struct in one shot.
    *draw_data = (test_draw_data_t) {
        .vertex_buffer = vb,
        .vertex_buffer_offset = 0,
        .num_vertices = num_vertices,
        .pipeline_layout = pipeline_layout,
        .pipeline = pipeline,
        .render_pass = pass,
        .desc_sets = {
            desc_set,
        },
    };
}

static void
test(void)
{
    test_data_t data = {0};

    data.mt = miptree_create();
    init_draw_data(&data.draw);

    miptree_upload(&data);
    miptree_download(&data);
    miptree_compare_images(data.mt);
}

#include "miptree_gen.c"
