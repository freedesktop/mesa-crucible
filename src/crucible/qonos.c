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

#include <crucible/qonos.h>

#include "cru_test.h"

VkMemoryRequirements
qoObjectGetMemoryRequirements(VkDevice dev, VkObjectType obj_type,
                              VkObject obj)
{
    VkResult result;
    VkMemoryRequirements mem_reqs;
    size_t mem_reqs_size = sizeof(mem_reqs);

    result = vkGetObjectInfo(dev, obj_type, obj,
                             VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS,
                             &mem_reqs_size, &mem_reqs);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
    }

    return mem_reqs;
}

VkMemoryRequirements
qoBufferGetMemoryRequirements(VkDevice dev, VkBuffer buffer)
{
    return qoObjectGetMemoryRequirements(dev, VK_OBJECT_TYPE_BUFFER, buffer);
}

VkMemoryRequirements
qoImageGetMemoryRequirements(VkDevice dev, VkImage image)
{
    return qoObjectGetMemoryRequirements(dev, VK_OBJECT_TYPE_IMAGE, image);
}

VkDeviceMemory
__qoAllocMemory(VkDevice dev, const VkMemoryAllocInfo *info)
{
    VkDeviceMemory memory;
    VkResult result;

    result = vkAllocMemory(dev, info, &memory);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(memory);
        t_cleanup_push_vk_device_memory(dev, memory);
    }

    return memory;
}

void *
qoMapMemory(VkDevice dev, VkDeviceMemory mem,
            VkDeviceSize offset, VkDeviceSize size,
            VkMemoryMapFlags flags)
{
    void *map;
    VkResult result;

    result = vkMapMemory(dev, mem, offset, size, flags, &map);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(map);
        t_cleanup_push_vk_memory_map(dev, mem);
    }

    return map;
}

VkDescriptorSetLayout
__qoCreateDescriptorSetLayout(VkDevice dev,
                              const VkDescriptorSetLayoutCreateInfo *info)
{
    VkDescriptorSetLayout layout;
    VkResult result;

    result = vkCreateDescriptorSetLayout(dev, info, &layout);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(layout);
        t_cleanup_push_vk_descriptor_set_layout(dev, layout);
    }

    return layout;
}

VkBuffer
__qoCreateBuffer(VkDevice dev, const VkBufferCreateInfo *info)
{
    VkBuffer buffer;
    VkResult result;

    result = vkCreateBuffer(dev, info, &buffer);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(buffer);
        t_cleanup_push_vk_buffer(dev, buffer);
    }

    return buffer;
}

VkBufferView
__qoCreateBufferView(VkDevice dev, const VkBufferViewCreateInfo *info)
{
    VkBufferView view;
    VkResult result;

    result = vkCreateBufferView(dev, info, &view);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(view);
        t_cleanup_push_vk_buffer_view(dev, view);
    }

    return view;
}

VkDynamicVpState
__qoCreateDynamicViewportState(VkDevice dev,
                               const VkDynamicVpStateCreateInfo *info)
{
    VkDynamicVpState state;
    VkResult result;

    result = vkCreateDynamicViewportState(dev, info, &state);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(state);
        t_cleanup_push_vk_dynamic_vp_state(dev, state);
    }

    return state;
}

VkDynamicRsState
__qoCreateDynamicRasterState(VkDevice dev,
                             const VkDynamicRsStateCreateInfo *info)
{
    VkDynamicRsState state;
    VkResult result;

    result = vkCreateDynamicRasterState(dev, info, &state);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(state);
        t_cleanup_push_vk_dynamic_rs_state(dev, state);
    }

    return state;
}

VkDynamicCbState
__qoCreateDynamicColorBlendState(VkDevice dev,
                                  const VkDynamicCbStateCreateInfo *info)
{
    VkDynamicCbState state;
    VkResult result;

    result = vkCreateDynamicColorBlendState(dev, info, &state);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(state);
        t_cleanup_push_vk_dynamic_cb_state(dev, state);
    }

    return state;
}

VkDynamicDsState
__qoCreateDynamicDepthStencilState(VkDevice dev,
                                   const VkDynamicDsStateCreateInfo *info)
{
    VkDynamicDsState state;
    VkResult result;

    result = vkCreateDynamicDepthStencilState(dev, info, &state);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(state);
        t_cleanup_push_vk_dynamic_ds_state(dev, state);
    }

    return state;
}

VkCmdBuffer
__qoCreateCommandBuffer(VkDevice dev, const VkCmdBufferCreateInfo *info)
{
    VkCmdBuffer cmd;
    VkResult result;

    result = vkCreateCommandBuffer(dev, info, &cmd);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(cmd);
        t_cleanup_push_vk_command_buffer(dev, cmd);
    }

    return cmd;
}

VkResult
__qoBeginCommandBuffer(VkCmdBuffer cmd, const VkCmdBufferBeginInfo *info)
{
    VkResult result;

    result = vkBeginCommandBuffer(cmd, info);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
    }

    return result;
}

VkResult
__qoEndCommandBuffer(VkCmdBuffer cmd)
{
    VkResult result;

    result = vkEndCommandBuffer(cmd);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
    }

    return result;
}

VkResult __qoEndCommandBuffer(VkCmdBuffer cmd);

VkImage
__qoCreateImage(VkDevice dev, const VkImageCreateInfo *info)
{
    VkImage image;
    VkResult result;

    result = vkCreateImage(dev, info, &image);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(image);
        t_cleanup_push_vk_image(dev, image);
    }

    return image;
}

VkShader
__qoCreateShader(VkDevice dev, const VkShaderCreateInfo *info)
{
    VkShader shader;
    VkResult result;

    result = vkCreateShader(dev, info, &shader);

    if (t_is_current()) {
        t_assert(result == VK_SUCCESS);
        t_assert(shader);
        t_cleanup_push_vk_shader(dev, shader);
    }

    return shader;
}
