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

#include "util/string.h"
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

const VkPhysicalDeviceFeatures *
__t_physical_dev_features(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.physical_dev_features;
}

const VkPhysicalDeviceProperties *
__t_physical_dev_props(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.physical_dev_props;
}

const VkPhysicalDeviceMemoryProperties *
__t_physical_dev_mem_props(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.physical_dev_mem_props;
}

const VkQueue *
__t_queue(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.queue[t_queue_num];
}

const VkQueue *
__t_queue_idx(int q)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.queue[q];
}

const VkDescriptorPool *
__t_descriptor_pool(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.descriptor_pool;
}

const VkCommandPool *
__t_cmd_pool(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.cmd_pool[t_queue_num];
}

const VkCommandPool *
__t_cmd_pool_idx(int q)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.cmd_pool[q];
}

const VkCommandBuffer *
__t_cmd_buffer(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->vk.cmd_buffer;
}

const VkImage *
__t_color_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.color_image;
}

const VkImageView *
__t_color_image_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.color_image_view;
}

const VkImage *
__t_depthstencil_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->vk.ds_image != VK_NULL_HANDLE);

    return &t->vk.ds_image;
}

const VkImageView *
__t_depthstencil_image_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->vk.depthstencil_image_view != VK_NULL_HANDLE);

    return &t->vk.depthstencil_image_view;
}

const VkRenderPass *
__t_render_pass(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->vk.render_pass;
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

const uint32_t *
__t_queue_num(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->opt.queue_num;
}

const bool *
__t_run_all_queues(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->opt.run_all_queues;
}

const char *
__t_name(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return string_data(&t->name);
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

cru_image_t *
t_ref_stencil_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(t->def->ref_stencil_filename);
    t_assert(!t->def->no_image);

    return t->ref.stencil_image;
}
