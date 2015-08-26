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
/// Crucible provides some default Vulkan memory types, described below. On
/// UMA systems, the default-provided types may be identical, because the
/// Vulkan implementation may expose only a single type. On NUMA systems, the
/// default-provided types are likely to be distinct. To ensure that your test
/// works correctly on NUMA systems, write your test assuming that
/// t_mem_type_index_for_mmap and t_mem_type_index_for_device_access point to
/// distinct types.
///
///     - t_mem_type_index_for_mmap: Prefer this memory type when allocating
///       memory that will be mapped with vkMapMemory. This type has properties
///       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and
///       VK_MEMORY_PROPERTY_HOST_NON_COHERENT_BIT. The Vulkan spec requires
///       implementations to expose at least one such memory type.
///
///     - t_mem_type_for_device_access: Prefer this memory type for best
///       performance during device-access. This type may have no property but
///       VK_MEMORY_PROPERTY_DEVICE_ONLY, which excludes support for
///       vkMapMemory.
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
#define t_physical_dev_mem_props  (__t_physical_dev_mem_props())
#define t_mem_type_index_for_mmap (__t_mem_type_index_for_mmap())
#define t_mem_type_index_for_device_access (__t_mem_type_index_for_device_access())
#define t_device (*__t_device())
#define t_queue (*__t_queue())
#define t_cmd_pool (*__t_cmd_pool())
#define t_cmd_buffer (*__t_cmd_buffer())
#define t_dynamic_vp_state (*__t_dynamic_vp_state())
#define t_dynamic_rs_state (*__t_dynamic_rs_state())
#define t_dynamic_cb_state (*__t_dynamic_cb_state())
#define t_dynamic_ds_state (*__t_dynamic_ds_state())
#define t_color_image (*__t_color_image())
#define t_color_attachment_view (*__t_color_attachment_view())
#define t_color_image_view (*__t_color_image_view())
#define t_depthstencil_image (*__t_depthstencil_image())
#define t_depthstencil_attachment_view (*__t_depthstencil_attachment_view())
#define t_depth_image_view (*__t_depth_image_view())
#define t_stencil_image_view (*__t_stencil_image_view())
#define t_framebuffer (*__t_framebuffer())
#define t_pipeline_cache (*__t_pipeline_cache())
#define t_width (*__t_width())
#define t_height (*__t_height())
#define t_use_spir_v (*__t_use_spir_v())
cru_image_t *t_ref_image(void);

const char *__t_name(void);
const void *__t_user_data(void);
const VkInstance *__t_instance(void);
const VkDevice *__t_device(void);
const VkPhysicalDevice *__t_physical_dev(void);
const VkPhysicalDeviceMemoryProperties *__t_physical_dev_mem_props(void);
const uint32_t __t_mem_type_index_for_mmap(void);
const uint32_t __t_mem_type_index_for_device_access(void);
const VkQueue *__t_queue(void);
const VkCmdPool *__t_cmd_pool(void);
const VkCmdBuffer *__t_cmd_buffer(void);
const VkDynamicViewportState *__t_dynamic_vp_state(void);
const VkDynamicRasterState *__t_dynamic_rs_state(void);
const VkDynamicColorBlendState *__t_dynamic_cb_state(void);
const VkDynamicDepthStencilState *__t_dynamic_ds_state(void);
const VkImage *__t_color_image(void);
const VkAttachmentView *__t_color_attachment_view(void);
const VkImageView *__t_color_image_view(void);
const VkImage *__t_depthstencil_image(void);
const VkAttachmentView *__t_depthstencil_attachment_view(void);
const VkImageView *__t_depth_image_view(void);
const VkImageView *__t_stencil_image_view(void);
const VkFramebuffer *__t_framebuffer(void);
const VkPipelineCache *__t_pipeline_cache(void);
const uint32_t *__t_height(void);
const uint32_t *__t_width(void);
const bool * __t_use_spir_v(void);
