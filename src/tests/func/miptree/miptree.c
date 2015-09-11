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
#include "util/misc.h"
#include "util/string.h"
#include "tapi/t.h"

#include "miptree-spirv.h"

typedef struct test_params test_params_t;
typedef struct test_data test_data_t;
typedef struct test_draw_data test_draw_data_t;
typedef struct miptree miptree_t;
typedef struct mipslice mipslice_t;

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
    VkImageAspect aspect;
    VkImageViewType view_type;
    uint32_t levels;
    uint32_t width;
    uint32_t height;
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

struct miptree {
    const cru_format_info_t *format_info;

    VkImage image;

    VkBuffer src_buffer;
    VkBuffer dest_buffer;

    uint32_t width;
    uint32_t height;
    uint32_t levels;
    uint32_t array_length;

    mipslice_t *slices;
    uint32_t num_slices;
};

struct mipslice {
    uint32_t level;
    uint32_t array_slice;
    uint32_t width;
    uint32_t height;

    VkImageView image_view;
    VkAttachmentView attachment_view;

    uint32_t buffer_offset;

    VkImage src_vk_image;
    VkImage dest_vk_image;

    cru_image_t *src_cru_image;
    cru_image_t *dest_cru_image;
};

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
miplevel_get_template_filename(const cru_format_info_t *format_info,
                               uint32_t image_width, uint32_t image_height,
                               uint32_t level, uint32_t num_levels,
                               uint32_t layer, uint32_t num_layers)
{
    string_t filename = STRING_INIT;

    const uint32_t level_width = cru_minify(image_width, level);
    const uint32_t level_height = cru_minify(image_height, level);

    switch (format_info->format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
        string_appendf(&filename, "mandrill");
        break;
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_S8_UINT:
        switch (layer) {
        case 0:
            string_appendf(&filename, "grass-grayscale");
            break;
        case 1:
            string_appendf(&filename, "pink-leaves-grayscale");
            break;
        default:
            t_failf("no image file exists for depth layer");
            break;
        }
        break;
    default:
        t_failf("unsuppported %s", format_info->name);
        break;
    }

    string_appendf(&filename, "-%ux%u.png", level_width, level_height);

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

    for (uint32_t l = 0; l < p->levels; ++l) {
        buffer_size += cpp * cru_minify(width, l) * cru_minify(height, l);
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
    string_t filename;

    filename = miplevel_get_template_filename(format_info,
                                              image_width, image_height,
                                              level, num_levels,
                                              layer, num_layers);

    // FIXME: Don't load the same file multiple times. It slows down the test
    // run.
    return t_new_cru_image_from_filename(string_data(&filename));
}

static const miptree_t *
miptree_create(void)
{
    const test_params_t *params = t_user_data;

    /* FINISHME: 1D, 1D array, cube map, and 3D textures */
    t_assert(params->view_type == VK_IMAGE_VIEW_TYPE_2D);

    const VkFormat format = params->format;
    const cru_format_info_t *format_info = t_format_info(format);
    const uint32_t cpp = format_info->cpp;
    const uint32_t levels = params->levels;
    const uint32_t width = params->width;
    const uint32_t height = params->height;
    const uint32_t array_length = params->array_length;
    const uint32_t num_slices = levels * array_length;
    const size_t buffer_size = miptree_calc_buffer_size();

    // Create the image that will contain the real miptree.
    VkImage image = qoCreateImage(t_device,
        .format = format,
        .mipLevels = levels,
        .arraySize = array_length,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT |
                 VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT |
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_SAMPLED_BIT);
    VkBuffer src_buffer = qoCreateBuffer(t_device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SOURCE_BIT);
    VkBuffer dest_buffer = qoCreateBuffer(t_device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SOURCE_BIT);

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

    miptree_t *mt = xzalloc(sizeof(*mt) + num_slices * sizeof(mt->slices[0]));
    t_cleanup_push_free(mt);

    mt->image = image;
    mt->src_buffer = src_buffer;
    mt->dest_buffer = dest_buffer;
    mt->format_info = format_info;
    mt->width = width;
    mt->height = height;
    mt->levels = levels;
    mt->array_length = array_length;
    mt->slices = (void *) mt + sizeof(*mt);
    mt->num_slices = num_slices;

    uint32_t buffer_offset = 0;

    for (uint32_t l = 0; l < levels; ++l) {
        const uint32_t level_width = cru_minify(width, l);
        const uint32_t level_height = cru_minify(height, l);

        for (uint32_t a = 0; a < array_length; ++a) {
            VkAttachmentView att_view = qoCreateAttachmentView(
                t_device,
                .image = image,
                .format = format,
                .mipLevel = l,
                .baseArraySlice = a);

            VkImageView image_view = qoCreateImageView(t_device,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .subresourceRange = {
                    .aspect = params->aspect,
                    .baseMipLevel = l,
                    .mipLevels = 1,
                    .baseArraySlice = a,
                    .arraySize = 1,
                });

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
                src_vk_image = qoCreateImage(t_device,
                    .format = format,
                    .mipLevels = 1,
                    .arraySize = 1,
                    .extent = {
                        .width = level_width,
                        .height = level_height,
                        .depth = 1,
                    },
                    .tiling = VK_IMAGE_TILING_LINEAR,
                    .usage = VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT);
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
                dest_vk_image = qoCreateImage(t_device,
                    .format = format,
                    .mipLevels = 1,
                    .arraySize = 1,
                    .extent = {
                        .width = level_width,
                        .height = level_height,
                        .depth = 1,
                    },
                    .tiling = VK_IMAGE_TILING_LINEAR,
                    .usage = VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT);
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
                                                       a, array_length);
            t_assert(level_width == cru_image_get_width(templ_image));
            t_assert(level_height == cru_image_get_height(templ_image));

            src_image = t_new_cru_image_from_pixels(src_pixels,
                    format, level_width, level_height);
            t_assert(cru_image_copy(src_image, templ_image));
            mipslice_perturb_pixels(src_pixels, format_info,
                                    level_width, level_height,
                                    l, levels, a, array_length);

            dest_image = t_new_cru_image_from_pixels(dest_pixels,
                    format, level_width, level_height);
            t_cleanup_push(dest_image);
            fill_rect_with_canary(dest_pixels, format_info,
                                  level_width, level_height);

            mt->slices[a * levels + l] = (mipslice_t) {
                .image_view = image_view,
                .attachment_view = att_view,

                .level = l,
                .array_slice = a,
                .width = level_width,
                .height = level_height,

                .buffer_offset = buffer_offset,

                .src_vk_image = src_vk_image,
                .dest_vk_image = dest_vk_image,

                .src_cru_image = src_image,
                .dest_cru_image = dest_image,
            };

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

    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspect = params->aspect,
                .mipLevel = slice->level,
                .arraySlice = slice->array_slice,
            },
            .imageOffset = { .x = 0, .y = 0, .z = 0 },
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
    vkQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
}

static void
miptree_download_copy_to_buffer(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;

    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspect = params->aspect,
                .mipLevel = slice->level,
                .arraySlice = slice->array_slice,
            },
            .imageOffset = { .x = 0, .y = 0, .z = 0 },
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
    vkQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
}


static void
miptree_upload_copy_from_linear_image(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;

    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkImageCopy copy = {
            .srcSubresource = {
                .aspect = params->aspect,
                .mipLevel = 0,
                .arraySlice = 0,
            },
            .srcOffset = { .x = 0, .y = 0, .z = 0 },

            .destSubresource = {
                .aspect = params->aspect,
                .mipLevel = slice->level,
                .arraySlice = slice->array_slice,
            },
            .destOffset = { .x = 0, .y = 0, .z = 0 },

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
    vkQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
}

static void
miptree_download_copy_to_linear_image(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;

    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkImageCopy copy = {
            .srcSubresource = {
                .aspect = params->aspect,
                .mipLevel = slice->level,
                .arraySlice = slice->array_slice,
            },
            .srcOffset = { .x = 0, .y = 0, .z = 0 },

            .destSubresource = {
                .aspect = params->aspect,
                .mipLevel = 0,
                .arraySlice = 0,
            },
            .destOffset = { .x = 0, .y = 0, .z = 0 },

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
    vkQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
}

static void
copy_color_images_with_draw(const test_data_t *data,
                            VkExtent2D extents[],
                            VkImageView image_views[],
                            VkAttachmentView attachment_views[],
                            uint32_t count)
{
    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd);
    vkCmdBindDynamicRasterState(cmd, t_dynamic_rs_state);
    vkCmdBindDynamicColorBlendState(cmd, t_dynamic_cb_state);
    vkCmdBindDynamicDepthStencilState(cmd, t_dynamic_ds_state);
    vkCmdBindVertexBuffers(cmd, /*startBinding*/ 0, /*bindingCount*/ 1,
                           (VkBuffer[]) { data->draw.vertex_buffer},
                           (VkDeviceSize[]) { data->draw.vertex_buffer_offset });
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, data->draw.pipeline);

    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t width = extents[i].width;
        const uint32_t height = extents[i].height;

        VkDynamicViewportState vp_state = qoCreateDynamicViewportState(t_device,
            .viewportAndScissorCount = 1,
            .pViewports = (VkViewport[]) {
                {
                    .originX = 0,
                    .originY = 0,
                    .width = width,
                    .height = height,
                    .minDepth = 0,
                    .maxDepth = 1
                },
            },
            .pScissors = (VkRect2D[]) {
                { {0, 0 }, {width, height} },
            });

        vkCmdBindDynamicViewportState(cmd, vp_state);

        VkFramebuffer fb = qoCreateFramebuffer(t_device,
            .attachmentCount = 1,
            .pAttachments = (VkAttachmentBindInfo[]) {
                {
                    .view = attachment_views[i],
                    .layout = VK_IMAGE_LAYOUT_GENERAL,
                },
            },
            .width = width,
            .height = height,
            .layers = 1);

        vkUpdateDescriptorSets(t_device,
            1, /* writeCount */
            (VkWriteDescriptorSet[]) {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .destSet = data->draw.desc_sets[0],
                    .destBinding = 0,
                    .destArrayElement = 0,
                    .count = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .pDescriptors = (VkDescriptorInfo[]) {
                        {
                            .imageView = image_views[i],
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
                .attachmentCount = 1,
                .pAttachmentClearValues = NULL
            }, VK_RENDER_PASS_CONTENTS_INLINE);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                data->draw.pipeline_layout,
                                /*firstSet*/ 0,
                                ARRAY_LENGTH(data->draw.desc_sets),
                                data->draw.desc_sets,
                                /*dynamicOffsetCount*/ 0,
                                /*dynamicOffsets*/ NULL);
        vkCmdDraw(cmd, /*firstVertex*/ 0, data->draw.num_vertices,
                  /*firstInstance*/ 0, /*instanceCount*/ 1);
        vkCmdEndRenderPass(cmd);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
}

static void
miptree_upload_copy_with_draw(const test_data_t *data)
{
    const test_params_t *params = t_user_data;
    const miptree_t *mt = data->mt;

    VkImageView image_views[mt->num_slices];
    VkAttachmentView att_views[mt->num_slices];
    VkExtent2D extents[mt->num_slices];

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        extents[i].width = slice->width;
        extents[i].height = slice->height;

        att_views[i] = slice->attachment_view;

        image_views[i] = qoCreateImageView(t_device,
            .image = slice->src_vk_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = mt->format_info->format,
            .channels = {
                VK_CHANNEL_SWIZZLE_R,
                VK_CHANNEL_SWIZZLE_G,
                VK_CHANNEL_SWIZZLE_B,
                VK_CHANNEL_SWIZZLE_A,
            },
            .subresourceRange = {
                .aspect = params->aspect,
                .baseMipLevel = 0,
                .mipLevels = 1,
                .baseArraySlice = 0,
                .arraySize = 1,
            });
    }

    copy_color_images_with_draw(data, extents, image_views, att_views,
                                mt->num_slices);
}

static void
miptree_download_copy_with_draw(const test_data_t *data)
{
    const miptree_t *mt = data->mt;
    VkImageView image_views[mt->num_slices];
    VkAttachmentView att_views[mt->num_slices];
    VkExtent2D extents[mt->num_slices];

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        extents[i].width = slice->width;
        extents[i].height = slice->height;

        image_views[i] = slice->image_view;

        att_views[i] = qoCreateAttachmentView(t_device,
            .image = slice->dest_vk_image,
            .format = mt->format_info->format,
            .mipLevel = 0,
            .baseArraySlice = 0,
            .arraySize = 1);
    }

    copy_color_images_with_draw(data, extents, image_views, att_views,
                                mt->num_slices);
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

    vkQueueWaitIdle(t_queue);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];
        const uint32_t l = slice->level;
        const uint32_t a = slice->array_slice;

        t_dump_image_f(slice->src_cru_image,
                       "level%02u.array%02u.ref.png", l, a);
        t_dump_image_f(slice->dest_cru_image,
                       "level%02u.array%02u.actual.png", l, a);

        if (!cru_image_compare(slice->src_cru_image, slice->dest_cru_image)) {
            loge("image incorrect at level %u, array slice %u", l, a);
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
                .colorCount = 1,
                .colorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_GENERAL,
                    },
                },
            }
        });

    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec2 a_position;

        void main()
        {
            gl_Position = vec4(a_position, 0, 1);
        }
    );

    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        layout(set = 0, binding = 0) uniform sampler2D u_tex;
        out vec4 f_color;

        void main()
        {
            f_color = texelFetch(u_tex, ivec2(gl_FragCoord), 0);
        }
    );

    VkDescriptorSetLayout set_layouts[] = {
        [0] = qoCreateDescriptorSetLayout(t_device,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .arraySize = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                },
            }),
    };

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .descriptorSetCount = ARRAY_LENGTH(set_layouts),
        .pSetLayouts = set_layouts);

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .layout = pipeline_layout,
            .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .bindingCount = 1,
                .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
                    {
                        .binding = 0,
                        .strideInBytes = num_position_components * sizeof(float),
                        .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
                    },
                },
                .attributeCount = 1,
                .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
                    {
                        .location = 0,
                        .binding = 0,
                        .format = VK_FORMAT_R32G32_SFLOAT,
                        .offsetInBytes = 0,
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

    VkDescriptorSet desc_sets[1];
    qoAllocDescriptorSets(t_device, QO_NULL_DESCRIPTOR_POOL,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          ARRAY_LENGTH(set_layouts),
                          set_layouts, desc_sets);

    // Prevent dumb bugs by initializing the struct in one shot.
    *draw_data = (test_draw_data_t) {
        .vertex_buffer = vb,
        .vertex_buffer_offset = 0,
        .num_vertices = num_vertices,
        .pipeline_layout = pipeline_layout,
        .pipeline = pipeline,
        .render_pass = pass,
        .desc_sets = {
            desc_sets[0],
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
