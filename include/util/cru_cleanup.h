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

#pragma once

#include <stdarg.h>

#include "util/macros.h"
#include "util/vk_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cru_cleanup_stack cru_cleanup_stack_t;
typedef struct cru_image cru_image_t;

typedef void (*cru_cleanup_callback_func_t)(void *data);

enum cru_cleanup_cmd {
    CRU_CLEANUP_CMD_CALLBACK,
    CRU_CLEANUP_CMD_FREE,
    CRU_CLEANUP_CMD_CRU_CLEANUP_STACK,
    CRU_CLEANUP_CMD_CRU_IMAGE,
    CRU_CLEANUP_CMD_VK_BUFFER,
    CRU_CLEANUP_CMD_VK_BUFFER_VIEW,
    CRU_CLEANUP_CMD_VK_COMMAND_BUFFER,
    CRU_CLEANUP_CMD_VK_COMMAND_POOL,
    CRU_CLEANUP_CMD_VK_DESCRIPTOR_POOL,
    CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET,
    CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET_LAYOUT,
    CRU_CLEANUP_CMD_VK_DEVICE,
    CRU_CLEANUP_CMD_VK_DEVICE_MEMORY,
    CRU_CLEANUP_CMD_VK_DEVICE_MEMORY_MAP,
    CRU_CLEANUP_CMD_VK_EVENT,
    CRU_CLEANUP_CMD_VK_FENCE,
    CRU_CLEANUP_CMD_VK_FRAMEBUFFER,
    CRU_CLEANUP_CMD_VK_IMAGE,
    CRU_CLEANUP_CMD_VK_IMAGE_VIEW,
    CRU_CLEANUP_CMD_VK_INSTANCE,
    CRU_CLEANUP_CMD_VK_PIPELINE,
    CRU_CLEANUP_CMD_VK_PIPELINE_CACHE,
    CRU_CLEANUP_CMD_VK_PIPELINE_LAYOUT,
    CRU_CLEANUP_CMD_VK_QUERY_POOL,
    CRU_CLEANUP_CMD_VK_RENDER_PASS,
    CRU_CLEANUP_CMD_VK_SAMPLER,
    CRU_CLEANUP_CMD_VK_SEMAPHORE,
    CRU_CLEANUP_CMD_VK_SHADER,
    CRU_CLEANUP_CMD_VK_SHADER_MODULE,
};

malloclike cru_cleanup_stack_t* cru_cleanup_create(void);
void cru_cleanup_reference(cru_cleanup_stack_t *c);
void cru_cleanup_release(cru_cleanup_stack_t *c);
void cru_cleanup_push_command(cru_cleanup_stack_t *c, enum cru_cleanup_cmd cmd, ...);
void cru_cleanup_push_commandv(cru_cleanup_stack_t *c, enum cru_cleanup_cmd cmd, va_list va);
void cru_cleanup_pop(cru_cleanup_stack_t *c);
void cru_cleanup_pop_all(cru_cleanup_stack_t *c);
void cru_cleanup_pop_noop(cru_cleanup_stack_t *c);
void cru_cleanup_pop_all_noop(cru_cleanup_stack_t *c);

#ifdef DOXYGEN
void cru_cleanup_push(cru_cleanup_stack_t *t, T obj, ...);
#else
#define cru_cleanup_push(c, x, ...) \
    _Generic((x), \
        cru_cleanup_callback_func_t     : __cru_cleanup_push_callback, \
        cru_cleanup_stack_t *           : __cru_cleanup_push_cru_cleanup_stack, \
        cru_image_t *                   : __cru_cleanup_push_cru_image, \
    )((c), (x), ##__VA_ARGS__)
#endif

static inline void cru_cleanup_push_callback(cru_cleanup_stack_t *c, cru_cleanup_callback_func_t func, void *data)                          { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_CALLBACK,                         func, data); }
static inline void cru_cleanup_push_free(cru_cleanup_stack_t *c, void *data)                                                                { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_FREE,                             data); }
static inline void cru_cleanup_push_cru_cleanup_stack(cru_cleanup_stack_t *c, cru_cleanup_stack_t *x)                                       { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_CRU_CLEANUP_STACK,                x); }
static inline void cru_cleanup_push_cru_image(cru_cleanup_stack_t *c, cru_image_t *x)                                                       { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_CRU_IMAGE,                        x); }

static inline void cru_cleanup_push_vk_instance(cru_cleanup_stack_t *c, VkInstance x, const VkAllocationCallbacks *a)                       { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_INSTANCE,                      x, a); }
static inline void cru_cleanup_push_vk_device(cru_cleanup_stack_t *c, VkDevice x, const VkAllocationCallbacks *a)                           { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_DEVICE,                        x, a); }

static inline void cru_cleanup_push_vk_buffer(cru_cleanup_stack_t *c, VkDevice dev, VkBuffer x)                                             { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_BUFFER,                        dev, x); }
static inline void cru_cleanup_push_vk_buffer_view(cru_cleanup_stack_t *c, VkDevice dev, VkBufferView x)                                    { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_BUFFER_VIEW,                   dev, x); }
static inline void cru_cleanup_push_vk_command_buffer(cru_cleanup_stack_t *c, VkDevice dev, VkCommandBuffer x)                                  { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_COMMAND_BUFFER,                    dev, x); }
static inline void cru_cleanup_push_vk_descriptor_pool(cru_cleanup_stack_t *c, VkDevice dev, VkDescriptorPool x)                            { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_DESCRIPTOR_POOL,               dev, x); }
static inline void cru_cleanup_push_vk_descriptor_set(cru_cleanup_stack_t *c, VkDevice dev, VkDescriptorPool pool, VkDescriptorSet set)     { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET,                dev, pool, set); }
static inline void cru_cleanup_push_vk_descriptor_set_layout(cru_cleanup_stack_t *c, VkDevice dev, VkDescriptorSetLayout x)                 { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET_LAYOUT,         dev, x); }
static inline void cru_cleanup_push_vk_event(cru_cleanup_stack_t *c, VkDevice dev, VkEvent x)                                               { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_EVENT,                         dev, x); }
static inline void cru_cleanup_push_vk_fence(cru_cleanup_stack_t *c, VkDevice dev, VkFence x)                                               { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_FENCE,                         dev, x); }
static inline void cru_cleanup_push_vk_framebuffer(cru_cleanup_stack_t *c, VkDevice dev, VkFramebuffer x)                                   { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_FRAMEBUFFER,                   dev, x); }
static inline void cru_cleanup_push_vk_image(cru_cleanup_stack_t *c, VkDevice dev, VkImage x)                                               { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_IMAGE,                         dev, x); }
static inline void cru_cleanup_push_vk_image_view(cru_cleanup_stack_t *c, VkDevice dev, VkImageView x)                                      { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_IMAGE_VIEW,                    dev, x); }
static inline void cru_cleanup_push_vk_pipeline(cru_cleanup_stack_t *c, VkDevice dev, VkPipeline x)                                         { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_PIPELINE,                      dev, x); }
static inline void cru_cleanup_push_vk_pipeline_layout(cru_cleanup_stack_t *c, VkDevice dev, VkPipelineLayout x)                            { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_PIPELINE_LAYOUT,               dev, x); }
static inline void cru_cleanup_push_vk_query_pool(cru_cleanup_stack_t *c, VkDevice dev, VkQueryPool x)                                      { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_QUERY_POOL,                    dev, x); }
static inline void cru_cleanup_push_vk_render_pass(cru_cleanup_stack_t *c, VkDevice dev, VkRenderPass x)                                    { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_RENDER_PASS,                   dev, x); }
static inline void cru_cleanup_push_vk_sampler(cru_cleanup_stack_t *c, VkDevice dev, VkSampler x)                                           { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_SAMPLER,                       dev, x); }
static inline void cru_cleanup_push_vk_semaphore(cru_cleanup_stack_t *c, VkDevice dev, VkSemaphore x)                                       { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_SEMAPHORE,                     dev, x); }
static inline void cru_cleanup_push_vk_shader(cru_cleanup_stack_t *c, VkDevice dev, VkShader x)                                             { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_SHADER,                        dev, x); }
static inline void cru_cleanup_push_vk_shader_module(cru_cleanup_stack_t *c, VkDevice dev, VkShaderModule x)                                { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_SHADER_MODULE,                 dev, x); }

#ifdef __cplusplus
}
#endif
