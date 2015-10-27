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

#include <string.h>

#include "framework/test/test.h"
#include "qonos/qonos.h"

void
qoEnumeratePhysicalDevices(VkInstance instance, uint32_t *count,
                           VkPhysicalDevice *physical_devices)
{
    VkResult result;

    result = vkEnumeratePhysicalDevices(instance, count, physical_devices);
    t_assert(result == VK_SUCCESS);
}

void
qoGetPhysicalDeviceProperties(VkPhysicalDevice physical_dev,
                              VkPhysicalDeviceProperties *properties)
{
    VkResult result;

    result = vkGetPhysicalDeviceProperties(physical_dev, properties);
    t_assert(result == VK_SUCCESS);
}

void
qoGetPhysicalDeviceMemoryProperties(
        VkPhysicalDevice physical_dev,
        VkPhysicalDeviceMemoryProperties *mem_props)
{
    VkResult result;

    result = vkGetPhysicalDeviceMemoryProperties(physical_dev, mem_props);
    t_assert(result == VK_SUCCESS);
}

VkMemoryRequirements
qoGetBufferMemoryRequirements(VkDevice dev, VkBuffer buffer)
{
    VkResult result;
    VkMemoryRequirements mem_reqs = {0};

    result = vkGetBufferMemoryRequirements(dev, buffer, &mem_reqs);
    t_assert(result == VK_SUCCESS);

    return mem_reqs;
}

VkMemoryRequirements
qoGetImageMemoryRequirements(VkDevice dev, VkImage image)
{
    VkResult result;
    VkMemoryRequirements mem_reqs = {0};

    result = vkGetImageMemoryRequirements(dev, image, &mem_reqs);
    t_assert(result == VK_SUCCESS);

    return mem_reqs;
}

VkResult
qoBindBufferMemory(VkDevice device, VkBuffer buffer,
                   VkDeviceMemory mem, VkDeviceSize offset)
{
    VkResult result;

    result = vkBindBufferMemory(device, buffer, mem, offset);
    t_assert(result == VK_SUCCESS);

    return result;
}

VkResult
qoBindImageMemory(VkDevice device, VkImage image,
                  VkDeviceMemory mem, VkDeviceSize offset)
{
    VkResult result;

    result = vkBindImageMemory(device, image, mem, offset);
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
    VkDeviceMemory memory = {0};
    VkResult result;

    t_assert(info->memoryTypeIndex != QO_MEMORY_TYPE_INDEX_INVALID);

    result = vkAllocMemory(dev, info, &memory);

    t_assert(result == VK_SUCCESS);
    t_assert(memory.handle);
    t_cleanup_push_vk_device_memory(dev, memory);

    return memory;
}

VkDeviceMemory
__qoAllocMemoryFromRequirements(VkDevice dev,
                                const VkMemoryRequirements *mem_reqs,
                                const VkMemoryAllocInfo *override_info)
{
    VkMemoryAllocInfo info = *override_info;

    if (info.allocationSize == 0)
        info.allocationSize = mem_reqs->size;

    t_assert(info.allocationSize >= mem_reqs->size);
    t_assert(info.memoryTypeIndex != QO_MEMORY_TYPE_INDEX_INVALID);
    t_assert((1 << info.memoryTypeIndex) & mem_reqs->memoryTypeBits);

    return __qoAllocMemory(dev, &info);
}

VkDeviceMemory
__qoAllocBufferMemory(VkDevice dev, VkBuffer buffer,
                      const VkMemoryAllocInfo *override_info)
{
    VkMemoryRequirements mem_reqs =
        qoGetBufferMemoryRequirements(dev, buffer);

    return __qoAllocMemoryFromRequirements(dev, &mem_reqs, override_info);
}

VkDeviceMemory
__qoAllocImageMemory(VkDevice dev, VkImage image,
                     const VkMemoryAllocInfo *override_info)
{
    VkMemoryRequirements mem_reqs =
        qoGetImageMemoryRequirements(dev, image);

    return __qoAllocMemoryFromRequirements(dev, &mem_reqs, override_info);
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
    t_cleanup_push_vk_device_memory_map(dev, mem);

    return map;
}

VkPipelineCache
__qoCreatePipelineCache(VkDevice dev, const VkPipelineCacheCreateInfo *info)
{
    VkPipelineCache pipeline_cache = {0};
    VkResult result;

    result = vkCreatePipelineCache(dev, info, &pipeline_cache);

    t_assert(result == VK_SUCCESS);
    t_assert(pipeline_cache.handle);
    t_cleanup_push_vk_pipeline_cache(dev, pipeline_cache);

    return pipeline_cache;
}

VkPipelineLayout
__qoCreatePipelineLayout(VkDevice dev, const VkPipelineLayoutCreateInfo *info)
{
    VkPipelineLayout pipeline_layout = {0};
    VkResult result;

    result = vkCreatePipelineLayout(dev, info, &pipeline_layout);

    t_assert(result == VK_SUCCESS);
    t_assert(pipeline_layout.handle);
    t_cleanup_push_vk_pipeline_layout(dev, pipeline_layout);

    return pipeline_layout;
}

VkSampler
__qoCreateSampler(VkDevice dev, const VkSamplerCreateInfo *info)
{
    VkSampler sampler = {0};
    VkResult result;

    result = vkCreateSampler(dev, info, &sampler);

    t_assert(result == VK_SUCCESS);
    t_assert(sampler.handle);
    t_cleanup_push_vk_sampler(dev, sampler);

    return sampler;
}

VkDescriptorSetLayout
__qoCreateDescriptorSetLayout(VkDevice dev,
                              const VkDescriptorSetLayoutCreateInfo *info)
{
    VkDescriptorSetLayout layout = {0};
    VkResult result;

    result = vkCreateDescriptorSetLayout(dev, info, &layout);

    t_assert(result == VK_SUCCESS);
    t_assert(layout.handle);
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

    memset(sets, 0, count * sizeof(sets[0]));
    result = vkAllocDescriptorSets(dev, descriptorPool, usage, count, layouts,
                                   sets);
    t_assert(result == VK_SUCCESS);

    for (uint32_t i = 0; i < count; ++i) {
        t_assert(sets[i].handle);
        t_cleanup_push_vk_descriptor_set(dev, descriptorPool, sets[i]);
    }

    return result;
}

VkBuffer
__qoCreateBuffer(VkDevice dev, const VkBufferCreateInfo *info)
{
    VkBuffer buffer = {0};
    VkResult result;

    result = vkCreateBuffer(dev, info, &buffer);

    t_assert(result == VK_SUCCESS);
    t_assert(buffer.handle);
    t_cleanup_push_vk_buffer(dev, buffer);

    return buffer;
}

VkBufferView
__qoCreateBufferView(VkDevice dev, const VkBufferViewCreateInfo *info)
{
    VkBufferView view = {0};
    VkResult result;

    result = vkCreateBufferView(dev, info, &view);

    t_assert(result == VK_SUCCESS);
    t_assert(view.handle);
    t_cleanup_push_vk_buffer_view(dev, view);

    return view;
}

VkCmdBuffer
__qoCreateCommandBuffer(VkDevice dev, VkCmdPool pool,
                        const VkCmdBufferCreateInfo *info)
{
    VkCmdBuffer cmd = QO_NULL_CMD_BUFFER;
    VkResult result;

    assert(memcmp(&info->cmdPool, &pool, sizeof(pool)) == 0);

    result = vkCreateCommandBuffer(dev, info, &cmd);

    t_assert(result == VK_SUCCESS);
    t_assert(cmd);
    t_cleanup_push_vk_cmd_buffer(dev, cmd);

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
    VkFramebuffer fb = {0};
    VkResult result;

    result = vkCreateFramebuffer(dev, info, &fb);

    t_assert(result == VK_SUCCESS);
    t_assert(fb.handle);
    t_cleanup_push_vk_framebuffer(dev, fb);

    return fb;
}

VkRenderPass
__qoCreateRenderPass(VkDevice dev, const VkRenderPassCreateInfo *info)
{
    VkRenderPass pass = {0};
    VkResult result;

    result = vkCreateRenderPass(dev, info, &pass);

    t_assert(result == VK_SUCCESS);
    t_assert(pass.handle);
    t_cleanup_push_vk_render_pass(dev, pass);

    return pass;
}

VkResult __qoEndCommandBuffer(VkCmdBuffer cmd);

VkImage
__qoCreateImage(VkDevice dev, const VkImageCreateInfo *info)
{
    VkImage image = {0};
    VkResult result;

    result = vkCreateImage(dev, info, &image);

    t_assert(result == VK_SUCCESS);
    t_assert(image.handle);
    t_cleanup_push_vk_image(dev, image);

    return image;
}

VkImageView
__qoCreateImageView(VkDevice dev, const VkImageViewCreateInfo *info)
{
    VkImageView view = {0};
    VkResult result;

    result = vkCreateImageView(dev, info, &view);

    t_assert(result == VK_SUCCESS);
    t_assert(view.handle);
    t_cleanup_push_vk_image_view(dev, view);

    return view;
}

VkShader
__qoCreateShader(VkDevice dev, const QoShaderCreateInfo *info)
{
    VkShaderModule module = {0};
    VkShader shader = {0};
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
    t_assert(module.handle);
    t_cleanup_push_vk_shader_module(dev, module);

    result = vkCreateShader(dev,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .module = module,
            .pName = "main",
            .flags = 0,
            .stage = info->stage,
        }, &shader);

    t_assert(result == VK_SUCCESS);
    t_assert(shader.handle);
    t_cleanup_push_vk_shader(dev, shader);

    return shader;
}
