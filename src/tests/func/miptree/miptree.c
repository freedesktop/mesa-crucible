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

#include <crucible/cru.h>

#include "miptree-spirv.h"

typedef struct test_params test_params_t;
typedef struct miptree miptree_t;
typedef struct mipslice mipslice_t;

enum miptree_upload_method {
    MIPTREE_UPLOAD_METHOD_COPY,
    MIPTREE_UPLOAD_METHOD_RENDER,
};

enum miptree_download_method {
    MIPTREE_DOWNLOAD_METHOD_COPY,
    MIPTREE_DOWNLOAD_METHOD_RENDER,
};

struct test_params {
    VkImageViewType view_type;
    uint32_t levels;
    uint32_t width;
    uint32_t height;
    uint32_t array_length;
    enum miptree_upload_method upload_method;
    enum miptree_download_method download_method;
};

struct miptree {
    VkImage image;

    VkBuffer src_buffer;
    VkBuffer dest_buffer;

    VkFormat format;
    uint32_t cpp;

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

    VkImageView texture_view;
    VkColorAttachmentView color_view;

    cru_image_t *src_cru_image;
    cru_image_t *dest_cru_image;
    uint32_t buffer_offset;
};

static const char *image512x512_filenames[] = {
    "mandrill-512x512.png",
    "mandrill-256x256.png",
    "mandrill-128x128.png",
    "mandrill-64x64.png",
    "mandrill-32x32.png",
    "mandrill-16x16.png",
    "mandrill-8x8.png",
    "mandrill-4x4.png",
    "mandrill-2x2.png",
    "mandrill-1x1.png",
};

// Fill the pixels with a canary color.
static void
fill_rect_with_canary(void *pixels, VkFormat format,
                      uint32_t width, uint32_t height)
{
    static const float peach[] = {1.0, 0.4, 0.2, 1.0};

    // Maybe we'll want to test other formats in the future.
    t_assert(format == VK_FORMAT_R8G8B8A8_UNORM);

    for (uint32_t i = 0; i < width * height; ++i) {
        uint8_t *rgba = pixels + (4 * i);

        rgba[0] = 255 * peach[0];
        rgba[1] = 255 * peach[1];
        rgba[2] = 255 * peach[2];
        rgba[3] = 255 * peach[3];
    }
}

/// Ensure that each mipslice's image is unique, and that each pair of images
/// is easily distinguishable visually.
static void
adjust_mipslice_color(void *pixels, VkFormat format,
                      uint32_t width, uint32_t height,
                      uint32_t level, uint32_t num_levels,
                      uint32_t layer, uint32_t num_layers)
{
    float red_scale = 1.0f - (float) level / num_levels;
    float blue_scale = 1.0f - (float) layer / num_layers;

    t_assertf(format == VK_FORMAT_R8G8B8A8_UNORM,
              "FINISHME: format 0x%x", format);

    for (uint32_t i = 0; i < width * height; ++i) {
        uint8_t *rgba = pixels + 4 * i;

        rgba[0] *= red_scale;
        rgba[2] *= blue_scale;
    }
}

static const char *
get_image_filename(uint32_t level)
{
    const test_params_t *p = t_user_data;

    if (p->width == 512 && p->height == 512) {
        t_assert(level < ARRAY_LENGTH(image512x512_filenames));
        return image512x512_filenames[level];
    } else {
        t_failf("test does support (width, height) = (%u, %u)",
                p->width, p->height);
    }
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

static const miptree_t *
miptree_create(void)
{
    const test_params_t *params = t_user_data;

    /* FINISHME: 1D, 1D array, cube map, and 3D textures */
    t_assert(params->view_type == VK_IMAGE_VIEW_TYPE_2D);

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const uint32_t cpp = 4;

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

    VkMemoryRequirements image_reqs =
        qoImageGetMemoryRequirements(t_device, image);
    VkMemoryRequirements src_buffer_reqs =
        qoBufferGetMemoryRequirements(t_device, src_buffer);
    VkMemoryRequirements dest_buffer_reqs =
        qoBufferGetMemoryRequirements(t_device, dest_buffer);

    VkDeviceMemory image_mem = qoAllocMemory(t_device,
        .allocationSize = image_reqs.size);
    VkDeviceMemory src_buffer_mem = qoAllocMemory(t_device,
        .allocationSize = src_buffer_reqs.size,
        .memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkDeviceMemory dest_buffer_mem = qoAllocMemory(t_device,
        .allocationSize = dest_buffer_reqs.size,
        .memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    void *src_buffer_map = qoMapMemory(t_device, src_buffer_mem,
                                       /*offset*/ 0, buffer_size, 0);
    void *dest_buffer_map = qoMapMemory(t_device, dest_buffer_mem,
                                        /*offset*/ 0, buffer_size, 0);

    qoQueueBindImageMemory(t_queue, image, /*index*/ 0,
                           image_mem, /*offset*/ 0);
    qoQueueBindBufferMemory(t_queue, src_buffer, /*index*/ 0,
                            src_buffer_mem, /*offset*/ 0);
    qoQueueBindBufferMemory(t_queue, dest_buffer, /*index*/ 0,
                            dest_buffer_mem, /*offset*/ 0);

    miptree_t *mt = xzalloc(sizeof(*mt) + num_slices * sizeof(mt->slices[0]));
    t_cleanup_push_callback(free, mt);

    mt->image = image;
    mt->src_buffer = src_buffer;
    mt->dest_buffer = dest_buffer;
    mt->format = format;
    mt->cpp = cpp;
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

        const char *filename = get_image_filename(l);
        cru_image_t *file_image = cru_image_load_file(filename);
        t_cleanup_push(file_image);

        t_assert(level_width == cru_image_get_width(file_image));
        t_assert(level_height == cru_image_get_height(file_image));

        for (uint32_t a = 0; a < array_length; ++a) {
            VkColorAttachmentView color_view = qoCreateColorAttachmentView(
                t_device,
                .image = image,
                .format = format,
                .mipLevel = l,
                .baseArraySlice = a);

            VkImageView texture_view = qoCreateImageView(t_device,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .subresourceRange = {
                    .aspect = VK_IMAGE_ASPECT_COLOR,
                    .baseMipLevel = l,
                    .mipLevels = 1,
                    .baseArraySlice = a,
                    .arraySize = 1,
                });

            void *src_pixels = src_buffer_map + buffer_offset;
            void *dest_pixels = dest_buffer_map + buffer_offset;

            cru_image_t *src_image;
            cru_image_t *dest_image;

            src_image = cru_image_from_pixels(src_pixels, format,
                                              level_width, level_height);
            t_cleanup_push(src_image);
            t_assert(cru_image_copy(src_image, file_image));

            dest_image = cru_image_from_pixels(dest_pixels, format,
                                               level_width, level_height);
            t_cleanup_push(dest_image);
            fill_rect_with_canary(dest_pixels, format, level_width, level_height);

            mt->slices[a * levels + l] = (mipslice_t) {
                .texture_view = texture_view,
                .color_view = color_view,
                .src_cru_image = src_image,
                .dest_cru_image = dest_image,
                .level = l,
                .array_slice = a,
                .width = level_width,
                .height = level_height,
                .buffer_offset = buffer_offset,
            };

            adjust_mipslice_color(src_pixels, format,
                                  level_width, level_height,
                                  l, levels, a, array_length);

            buffer_offset += cpp * level_width * level_height;
        }
    }

    return mt;
}

static void
miptree_upload_with_copy(const miptree_t *mt)
{
    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
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
    vkQueueSubmit(t_queue, 1, &cmd, 0);
}

static void
miptree_download_with_copy(const miptree_t *mt)
{
    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
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
    vkQueueSubmit(t_queue, 1, &cmd, 0);
}

static void
miptree_copy_src_buffer_to_textures(const miptree_t *mt, VkImage *tex_images)
{
    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
                .mipLevel = 0,
                .arraySlice = 0,
            },
            .imageOffset = {.x = 0, .y = 0, .z = 0},
            .imageExtent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
        };

        vkCmdCopyBufferToImage(cmd, mt->src_buffer, tex_images[i],
                               VK_IMAGE_LAYOUT_GENERAL, 1, &copy);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, 0);
}

static void
miptree_copy_textures_to_dest_buffer(const miptree_t *mt, VkImage *tex_images)
{
    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device);
    qoBeginCommandBuffer(cmd);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        VkBufferImageCopy copy = {
            .bufferOffset = slice->buffer_offset,
            .imageSubresource = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
                .mipLevel = 0,
                .arraySlice = 0,
            },
            .imageOffset = {.x = 0, .y = 0, .z = 0},
            .imageExtent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
        };

        vkCmdCopyImageToBuffer(cmd, tex_images[i], VK_IMAGE_LAYOUT_GENERAL,
                               mt->dest_buffer, 1, &copy);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, 0);
}

static void
render_textures(VkFormat format, VkImageView *tex_views,
                VkColorAttachmentView *color_views, VkExtent2D *extents,
                uint32_t count)
{
    static const uint32_t num_vertices = 4;
    static const uint32_t num_position_components = 2;
    static const float position_data[] = {
        -1, -1,
         1, -1,
         1,  1,
        -1,  1,
    };

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

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .layout = pipeline_layout,
            .pNext =
        &(VkPipelineVertexInputStateCreateInfo) {
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
        }}});

    size_t vb_size = sizeof(position_data);
    VkBuffer vb = qoCreateBuffer(t_device, .size = vb_size,
                                 .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkMemoryRequirements vb_reqs = qoBufferGetMemoryRequirements(t_device, vb);
    VkDeviceMemory vb_mem = qoAllocMemory(t_device,
                                          .allocationSize = vb_reqs.size);
    qoQueueBindBufferMemory(t_queue, vb, /*index*/ 0, vb_mem, /*offset*/ 0);
    void *vb_map = qoMapMemory(t_device, vb_mem, /*offset*/ 0,
                               vb_size, /*flags*/ 0);

    uint32_t vb_position_offset = 0;
    memcpy(vb_map + vb_position_offset, position_data, sizeof(position_data));

    VkDescriptorSet sets[1];
    qoAllocDescriptorSets(t_device, /*pool*/ 0,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          ARRAY_LENGTH(set_layouts),
                          set_layouts, sets);

    VkCmdBuffer cmd = qoCreateCommandBuffer(t_device);
    qoBeginCommandBuffer(cmd);
    vkCmdBindDynamicStateObject(cmd, VK_STATE_BIND_POINT_RASTER, t_dynamic_rs_state);
    vkCmdBindDynamicStateObject(cmd, VK_STATE_BIND_POINT_COLOR_BLEND, t_dynamic_cb_state);
    vkCmdBindDynamicStateObject(cmd, VK_STATE_BIND_POINT_DEPTH_STENCIL, t_dynamic_ds_state);

    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t width = extents[i].width;
        const uint32_t height = extents[i].height;

        VkDynamicVpState vp_state = qoCreateDynamicViewportState(t_device,
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

        vkCmdBindDynamicStateObject(cmd, VK_STATE_BIND_POINT_VIEWPORT,
                                    vp_state);

        VkFramebuffer fb = qoCreateFramebuffer(t_device,
            .colorAttachmentCount = 1,
            .pColorAttachments = (VkColorAttachmentBindInfo[]) {
                {
                    .view = color_views[i],
                    .layout = VK_IMAGE_LAYOUT_GENERAL,
                },
            },
            .sampleCount = 1,
            .width = width,
            .height = height,
            .layers = 1);

        VkRenderPass pass = qoCreateRenderPass(t_device,
            .renderArea = { {0, 0}, {width, height} },
            .extent = {0},
            .colorAttachmentCount = 1,
            .sampleCount = 1,
            .layers = 1,
            .pColorFormats = (VkFormat[]) { format },
            .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_GENERAL },
            .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_LOAD },
            .pColorLoadClearValues = (VkClearColor[]) {},
            .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE });

        vkUpdateDescriptors(t_device, sets[0], 1,
            (const void * []) {
                &(VkUpdateImages) {
                    .sType = VK_STRUCTURE_TYPE_UPDATE_IMAGES,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .binding = 0,
                    .arrayIndex = 0,
                    .count = 1,
                    .pImageViews = (VkImageViewAttachInfo[]) {
                        {
                            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ATTACH_INFO,
                            .view = tex_views[i],
                            .layout = VK_IMAGE_LAYOUT_GENERAL,
                        },
                    },
                },
            }
        );

        vkCmdBeginRenderPass(cmd,
            &(VkRenderPassBegin) {
                .renderPass = pass,
                .framebuffer = fb,
            });
        vkCmdBindVertexBuffers(cmd, /*startBinding*/ 0, /*bindingCount*/ 1,
                               (VkBuffer[]) {vb},
                               (VkDeviceSize[]) {vb_position_offset});
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                /*firstSet*/ 0, ARRAY_LENGTH(sets), sets,
                                /*dynamicOffsetCount*/ 0,
                                /*dynamicOffsets*/ NULL);
        vkCmdDraw(cmd, /*firstVertex*/ 0, num_vertices,
                  /*firstInstance*/ 0, /*instanceCount*/ 1);
        vkCmdEndRenderPass(cmd);
    }

    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, 0);
}

static void
miptree_create_tex_images(const miptree_t *mt, VkImage *tex_images)
{
    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        tex_images[i] = qoCreateImage(t_device,
            .format = mt->format,
            .mipLevels = 1,
            .arraySize = 1,
            .extent = {
                .width = slice->width,
                .height = slice->height,
                .depth = 1,
            },
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT);
        VkMemoryRequirements tex_reqs =
            qoImageGetMemoryRequirements(t_device, tex_images[i]);
        VkDeviceMemory tex_mem = qoAllocMemory(t_device,
                                           .allocationSize = tex_reqs.size);
        qoQueueBindImageMemory(t_queue, tex_images[i], /*index*/ 0,
                               tex_mem, /*offset*/ 0);
    }
}

static void
miptree_upload_with_render(const miptree_t *mt)
{
    VkImage tex_images[mt->num_slices];
    VkImageView tex_views[mt->num_slices];
    VkColorAttachmentView color_views[mt->num_slices];
    VkExtent2D extents[mt->num_slices];

    miptree_create_tex_images(mt, tex_images);
    miptree_copy_src_buffer_to_textures(mt, tex_images);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        extents[i].width = slice->width;
        extents[i].height = slice->height;

        color_views[i] = slice->color_view;

        tex_views[i] = qoCreateImageView(t_device,
            .image = tex_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = mt->format,
            .channels = {
                VK_CHANNEL_SWIZZLE_R,
                VK_CHANNEL_SWIZZLE_G,
                VK_CHANNEL_SWIZZLE_B,
                VK_CHANNEL_SWIZZLE_A,
            },
            .subresourceRange = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
                .baseMipLevel = 0,
                .mipLevels = 1,
                .baseArraySlice = 0,
                .arraySize = 1,
            });
    }

    render_textures(mt->format, tex_views, color_views, extents,
                    mt->num_slices);
}

static void
miptree_download_with_render(const miptree_t *mt)
{
    VkImage tex_images[mt->num_slices];
    VkImageView tex_views[mt->num_slices];
    VkColorAttachmentView color_views[mt->num_slices];
    VkExtent2D extents[mt->num_slices];

    miptree_create_tex_images(mt, tex_images);

    for (uint32_t i = 0; i < mt->num_slices; ++i) {
        const mipslice_t *slice = &mt->slices[i];

        extents[i].width = slice->width;
        extents[i].height = slice->height;

        tex_views[i] = slice->texture_view;

        color_views[i] = qoCreateColorAttachmentView(t_device,
            .image = tex_images[i],
            .format = mt->format,
            .mipLevel = 0,
            .baseArraySlice = 0,
            .arraySize = 1);
    }

    render_textures(mt->format, tex_views, color_views, extents,
                    mt->num_slices);
    miptree_copy_textures_to_dest_buffer(mt, tex_images);
}

static void
miptree_upload(const miptree_t *mt)
{
    const test_params_t *params = t_user_data;

    switch (params->upload_method) {
    case MIPTREE_UPLOAD_METHOD_COPY:
        miptree_upload_with_copy(mt);
        break;
    case MIPTREE_UPLOAD_METHOD_RENDER:
        miptree_upload_with_render(mt);
        break;
    }
}

static void
miptree_download(const miptree_t *mt)
{
    const test_params_t *params = t_user_data;

    switch (params->download_method) {
    case MIPTREE_DOWNLOAD_METHOD_COPY:
        miptree_download_with_copy(mt);
        break;
    case MIPTREE_DOWNLOAD_METHOD_RENDER:
        miptree_download_with_render(mt);
        break;
    }
}

static void cru_noreturn
miptree_compare_images(const miptree_t *mt)
{
    cru_test_result_t result = CRU_TEST_RESULT_PASS;

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
            cru_loge("image incorrect at level %u, array slice %u", l, a);
            result = CRU_TEST_RESULT_FAIL;
        }
    }

    t_end(result);
}

static void
test(void)
{
    const miptree_t *mt = NULL;

    mt = miptree_create();
    miptree_upload(mt);
    miptree_download(mt);
    miptree_compare_images(mt);
}

#include "miptree_gen.c"
