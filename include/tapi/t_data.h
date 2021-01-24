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
/// \brief Test data
///

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util/vk_wrapper.h"

typedef struct cru_image cru_image_t;

#define t_name __t_name()
#define t_user_data __t_user_data()
#define t_instance (*__t_instance())
#define t_physical_dev (*__t_physical_dev())
#define t_physical_dev_features  (__t_physical_dev_features())
#define t_physical_dev_props  (__t_physical_dev_props())
#define t_physical_dev_mem_props  (__t_physical_dev_mem_props())
#define t_device (*__t_device())
#define t_queue (*__t_queue())
#define t_queue_idx(q) (*__t_queue_idx(q))
#define t_descriptor_pool (*__t_descriptor_pool())
#define t_cmd_pool (*__t_cmd_pool())
#define t_cmd_pool_idx(q) (*__t_cmd_pool_idx(q))
#define t_cmd_buffer (*__t_cmd_buffer())
#define t_color_image (*__t_color_image())
#define t_color_attachment_view (*__t_color_attachment_view())
#define t_color_image_view (*__t_color_image_view())
#define t_depthstencil_image (*__t_depthstencil_image())
#define t_depthstencil_image_view (*__t_depthstencil_image_view())
#define t_render_pass (*__t_render_pass())
#define t_framebuffer (*__t_framebuffer())
#define t_pipeline_cache (*__t_pipeline_cache())
#define t_width (*__t_width())
#define t_height (*__t_height())
#define t_queue_num (*__t_queue_num())
#define t_run_all_queues (*__t_run_all_queues())
cru_image_t *t_ref_image(void);
cru_image_t *t_ref_stencil_image(void);

const char *__t_name(void);
const void *__t_user_data(void);
const VkInstance *__t_instance(void);
const VkDevice *__t_device(void);
const VkPhysicalDevice *__t_physical_dev(void);
const VkPhysicalDeviceFeatures *__t_physical_dev_features(void);
const VkPhysicalDeviceProperties *__t_physical_dev_props(void);
const VkPhysicalDeviceMemoryProperties *__t_physical_dev_mem_props(void);
const VkQueue *__t_queue(void);
const VkQueue *__t_queue_idx(int q);
const VkDescriptorPool *__t_descriptor_pool(void);
const VkCommandPool *__t_cmd_pool(void);
const VkCommandPool *__t_cmd_pool_idx(int q);
const VkCommandBuffer *__t_cmd_buffer(void);
const VkImage *__t_color_image(void);
const VkImageView *__t_color_image_view(void);
const VkImage *__t_depthstencil_image(void);
const VkImageView *__t_depthstencil_image_view(void);
const VkRenderPass *__t_render_pass(void);
const VkFramebuffer *__t_framebuffer(void);
const VkPipelineCache *__t_pipeline_cache(void);
const uint32_t *__t_height(void);
const uint32_t *__t_width(void);
const uint32_t * __t_queue_num(void);
const bool * __t_run_all_queues(void);
