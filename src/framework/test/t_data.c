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

#include "test.h"

const VkInstance *
__t_instance(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.instance;
}

const VkDevice *
__t_device(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.device;
}

const VkPhysicalDevice *
__t_physical_dev(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.physical_dev;
}

const VkPhysicalDeviceMemoryProperties *
__t_physical_dev_mem_props(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.physical_dev_mem_props;
}

const uint32_t
__t_mem_type_index_for_mmap(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->vk.mem_type_index_for_mmap;
}

const uint32_t
__t_mem_type_index_for_device_access(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->vk.mem_type_index_for_device_access;
}

const VkQueue *
__t_queue(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.queue;
}

const VkCmdPool *
__t_cmd_pool(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.cmd_pool;
}

const VkCmdBuffer *
__t_cmd_buffer(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.cmd_buffer;
}

const VkDynamicViewportState *
__t_dynamic_vp_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.dynamic_vp_state;
}

const VkDynamicRasterState *
__t_dynamic_rs_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.dynamic_rs_state;
}

const VkDynamicColorBlendState *
__t_dynamic_cb_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.dynamic_cb_state;
}

const VkDynamicDepthStencilState *
__t_dynamic_ds_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.dynamic_ds_state;
}

const VkImage *
__t_color_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.color_image;
}

const VkAttachmentView *
__t_color_attachment_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.color_attachment_view;
}

const VkImageView *
__t_color_image_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.color_texture_view;
}

const VkImage *
__t_ds_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->vk.ds_image.handle);

    return &t->vk.ds_image;
}

const VkAttachmentView *
__t_ds_attachment_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->vk.ds_attachment_view.handle);

    return &t->vk.ds_attachment_view;
}

const VkImageView *
__t_depth_image_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->vk.depth_image_view.handle);

    return &t->vk.depth_image_view;
}

const VkFramebuffer *
__t_framebuffer(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.framebuffer;
}

const VkPipelineCache *
__t_pipeline_cache(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.pipeline_cache;
}

const uint32_t *
__t_height(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->ref.height;
}

const uint32_t *
__t_width(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->ref.width;
}

const bool *
__t_use_spir_v(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->opt.use_spir_v;
}

const char *
__t_name(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->def->name;
}

const void *
__t_user_data(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->def->user_data;
}

cru_image_t *
t_ref_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return t->ref.image;
}
