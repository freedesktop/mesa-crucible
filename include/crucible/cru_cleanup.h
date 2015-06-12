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

#include <crucible/cru_macros.h>
#include <crucible/vk_wrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cru_cleanup_stack cru_cleanup_stack_t;
typedef struct cru_image cru_image_t;

typedef void (*cru_cleanup_callback_func_t)(void *data);

enum cru_cleanup_cmd {
    CRU_CLEANUP_CMD_CALLBACK,
    CRU_CLEANUP_CMD_CRU_CLEANUP_STACK,
    CRU_CLEANUP_CMD_CRU_IMAGE,
    CRU_CLEANUP_CMD_VK_INSTANCE,
    CRU_CLEANUP_CMD_VK_DEVICE,
    CRU_CLEANUP_CMD_VK_DEVICE_MEMORY,
    CRU_CLEANUP_CMD_VK_MEMORY_MAP,
    CRU_CLEANUP_CMD_VK_OBJECT,
};

cru_cleanup_stack_t* cru_cleanup_create(void) cru_malloclike;
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
        cru_cleanup_callback_func_t  : __cru_cleanup_push_callback, \
        cru_cleanup_stack_t *        : __cru_cleanup_push_cru_cleanup_stack, \
        cru_image_t *                : __cru_cleanup_push_cru_image \
    )((c), (x), ##__VA_ARGS__)
#endif

static inline void cru_cleanup_push_callback(cru_cleanup_stack_t *c, cru_cleanup_callback_func_t func, void *data)              { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_CALLBACK, func, data); }
static inline void cru_cleanup_push_cru_cleanup_stack(cru_cleanup_stack_t *c, cru_cleanup_stack_t *x)                           { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_CRU_CLEANUP_STACK, x); }
static inline void cru_cleanup_push_cru_image(cru_cleanup_stack_t *c, cru_image_t *x)                                           { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_CRU_IMAGE, x); }
static inline void cru_cleanup_push_vk_instance(cru_cleanup_stack_t *c, VkInstance x)                                           { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_INSTANCE, x); }
static inline void cru_cleanup_push_vk_device(cru_cleanup_stack_t *c, VkDevice x)                                               { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_DEVICE, x); }
static inline void cru_cleanup_push_vk_device_memory(cru_cleanup_stack_t *c, VkDevice d, VkDeviceMemory m)                      { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_DEVICE_MEMORY, d, m); }
static inline void cru_cleanup_push_vk_memory_map(cru_cleanup_stack_t *c, VkDevice d, VkDeviceMemory m)                         { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_MEMORY_MAP, d, m); }
static inline void cru_cleanup_push_vk_object(cru_cleanup_stack_t *c, VkDevice dev, VkObjectType obj_type, VkObject obj)        { cru_cleanup_push_command(c, CRU_CLEANUP_CMD_VK_OBJECT, dev, obj_type, obj); }

static inline void cru_cleanup_push_vk_physical_device(cru_cleanup_stack_t *c, VkDevice dev, VkPhysicalDevice x)                { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_PHYSICAL_DEVICE, x); }
static inline void cru_cleanup_push_vk_queue(cru_cleanup_stack_t *c, VkDevice dev, VkQueue x)                                   { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_QUEUE, x); }
static inline void cru_cleanup_push_vk_command_buffer(cru_cleanup_stack_t *c, VkDevice dev, VkCmdBuffer x)                      { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_COMMAND_BUFFER, x); }
static inline void cru_cleanup_push_vk_buffer(cru_cleanup_stack_t *c, VkDevice dev, VkBuffer x)                                 { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_BUFFER, x); }
static inline void cru_cleanup_push_vk_buffer_view(cru_cleanup_stack_t *c, VkDevice dev, VkBufferView x)                        { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_BUFFER_VIEW, x); }
static inline void cru_cleanup_push_vk_image(cru_cleanup_stack_t *c, VkDevice dev, VkImage x)                                   { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_IMAGE, x); }
static inline void cru_cleanup_push_vk_image_view(cru_cleanup_stack_t *c, VkDevice dev, VkImageView x)                          { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_IMAGE_VIEW, x); }
static inline void cru_cleanup_push_vk_color_attachment_view(cru_cleanup_stack_t *c, VkDevice dev, VkColorAttachmentView x)     { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_COLOR_ATTACHMENT_VIEW, x); }
static inline void cru_cleanup_push_vk_depth_stencil_view(cru_cleanup_stack_t *c, VkDevice dev, VkDepthStencilView x)           { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DEPTH_STENCIL_VIEW, x); }
static inline void cru_cleanup_push_vk_shader(cru_cleanup_stack_t *c, VkDevice dev, VkShader x)                                 { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_SHADER, x); }
static inline void cru_cleanup_push_vk_pipeline(cru_cleanup_stack_t *c, VkDevice dev, VkPipeline x)                             { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_PIPELINE, x); }
static inline void cru_cleanup_push_vk_pipeline_layout(cru_cleanup_stack_t *c, VkDevice dev, VkPipelineLayout x)                { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_PIPELINE_LAYOUT, x); }
static inline void cru_cleanup_push_vk_sampler(cru_cleanup_stack_t *c, VkDevice dev, VkSampler x)                               { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_SAMPLER, x); }
static inline void cru_cleanup_push_vk_descriptor_set(cru_cleanup_stack_t *c, VkDevice dev, VkDescriptorSet x)                  { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, x); }
static inline void cru_cleanup_push_vk_descriptor_set_layout(cru_cleanup_stack_t *c, VkDevice dev, VkDescriptorSetLayout x)     { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, x); }
static inline void cru_cleanup_push_vk_descriptor_pool(cru_cleanup_stack_t *c, VkDevice dev, VkDescriptorPool x)                { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DESCRIPTOR_POOL, x); }
static inline void cru_cleanup_push_vk_dynamic_vp_state(cru_cleanup_stack_t *c, VkDevice dev, VkDynamicVpState x)               { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DYNAMIC_VP_STATE, x); }
static inline void cru_cleanup_push_vk_dynamic_rs_state(cru_cleanup_stack_t *c, VkDevice dev, VkDynamicRsState x)               { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DYNAMIC_RS_STATE, x); }
static inline void cru_cleanup_push_vk_dynamic_cb_state(cru_cleanup_stack_t *c, VkDevice dev, VkDynamicCbState x)               { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DYNAMIC_CB_STATE, x); }
static inline void cru_cleanup_push_vk_dynamic_ds_state(cru_cleanup_stack_t *c, VkDevice dev, VkDynamicDsState x)               { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_DYNAMIC_DS_STATE, x); }
static inline void cru_cleanup_push_vk_fence(cru_cleanup_stack_t *c, VkDevice dev, VkFence x)                                   { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_FENCE, x); }
static inline void cru_cleanup_push_vk_semaphore(cru_cleanup_stack_t *c, VkDevice dev, VkSemaphore x)                           { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_SEMAPHORE, x); }
static inline void cru_cleanup_push_vk_event(cru_cleanup_stack_t *c, VkDevice dev, VkEvent x)                                   { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_EVENT, x); }
static inline void cru_cleanup_push_vk_query_pool(cru_cleanup_stack_t *c, VkDevice dev, VkQueryPool x)                          { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_QUERY_POOL, x); }
static inline void cru_cleanup_push_vk_framebuffer(cru_cleanup_stack_t *c, VkDevice dev, VkFramebuffer x)                       { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_FRAMEBUFFER, x); }
static inline void cru_cleanup_push_vk_render_pass(cru_cleanup_stack_t *c, VkDevice dev, VkRenderPass x)                        { cru_cleanup_push_vk_object(c, dev, VK_OBJECT_TYPE_RENDER_PASS, x); }

#ifdef __cplusplus
}
#endif
