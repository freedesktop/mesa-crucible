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
/// \brief Test threads' cleanup stacks
///
/// Each test thread owns its own cleanup stack. Therefore concurrent calls to
/// t_cleanup*() are safe.
///
/// TODO: Document how the test's cleanup phase works.

#pragma once

#include <stdarg.h>

#include "util/cru_cleanup.h"
#include "util/vk_wrapper.h"

typedef struct cru_image cru_image_t;

void t_cleanup_push_command(enum cru_cleanup_cmd cmd, ...);
void t_cleanup_push_commandv(enum cru_cleanup_cmd cmd, va_list va);
void t_cleanup_pop(void);
void t_cleanup_pop_all(void);

#ifdef DOXYGEN
void t_cleanup_push(T obj, ...);
#else
#define t_cleanup_push(x, ...) \
    _Generic((x), \
        cru_cleanup_callback_func_t     : t_cleanup_push_callback, \
        cru_cleanup_stack_t *           : t_cleanup_push_cru_cleanup_stack, \
        cru_image_t *                   : t_cleanup_push_cru_image \
    )((x), ##__VA_ARGS__)
#endif

static inline void t_cleanup_push_callback(void (*func)(void* data), void *data)                                    { t_cleanup_push_command(CRU_CLEANUP_CMD_CALLBACK, func, data); }
static inline void t_cleanup_push_free(void *data)                                                                  { t_cleanup_push_command(CRU_CLEANUP_CMD_FREE, data); }
static inline void t_cleanup_push_cru_cleanup_stack(cru_cleanup_stack_t *x)                                         { t_cleanup_push_command(CRU_CLEANUP_CMD_CRU_CLEANUP_STACK, x); }
static inline void t_cleanup_push_cru_image(cru_image_t *x)                                                         { t_cleanup_push_command(CRU_CLEANUP_CMD_CRU_IMAGE, x); }

static inline void t_cleanup_push_vk_debug_cb(PFN_vkDestroyDebugReportCallbackEXT f, VkInstance i, VkDebugReportCallbackEXT cb) { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DEBUG_CB, f, i, cb); }
static inline void t_cleanup_push_vk_instance(VkInstance x, const VkAllocationCallbacks *a)                         { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_INSTANCE, x, a); }
static inline void t_cleanup_push_vk_device(VkDevice x, const VkAllocationCallbacks *a)                             { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DEVICE, x, a); }

static inline void t_cleanup_push_vk_buffer(VkDevice dev, VkBuffer x)                                               { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_BUFFER, dev, x); }
static inline void t_cleanup_push_vk_buffer_view(VkDevice dev, VkBufferView x)                                      { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_BUFFER_VIEW, dev, x); }
static inline void t_cleanup_push_vk_cmd_buffer(VkDevice dev, VkCommandPool pool, VkCommandBuffer x)                                        { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_COMMAND_BUFFER, dev, pool, x); }
static inline void t_cleanup_push_vk_cmd_pool(VkDevice dev, VkCommandPool x)                                            { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_COMMAND_POOL, dev, x); }
static inline void t_cleanup_push_vk_descriptor_pool(VkDevice dev, VkDescriptorPool x)                              { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DESCRIPTOR_POOL, dev, x); }
static inline void t_cleanup_push_vk_descriptor_set(VkDevice dev, VkDescriptorPool pool, VkDescriptorSet set)       { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET, dev, pool, set); }
static inline void t_cleanup_push_vk_descriptor_set_layout(VkDevice dev, VkDescriptorSetLayout x)                   { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET_LAYOUT, dev, x); }
static inline void t_cleanup_push_vk_device_memory(VkDevice dev, VkDeviceMemory x)                                  { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DEVICE_MEMORY, dev, x); }
static inline void t_cleanup_push_vk_device_memory_map(VkDevice dev, VkDeviceMemory x)                              { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DEVICE_MEMORY_MAP, dev, x); }
static inline void t_cleanup_push_vk_event(VkDevice dev, VkEvent x)                                                 { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_EVENT, dev, x); }
static inline void t_cleanup_push_vk_fence(VkDevice dev, VkFence x)                                                 { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_FENCE, dev, x); }
static inline void t_cleanup_push_vk_framebuffer(VkDevice dev, VkFramebuffer x)                                     { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_FRAMEBUFFER, dev, x); }
static inline void t_cleanup_push_vk_image(VkDevice dev, VkImage x)                                                 { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_IMAGE, dev, x); }
static inline void t_cleanup_push_vk_image_view(VkDevice dev, VkImageView x)                                        { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_IMAGE_VIEW, dev, x); }
static inline void t_cleanup_push_vk_pipeline(VkDevice dev, VkPipeline x)                                           { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_PIPELINE, dev, x); }
static inline void t_cleanup_push_vk_pipeline_cache(VkDevice dev, VkPipelineCache x)                                { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_PIPELINE_CACHE, dev, x); }
static inline void t_cleanup_push_vk_pipeline_layout(VkDevice dev, VkPipelineLayout x)                              { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_PIPELINE_LAYOUT, dev, x); }
static inline void t_cleanup_push_vk_query_pool(VkDevice dev, VkQueryPool x)                                        { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_QUERY_POOL, dev, x); }
static inline void t_cleanup_push_vk_render_pass(VkDevice dev, VkRenderPass x)                                      { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_RENDER_PASS, dev, x); }
static inline void t_cleanup_push_vk_sampler(VkDevice dev, VkSampler x)                                             { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_SAMPLER, dev, x); }
static inline void t_cleanup_push_vk_semaphore(VkDevice dev, VkSemaphore x)                                         { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_SEMAPHORE, dev, x); }
static inline void t_cleanup_push_vk_shader_module(VkDevice dev, VkShaderModule x)                                  { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_SHADER_MODULE, dev, x); }
