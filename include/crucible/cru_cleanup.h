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

#ifdef __cplusplus
}
#endif
