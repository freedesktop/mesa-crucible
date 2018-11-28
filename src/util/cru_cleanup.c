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

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include <pthread.h>

#include "util/cru_cleanup.h"
#include "util/cru_image.h"
#include "util/log.h"
#include "util/cru_refcount.h"
#include "util/cru_vec.h"
#include "util/xalloc.h"

struct cru_cleanup_stack {
    /// \brief Stack of commands
    ///
    /// The stack's memory layout is:
    ///
    ///   struct cmd_FOO1;
    ///   struct cmd_header;
    ///   struct cmd_FOO2;
    ///   struct cmd_header;
    ///   ...
    ///
    /// The metadata (the header) must come *after* its command because (1)
    /// commands are variable length and (2) the stack is unwound in LIFO
    /// order.
    cru_void_vec_t commands;

    cru_refcount_t refcount;
};

struct cmd_header {
   enum cru_cleanup_cmd cmd_type;
};

struct cmd_callback {
    void (*func)(void *data);
    void *data;
};

struct cmd_free {
    void *data;
};

struct cmd_cru_cleanup_stack {
    cru_cleanup_stack_t *cleanup;
};

// Non-dispatchable Vulkan objects
struct cmd_vk_instance {
    VkInstance instance;
    const VkAllocationCallbacks *alloc;
};

struct cmd_debug_cb {
    PFN_vkDestroyDebugReportCallbackEXT func;
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
};

struct cmd_vk_device {
    VkDevice dev;
    const VkAllocationCallbacks *alloc;
};

// Dispatchable Vulkan objects
struct cmd_cru_image {
    cru_image_t *image;
};

struct cmd_cru_image_array {
    cru_image_array_t *ia;
};

struct cmd_vk_buffer {
    VkDevice dev;
    VkBuffer x;
};

struct cmd_vk_buffer_view {
    VkDevice dev;
    VkBufferView x;
};

struct cmd_vk_cmd_buffer {
    VkDevice dev;
    VkCommandPool pool;
    VkCommandBuffer x;
};

struct cmd_vk_cmd_pool {
    VkDevice dev;
    VkCommandPool x;
};

struct cmd_vk_descriptor_pool {
    VkDevice dev;
    VkDescriptorPool x;
};

struct cmd_vk_descriptor_set {
    VkDevice dev;
    VkDescriptorPool pool;
    VkDescriptorSet set;
};

struct cmd_vk_descriptor_set_layout {
    VkDevice dev;
    VkDescriptorSetLayout x;
};

struct cmd_vk_device_memory {
    VkDevice dev;
    VkDeviceMemory x;
};

struct cmd_vk_event {
    VkDevice dev;
    VkEvent x;
};

struct cmd_vk_fence {
    VkDevice dev;
    VkFence x;
};

struct cmd_vk_framebuffer {
    VkDevice dev;
    VkFramebuffer x;
};

struct cmd_vk_image {
    VkDevice dev;
    VkImage x;
};

struct cmd_vk_image_view {
    VkDevice dev;
    VkImageView x;
};

struct cmd_vk_pipeline {
    VkDevice dev;
    VkPipeline x;
};

struct cmd_vk_pipeline_cache {
    VkDevice dev;
    VkPipelineCache x;
};

struct cmd_vk_pipeline_layout {
    VkDevice dev;
    VkPipelineLayout x;
};

struct cmd_vk_query_pool {
    VkDevice dev;
    VkQueryPool x;
};

struct cmd_vk_render_pass {
    VkDevice dev;
    VkRenderPass x;
};

struct cmd_vk_sampler {
    VkDevice dev;
    VkSampler x;
};

struct cmd_vk_semaphore {
    VkDevice dev;
    VkSemaphore x;
};

struct cmd_vk_shader_module {
    VkDevice dev;
    VkShaderModule x;
};

cru_cleanup_stack_t*
cru_cleanup_create(void)
{
    cru_cleanup_stack_t *c = NULL;

    c = xmalloc(sizeof(*c));
    cru_refcount_init(&c->refcount);
    cru_vec_init(&c->commands);

    return c;
}

void
cru_cleanup_reference(cru_cleanup_stack_t *c)
{
    cru_refcount_get(&c->refcount);
}

/// All commands are popped off the stack when the last refcount is dropped.
void
cru_cleanup_release(cru_cleanup_stack_t *c)
{
    if (cru_refcount_put(&c->refcount) > 0)
        return;

    cru_cleanup_pop_all(c);
    cru_vec_finish(&c->commands);
    free(c);
}

void
cru_cleanup_push_command(cru_cleanup_stack_t *c,
                          enum cru_cleanup_cmd cmd, ...)
{
    va_list va;

    va_start(va, cmd);
    cru_cleanup_push_commandv(c, cmd, va);
    va_end(va);
}

void
cru_cleanup_push_commandv(cru_cleanup_stack_t *c,
                           enum cru_cleanup_cmd cmd_type,
                           va_list va)
{
    struct cmd_header *header;

   #define CMD_CREATE(T) \
        T *cmd = cru_vec_push(&c->commands, sizeof(*cmd))

   #define CMD_SET(var) \
        cmd->var = va_arg(va, __typeof__(cmd->var))

    switch (cmd_type) {
        // Misc objects
        case CRU_CLEANUP_CMD_CALLBACK: {
            CMD_CREATE(struct cmd_callback);
            CMD_SET(func);
            CMD_SET(data);
            break;
        }
        case CRU_CLEANUP_CMD_FREE: {
            CMD_CREATE(struct cmd_free);
            CMD_SET(data);
            break;
        }

        // Crucible objects
        case CRU_CLEANUP_CMD_CRU_CLEANUP_STACK: {
            CMD_CREATE(struct cmd_cru_cleanup_stack);
            CMD_SET(cleanup);
            break;
        }
        case CRU_CLEANUP_CMD_CRU_IMAGE: {
            CMD_CREATE(struct cmd_cru_image);
            CMD_SET(image);
            break;
        }
        case CRU_CLEANUP_CMD_CRU_IMAGE_ARRAY: {
            CMD_CREATE(struct cmd_cru_image_array);
            CMD_SET(ia);
            break;
        }

        // Dispatchable Vulkan objects
        case CRU_CLEANUP_CMD_VK_INSTANCE: {
            CMD_CREATE(struct cmd_vk_instance);
            CMD_SET(instance);
            CMD_SET(alloc);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEBUG_CB: {
            CMD_CREATE(struct cmd_debug_cb);
            CMD_SET(func);
            CMD_SET(instance);
            CMD_SET(callback);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE: {
            CMD_CREATE(struct cmd_vk_device);
            CMD_SET(dev);
            CMD_SET(alloc);
            break;
        }

        // Non-dispatchable Vulkan objects
        case CRU_CLEANUP_CMD_VK_BUFFER: {
            CMD_CREATE(struct cmd_vk_buffer);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_BUFFER_VIEW: {
            CMD_CREATE(struct cmd_vk_buffer_view);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_COMMAND_BUFFER: {
            CMD_CREATE(struct cmd_vk_cmd_buffer);
            CMD_SET(dev);
            CMD_SET(pool);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_COMMAND_POOL: {
            CMD_CREATE(struct cmd_vk_cmd_pool);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DESCRIPTOR_POOL: {
            CMD_CREATE(struct cmd_vk_descriptor_pool);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET: {
            CMD_CREATE(struct cmd_vk_descriptor_set);
            CMD_SET(dev);
            CMD_SET(pool);
            CMD_SET(set);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET_LAYOUT: {
            CMD_CREATE(struct cmd_vk_descriptor_set_layout);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE_MEMORY: {
            CMD_CREATE(struct cmd_vk_device_memory);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE_MEMORY_MAP: {
            CMD_CREATE(struct cmd_vk_device_memory);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_EVENT: {
            CMD_CREATE(struct cmd_vk_event);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_FENCE: {
            CMD_CREATE(struct cmd_vk_fence);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_FRAMEBUFFER: {
            CMD_CREATE(struct cmd_vk_framebuffer);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_IMAGE: {
            CMD_CREATE(struct cmd_vk_image);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_IMAGE_VIEW: {
            CMD_CREATE(struct cmd_vk_image_view);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_PIPELINE: {
            CMD_CREATE(struct cmd_vk_pipeline);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_PIPELINE_CACHE: {
            CMD_CREATE(struct cmd_vk_pipeline_cache);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_PIPELINE_LAYOUT: {
            CMD_CREATE(struct cmd_vk_pipeline_layout);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_QUERY_POOL: {
            CMD_CREATE(struct cmd_vk_query_pool);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_RENDER_PASS: {
            CMD_CREATE(struct cmd_vk_render_pass);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_SAMPLER: {
            CMD_CREATE(struct cmd_vk_sampler);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_SEMAPHORE: {
            CMD_CREATE(struct cmd_vk_semaphore);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
        case CRU_CLEANUP_CMD_VK_SHADER_MODULE: {
            CMD_CREATE(struct cmd_vk_shader_module);
            CMD_SET(dev);
            CMD_SET(x);
            break;
        }
    }

    header = cru_vec_push(&c->commands, sizeof(*header));
    header->cmd_type = cmd_type;

   #undef CMD_CREATE
   #undef CMD_SET
}

/// Return false if there is no command to pop.
static bool
cru_cleanup_pop_impl(cru_cleanup_stack_t *c, bool noop)
{
    struct cmd_header *header;

    if (c->commands.len == 0)
        return false;

    header = cru_vec_pop(&c->commands, sizeof(*header));

    #define CMD_GET(T) \
        T *cmd = cru_vec_pop(&c->commands, sizeof(*cmd))

    // If this pop is a no-op, then don't do the command.
    #define CMD_DO(func_call) \
        do { \
            if (!noop) { \
                func_call; \
            } \
        } while (0)

    switch (header->cmd_type) {
        // Misc objects
        case CRU_CLEANUP_CMD_CALLBACK: {
            CMD_GET(struct cmd_callback);
            CMD_DO(cmd->func(cmd->data));
            break;
        }
        case CRU_CLEANUP_CMD_FREE: {
            CMD_GET(struct cmd_free);
            CMD_DO(free(cmd->data));
            break;
        }

        // Crucible objects
        case CRU_CLEANUP_CMD_CRU_CLEANUP_STACK: {
            CMD_GET(struct cmd_cru_cleanup_stack);
            CMD_DO(cru_cleanup_release(cmd->cleanup));
            break;
        }
        case CRU_CLEANUP_CMD_CRU_IMAGE: {
            CMD_GET(struct cmd_cru_image);
            CMD_DO(cru_image_release(cmd->image));
            break;
        }
        case CRU_CLEANUP_CMD_CRU_IMAGE_ARRAY: {
            CMD_GET(struct cmd_cru_image_array);
            CMD_DO(cru_image_array_release(cmd->ia));
            break;
        }

        // Dispatchable Vulkan objects
        case CRU_CLEANUP_CMD_VK_DEVICE: {
            CMD_GET(struct cmd_vk_device);
            CMD_DO(vkDestroyDevice(cmd->dev, cmd->alloc));
            break;
        }
        case CRU_CLEANUP_CMD_VK_INSTANCE: {
            CMD_GET(struct cmd_vk_instance);
            CMD_DO(vkDestroyInstance(cmd->instance, cmd->alloc));
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEBUG_CB: {
            CMD_GET(struct cmd_debug_cb);
            CMD_DO(cmd->func(cmd->instance, cmd->callback, NULL));
            break;
        }

        // Non-dispatchable Vulkan objects
        case CRU_CLEANUP_CMD_VK_BUFFER: {
            CMD_GET(struct cmd_vk_buffer);
            CMD_DO(vkDestroyBuffer(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_BUFFER_VIEW: {
            CMD_GET(struct cmd_vk_buffer_view);
            CMD_DO(vkDestroyBufferView(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_COMMAND_BUFFER: {
            CMD_GET(struct cmd_vk_cmd_buffer);
            CMD_DO(vkFreeCommandBuffers(cmd->dev, cmd->pool, 1, &cmd->x));
            break;
        }
        case CRU_CLEANUP_CMD_VK_COMMAND_POOL: {
            CMD_GET(struct cmd_vk_cmd_pool);
            CMD_DO(vkDestroyCommandPool(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_DESCRIPTOR_POOL: {
            CMD_GET(struct cmd_vk_descriptor_pool);
            CMD_DO(vkDestroyDescriptorPool(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET: {
            CMD_GET(struct cmd_vk_descriptor_set);
            CMD_DO(vkFreeDescriptorSets(cmd->dev, cmd->pool, 1, &cmd->set));
            break;
        }
        case CRU_CLEANUP_CMD_VK_DESCRIPTOR_SET_LAYOUT: {
            CMD_GET(struct cmd_vk_descriptor_set_layout);
            CMD_DO(vkDestroyDescriptorSetLayout(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE_MEMORY: {
            CMD_GET(struct cmd_vk_device_memory);
            CMD_DO(vkFreeMemory(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE_MEMORY_MAP: {
            CMD_GET(struct cmd_vk_device_memory);
            CMD_DO(vkUnmapMemory(cmd->dev, cmd->x));
            break;
        }
        case CRU_CLEANUP_CMD_VK_EVENT: {
            CMD_GET(struct cmd_vk_event);
            CMD_DO(vkDestroyEvent(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_FENCE: {
            CMD_GET(struct cmd_vk_fence);
            CMD_DO(vkDestroyFence(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_FRAMEBUFFER: {
            CMD_GET(struct cmd_vk_framebuffer);
            CMD_DO(vkDestroyFramebuffer(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_IMAGE: {
            CMD_GET(struct cmd_vk_image);
            CMD_DO(vkDestroyImage(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_IMAGE_VIEW: {
            CMD_GET(struct cmd_vk_image_view);
            CMD_DO(vkDestroyImageView(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_PIPELINE: {
            CMD_GET(struct cmd_vk_pipeline);
            CMD_DO(vkDestroyPipeline(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_PIPELINE_CACHE: {
            CMD_GET(struct cmd_vk_pipeline_cache);
            CMD_DO(vkDestroyPipelineCache(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_PIPELINE_LAYOUT: {
            CMD_GET(struct cmd_vk_pipeline_layout);
            CMD_DO(vkDestroyPipelineLayout(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_QUERY_POOL: {
            CMD_GET(struct cmd_vk_query_pool);
            CMD_DO(vkDestroyQueryPool(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_RENDER_PASS: {
            CMD_GET(struct cmd_vk_render_pass);
            CMD_DO(vkDestroyRenderPass(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_SAMPLER: {
            CMD_GET(struct cmd_vk_sampler);
            CMD_DO(vkDestroySampler(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_SEMAPHORE: {
            CMD_GET(struct cmd_vk_semaphore);
            CMD_DO(vkDestroySemaphore(cmd->dev, cmd->x, NULL));
            break;
        }
        case CRU_CLEANUP_CMD_VK_SHADER_MODULE: {
            CMD_GET(struct cmd_vk_shader_module);
            CMD_DO(vkDestroyShaderModule(cmd->dev, cmd->x, NULL));
            break;
        }
    }

    return true;

    #undef CMD_GET
    #undef CMD_DO
}

void
cru_cleanup_pop(cru_cleanup_stack_t *c)
{
    cru_cleanup_pop_impl(c, false);
}

void
cru_cleanup_pop_noop(cru_cleanup_stack_t *c)
{
    cru_cleanup_pop_impl(c, true);
}

void
cru_cleanup_pop_all(cru_cleanup_stack_t *c)
{
    while (cru_cleanup_pop_impl(c, false))
      ;;
}

void
cru_cleanup_pop_all_noop(cru_cleanup_stack_t *c)
{
    while (cru_cleanup_pop_impl(c, true))
      ;;
}
