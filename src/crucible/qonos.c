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

void
qoEnumeratePhysicalDevices(VkInstance instance, uint32_t *count,
                           VkPhysicalDevice *physical_devices)
{
    VkResult result;

    result = vkEnumeratePhysicalDevices(instance, count, physical_devices);
    t_assert(result == VK_SUCCESS);
}

static VkMemoryRequirements
ObjectGetMemoryRequirements(VkDevice dev, VkObjectType obj_type,
                            VkObject obj)
{
    VkResult result;
    VkMemoryRequirements mem_reqs;

    result = vkGetObjectMemoryRequirements(dev, obj_type, obj, &mem_reqs);
    t_assert(result == VK_SUCCESS);

    return mem_reqs;
}

VkMemoryRequirements
qoGetBufferMemoryRequirements(VkDevice dev, VkBuffer buffer)
{
    return ObjectGetMemoryRequirements(dev, VK_OBJECT_TYPE_BUFFER, buffer);
}

VkMemoryRequirements
qoGetImageMemoryRequirements(VkDevice dev, VkImage image)
{
    return ObjectGetMemoryRequirements(dev, VK_OBJECT_TYPE_IMAGE, image);
}

VkResult
qoBindBufferMemory(VkDevice device, VkBuffer buffer,
                   VkDeviceMemory mem, VkDeviceSize offset)
{
    VkResult result;

    result = vkBindObjectMemory(device, VK_OBJECT_TYPE_BUFFER,
                                buffer, mem, offset);
    t_assert(result == VK_SUCCESS);

    return result;
}

VkResult
qoBindImageMemory(VkDevice device, VkImage image,
                  VkDeviceMemory mem, VkDeviceSize offset)
{
    VkResult result;

    result = vkBindObjectMemory(device, VK_OBJECT_TYPE_IMAGE,
                                image, mem, offset);
    t_assert(result == VK_SUCCESS);

    return result;
}

VkResult
qoQueueSubmit(VkQueue queue, uint32_t cmdBufferCount,
              const VkCmdBuffer *cmdBuffers, VkFence fence)
{
    VkResult result;

    result = vkQueueSubmit(queue, cmdBufferCount, cmdBuffers, fence);
    t_assert(result == VK_SUCCESS);

    return result;
}

VkDeviceMemory
__qoAllocMemory(VkDevice dev, const VkMemoryAllocInfo *info)
{
    VkDeviceMemory memory;
    VkResult result;

    result = vkAllocMemory(dev, info, &memory);

    t_assert(result == VK_SUCCESS);
    t_assert(memory);
    t_cleanup_push_vk_device_memory(dev, memory);

    return memory;
}

VkDeviceMemory
__qoAllocMemoryFromRequirements(VkDevice dev,
                                const VkMemoryRequirements *mem_reqs,
                                const VkMemoryAllocInfo *override_info)
{
    // FIXME: Respect VkMemoryRequirements::memPropsRequired and
    // memPropsAllowed.

    VkMemoryAllocInfo info = *override_info;

    if (info.allocationSize == 0)
        info.allocationSize = mem_reqs->size;

    t_assert(info.allocationSize >= mem_reqs->size);

    return __qoAllocMemory(dev, &info);
}

VkDeviceMemory
__qoAllocBufferMemory(VkDevice dev, VkBuffer buffer,
                      const VkMemoryAllocInfo *override_info)
{
    VkMemoryRequirements mem_reqs =
        qoGetBufferMemoryRequirements(dev, buffer);

    return qoAllocMemoryFromRequirements(dev, &mem_reqs, override_info);
}

VkDeviceMemory
__qoAllocImageMemory(VkDevice dev, VkImage image,
                     const VkMemoryAllocInfo *override_info)
{
    VkMemoryRequirements mem_reqs =
        qoGetImageMemoryRequirements(dev, image);

    return qoAllocMemoryFromRequirements(dev, &mem_reqs, override_info);
}

void *
qoMapMemory(VkDevice dev, VkDeviceMemory mem,
            VkDeviceSize offset, VkDeviceSize size,
            VkMemoryMapFlags flags)
{
    void *map;
    VkResult result;

    result = vkMapMemory(dev, mem, offset, size, flags, &map);

    t_assert(result == VK_SUCCESS);
    t_assert(map);
    t_cleanup_push_vk_memory_map(dev, mem);

    return map;
}

VkPipelineLayout
__qoCreatePipelineLayout(VkDevice dev, const VkPipelineLayoutCreateInfo *info)
{
    VkPipelineLayout pipeline_layout;
    VkResult result;

    result = vkCreatePipelineLayout(dev, info, &pipeline_layout);

    t_assert(result == VK_SUCCESS);
    t_assert(pipeline_layout);
    t_cleanup_push_vk_pipeline_layout(dev, pipeline_layout);

    return pipeline_layout;
}

VkSampler
__qoCreateSampler(VkDevice dev, const VkSamplerCreateInfo *info)
{
    VkResult result;
    VkSampler sampler;

    result = vkCreateSampler(dev, info, &sampler);

    t_assert(result == VK_SUCCESS);
    t_assert(sampler);
    t_cleanup_push_vk_sampler(dev, sampler);

    return sampler;
}

VkDescriptorSetLayout
__qoCreateDescriptorSetLayout(VkDevice dev,
                              const VkDescriptorSetLayoutCreateInfo *info)
{
    VkDescriptorSetLayout layout;
    VkResult result;

    result = vkCreateDescriptorSetLayout(dev, info, &layout);

    t_assert(result == VK_SUCCESS);
    t_assert(layout);
    t_cleanup_push_vk_descriptor_set_layout(dev, layout);

    return layout;
}

VkResult
qoAllocDescriptorSets(VkDevice dev, VkDescriptorPool descriptorPool,
                      VkDescriptorSetUsage usage, uint32_t count,
                      const VkDescriptorSetLayout *layouts,
                      VkDescriptorSet *sets)
{
    VkResult result;
    uint32_t out_count = 0;

    result = vkAllocDescriptorSets(dev, descriptorPool, usage, count, layouts,
                                   sets, &out_count);
    t_assert(result == VK_SUCCESS);
    t_assert(out_count == count);

    for (uint32_t i = 0; i < count; ++i) {
        t_cleanup_push_vk_descriptor_set(dev, sets[i]);
    }

    return result;
}

VkBuffer
__qoCreateBuffer(VkDevice dev, const VkBufferCreateInfo *info)
{
    VkBuffer buffer;
    VkResult result;

    result = vkCreateBuffer(dev, info, &buffer);

    t_assert(result == VK_SUCCESS);
    t_assert(buffer);
    t_cleanup_push_vk_buffer(dev, buffer);

    return buffer;
}

VkBufferView
__qoCreateBufferView(VkDevice dev, const VkBufferViewCreateInfo *info)
{
    VkBufferView view;
    VkResult result;

    result = vkCreateBufferView(dev, info, &view);

    t_assert(result == VK_SUCCESS);
    t_assert(view);
    t_cleanup_push_vk_buffer_view(dev, view);

    return view;
}

VkDynamicVpState
__qoCreateDynamicViewportState(VkDevice dev,
                               const VkDynamicVpStateCreateInfo *info)
{
    VkDynamicVpState state;
    VkResult result;

    result = vkCreateDynamicViewportState(dev, info, &state);

    t_assert(result == VK_SUCCESS);
    t_assert(state);
    t_cleanup_push_vk_dynamic_vp_state(dev, state);

    return state;
}

VkDynamicRsState
__qoCreateDynamicRasterState(VkDevice dev,
                             const VkDynamicRsStateCreateInfo *info)
{
    VkDynamicRsState state;
    VkResult result;

    result = vkCreateDynamicRasterState(dev, info, &state);

    t_assert(result == VK_SUCCESS);
    t_assert(state);
    t_cleanup_push_vk_dynamic_rs_state(dev, state);

    return state;
}

VkDynamicCbState
__qoCreateDynamicColorBlendState(VkDevice dev,
                                  const VkDynamicCbStateCreateInfo *info)
{
    VkDynamicCbState state;
    VkResult result;

    result = vkCreateDynamicColorBlendState(dev, info, &state);

    t_assert(result == VK_SUCCESS);
    t_assert(state);
    t_cleanup_push_vk_dynamic_cb_state(dev, state);

    return state;
}

VkDynamicDsState
__qoCreateDynamicDepthStencilState(VkDevice dev,
                                   const VkDynamicDsStateCreateInfo *info)
{
    VkDynamicDsState state;
    VkResult result;

    result = vkCreateDynamicDepthStencilState(dev, info, &state);

    t_assert(result == VK_SUCCESS);
    t_assert(state);
    t_cleanup_push_vk_dynamic_ds_state(dev, state);

    return state;
}

VkCmdBuffer
__qoCreateCommandBuffer(VkDevice dev, const VkCmdBufferCreateInfo *info)
{
    VkCmdBuffer cmd;
    VkResult result;

    result = vkCreateCommandBuffer(dev, info, &cmd);

    t_assert(result == VK_SUCCESS);
    t_assert(cmd);
    t_cleanup_push_vk_command_buffer(dev, cmd);

    return cmd;
}

VkResult
__qoBeginCommandBuffer(VkCmdBuffer cmd, const VkCmdBufferBeginInfo *info)
{
    VkResult result;

    result = vkBeginCommandBuffer(cmd, info);
    t_assert(result == VK_SUCCESS);

    return result;
}

VkResult
__qoEndCommandBuffer(VkCmdBuffer cmd)
{
    VkResult result;

    result = vkEndCommandBuffer(cmd);
    t_assert(result == VK_SUCCESS);

    return result;
}

VkFramebuffer
__qoCreateFramebuffer(VkDevice dev, const VkFramebufferCreateInfo *info)
{
    VkResult result;
    VkFramebuffer fb;

    result = vkCreateFramebuffer(dev, info, &fb);

    t_assert(result == VK_SUCCESS);
    t_assert(fb);
    t_cleanup_push_vk_framebuffer(dev, fb);

    return fb;
}

VkRenderPass
__qoCreateRenderPass(VkDevice dev, const VkRenderPassCreateInfo *info)
{
    VkResult result;
    VkRenderPass pass;

    result = vkCreateRenderPass(dev, info, &pass);

    t_assert(result == VK_SUCCESS);
    t_assert(pass);
    t_cleanup_push_vk_render_pass(dev, pass);

    return pass;
}

VkResult __qoEndCommandBuffer(VkCmdBuffer cmd);

VkImage
__qoCreateImage(VkDevice dev, const VkImageCreateInfo *info)
{
    VkImage image;
    VkResult result;

    result = vkCreateImage(dev, info, &image);

    t_assert(result == VK_SUCCESS);
    t_assert(image);
    t_cleanup_push_vk_image(dev, image);

    return image;
}

VkImageView
__qoCreateImageView(VkDevice dev, const VkImageViewCreateInfo *info)
{
    VkImageView view;
    VkResult result;

    result = vkCreateImageView(dev, info, &view);

    t_assert(result == VK_SUCCESS);
    t_assert(view);
    t_cleanup_push_vk_image_view(dev, view);

    return view;
}

VkColorAttachmentView
__qoCreateColorAttachmentView(VkDevice dev, const VkColorAttachmentViewCreateInfo *info)
{
    VkColorAttachmentView view;
    VkResult result;

    result = vkCreateColorAttachmentView(dev, info, &view);

    t_assert(result == VK_SUCCESS);
    t_assert(view);
    t_cleanup_push_vk_color_attachment_view(dev, view);

    return view;
}

VkDepthStencilView
__qoCreateDepthStencilView(VkDevice dev, const VkDepthStencilViewCreateInfo *info)
{
    VkDepthStencilView view;
    VkResult result;

    result = vkCreateDepthStencilView(dev, info, &view);

    t_assert(result == VK_SUCCESS);
    t_assert(view);
    t_cleanup_push_vk_depth_stencil_view(dev, view);

    return view;
}

VkShader
__qoCreateShader(VkDevice dev, const QoShaderCreateInfo *info)
{
    VkShaderModule module;
    VkShader shader;
    VkResult result;

    VkShaderModuleCreateInfo module_info = {
        .sType =VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    };

    if (t_use_spir_v && info->spirvSize > 0) {
        assert(info->pSpirv != NULL);
        module_info.codeSize = info->spirvSize;
        module_info.pCode = info->pSpirv;
    } else if (info->glslSize > 0) {
        assert(info->pGlsl != NULL);
        module_info.codeSize = info->glslSize;
        module_info.pCode = info->pGlsl;
    } else {
        assert(info->spirvSize > 0 && info->pSpirv != NULL);
        module_info.codeSize = info->spirvSize;
        module_info.pCode = info->pSpirv;
    }

    result = vkCreateShaderModule(dev, &module_info, &module);

    t_assert(result == VK_SUCCESS);
    t_assert(module);
    t_cleanup_push_vk_shader_module(dev, module);

    result = vkCreateShader(dev,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .module = module,
            .pName = "main",
            .flags = 0,
        }, &shader);

    t_assert(result == VK_SUCCESS);
    t_assert(shader);
    t_cleanup_push_vk_shader(dev, shader);

    return shader;
}
