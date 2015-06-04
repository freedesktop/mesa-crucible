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

#include <crucible/cru_array.h>
#include <crucible/cru_cleanup.h>
#include <crucible/cru_image.h>
#include <crucible/cru_refcount.h>
#include <crucible/xalloc.h>

struct cru_cleanup_stack {
    /// \brief Stack of commands
    ///
    /// The array contents look like:
    ///
    ///   struct cmd_FOO1;
    ///   struct cmd_header;
    ///   struct cmd_FOO2;
    ///   struct cmd_header;
    ///   ...
    ///
    /// Each header comes *after* its command.
    cru_array_t commands;

    cru_refcount_t refcount;
};

struct cmd_header {
   enum cru_cleanup_cmd cmd_type;
};

struct cmd_callback {
    void (*func)(void *data);
    void *data;
};

struct cmd_cru_cleanup_stack {
    cru_cleanup_stack_t *cleanup;
};

struct cmd_cru_image {
    cru_image_t *image;
};

struct cmd_vk_device {
    VkDevice device;
};

struct cmd_vk_device_memory {
    VkDevice device;
    VkDeviceMemory device_memory;
};

struct cmd_vk_instance {
   VkInstance instance;
};

struct cmd_vk_object {
    VkDevice device;
    VkObjectType obj_type;
    VkObject obj;
};

cru_cleanup_stack_t*
cru_cleanup_create(void)
{
    cru_cleanup_stack_t *c = NULL;

    c = xmalloc(sizeof(*c));
    cru_refcount_init(&c->refcount);

    if (!cru_array_init(&c->commands)) {
        free(c);
        return NULL;
    }

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

    cru_array_finish(&c->commands);
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
        T *cmd = cru_array_push(&c->commands, sizeof(*cmd)); \
        if (!cmd) \
            abort()

   #define CMD_SET(var) \
        cmd->var = va_arg(va, __typeof__(cmd->var))

    switch (cmd_type) {
        case CRU_CLEANUP_CMD_CALLBACK: {
            CMD_CREATE(struct cmd_callback);
            CMD_SET(func);
            CMD_SET(data);
            break;
        }
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
        case CRU_CLEANUP_CMD_VK_INSTANCE: {
            CMD_CREATE(struct cmd_vk_instance);
            CMD_SET(instance);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE: {
            CMD_CREATE(struct cmd_vk_device);
            CMD_SET(device);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE_MEMORY: {
            CMD_CREATE(struct cmd_vk_device_memory);
            CMD_SET(device);
            CMD_SET(device_memory);
            break;
        }
        case CRU_CLEANUP_CMD_VK_MEMORY_MAP: {
            CMD_CREATE(struct cmd_vk_device_memory);
            CMD_SET(device);
            CMD_SET(device_memory);
            break;
        }
        case CRU_CLEANUP_CMD_VK_OBJECT: {
            CMD_CREATE(struct cmd_vk_object);
            CMD_SET(device);
            CMD_SET(obj_type);
            CMD_SET(obj);
            break;
        }
    }

    header = cru_array_push(&c->commands, sizeof(*header));
    if (!header)
        abort();

    header->cmd_type = cmd_type;

   #undef CMD_CREATE
   #undef CMD_SET
}

/// Return false if there is no command to pop.
static bool
cru_cleanup_pop_impl(cru_cleanup_stack_t *c, bool noop)
{
    struct cmd_header *header;

    header = cru_array_pop(&c->commands, sizeof(*header));
    if (!header)
        return false;

    if (noop)
        return true;

   #define CMD_GET(T) \
        T *cmd = cru_array_pop(&c->commands, sizeof(*cmd)); \
        assert(cmd)

    switch (header->cmd_type) {
        case CRU_CLEANUP_CMD_CALLBACK: {
            CMD_GET(struct cmd_callback);
            cmd->func(cmd->data);
            break;
        }
        case CRU_CLEANUP_CMD_CRU_CLEANUP_STACK: {
            CMD_GET(struct cmd_cru_cleanup_stack);
            cru_cleanup_release(cmd->cleanup);
            break;
        }
        case CRU_CLEANUP_CMD_CRU_IMAGE: {
            CMD_GET(struct cmd_cru_image);
            cru_image_release(cmd->image);
            break;
        }
        case CRU_CLEANUP_CMD_VK_INSTANCE: {
            CMD_GET(struct cmd_vk_instance);
            vkDestroyInstance(cmd->instance);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE: {
            CMD_GET(struct cmd_vk_device);
            vkDestroyDevice(cmd->device);
            break;
        }
        case CRU_CLEANUP_CMD_VK_DEVICE_MEMORY: {
            CMD_GET(struct cmd_vk_device_memory);
            vkFreeMemory(cmd->device, cmd->device_memory);
            break;
        }
        case CRU_CLEANUP_CMD_VK_MEMORY_MAP: {
            CMD_GET(struct cmd_vk_device_memory);
            vkUnmapMemory(cmd->device, cmd->device_memory);
            break;
        }
        case CRU_CLEANUP_CMD_VK_OBJECT: {
            CMD_GET(struct cmd_vk_object);
            vkDestroyObject(cmd->device, cmd->obj_type, cmd->obj);
            break;
        }
    }

    return true;

   #undef CMD_GET
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
