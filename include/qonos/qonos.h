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
/// \brief Vulkan wrappers from the planet Qo'noS.
///
/// The Qonos functions will fail the current test if the wrapped Vulkan
/// function fails. However, the Qonos functions do not require that a test be
/// running. They are safe to use inside and outside tests.
///
/// \section Conventions for Info Struct Parameters
///
/// If the signature a Vulkan function, say vkCreateFoo(), contains an info
/// struct parameter of type VkFooCreateInfo, then in the signature of its
/// Qonos wrapper, qoCreateFoo(), the struct is expanded to inline named
/// parameters. The wrapper assigns a sensible default value to each named
/// parameter. The default values are defined by macro
/// QO_FOO_CREATE_INFO_DEFAULTS.
///
/// For example, the following are approximately equivalent:
///
///     // Example 1
///     // Create state using qoCreateDynamicDepthStencilState. We need only
///     // set a named parameter if it deviates from its default value.
///     VkDynamicDepthStencilState state =
///         qoCreateDynamicDepthStencilState(device, .stencilWriteMask = 0x17);
///
///     // Example 2:
///     // Create state using vkCreateDynamicDepthStencilState, but use
///     // QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS to set sensible defaults.
///     VkDynamicDepthStencilState state;
///     VkDynamicDepthStencilStateCreateInfo info = {
///         QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS,
///         .stencilWriteMask = 0x17,
///     };
///     vkCreateDynamicDepthStencilState(device, &info, &state);
///
///     // Example 3:
///     // Create state using the raw Vulkan API.
///     VkDynamicDepthStencilState state;
///     VkDynamicDepthStencilStateCreateInfo info = {
///             .sType = VK_STRUCTURE_TYPE_DYNAMIC_DS_STATE_CREATE_INFO,
///             .minDepth = 0.0f,           // OpenGL default
///             .maxDepth = 1.0f,           // OpenGL default
///             .stencilReadMask = ~0,      // OpenGL default
///             .stencilWriteMask = 0x17,   // NOT default
///             .stencilFrontRef = 0,       // OpenGL default
///             .stencilBackRef = 0,        // OpenGL default
///     };
///     vkCreateDynamicDepthStencilState(device, &info, &state);
///
///
/// \section Implementation Details: Trailing commas
///
/// A syntax error occurs if a comma follows the last argument of a function call.
/// For example:
///
///     ok:     printf("%d + %d == %d\n", 1, 2, 3);
///     error:  printf("%d + %d == %d\n", 1, 2, 3,);
///
/// Observe that the definitions of the variadic function macros in this header
/// expand `...` to `##__VA_ARGS,`. The trailing comma is significant.  It
/// ensures that, just as for real function calls, a syntax error occurs if
/// a comma follows the last argument passed to the macro.
///
///     ok:     qoCreateBuffer(dev, .size=4096);
///     error:  qoCreateBuffer(dev, .size=4096,);

#pragma once

#include "util/vk_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QO_MEMORY_TYPE_INDEX_INVALID UINT32_MAX

typedef struct QoExtraGraphicsPipelineCreateInfo_ {
    VkGraphicsPipelineCreateInfo *pNext;
    VkPrimitiveTopology topology;
    VkShader vertexShader;
    VkShader geometryShader;
    VkShader fragmentShader;
    uint32_t dynamicStates; // Bitfield
} QoExtraGraphicsPipelineCreateInfo;

typedef struct QoShaderCreateInfo_ {
    void *pNext;
    size_t spirvSize;
    const void *pSpirv;
    size_t glslSize;
    const char *pGlsl;
    VkShaderStage stage;
} QoShaderCreateInfo;

#define QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS \
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

#define QO_MEMORY_ALLOC_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO, \
    .memoryTypeIndex = QO_MEMORY_TYPE_INDEX_INVALID

#define QO_BUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO

#define QO_BUFFER_VIEW_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO

#define QO_PIPELINE_CACHE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, \
    .initialDataSize = 0, \
    .pInitialData = NULL

#define QO_PIPELINE_LAYOUT_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, \
    .setLayoutCount = 0, \
    .pSetLayouts = NULL

#define QO_SAMPLER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO

#define QO_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO

#define QO_DESCRIPTOR_SET_ALLOCATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, \
    .descriptorPool = VK_NULL_HANDLE, \
    .setLayoutCount = 1

#define QO_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, \
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, \
    .primitiveRestartEnable = false

#define QO_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, \
    .depthClipEnable = false, \
    .rasterizerDiscardEnable = false, \
    .polygonMode = VK_POLYGON_MODE_FILL, \
    .cullMode = VK_CULL_MODE_NONE, \
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE, \
    .depthBiasConstantFactor = 0.0f, \
    .depthBiasClamp = 0.0f, \
    .depthBiasSlopeFactor = 0.0f, \
    .lineWidth = 1.0f

#define QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, \
    .depthTestEnable = false, \
    .depthWriteEnable = false, \
    .depthBoundsTestEnable = false, \
    .stencilTestEnable = false, \
    .front = { \
        .compareMask = ~0,  /* default in OpenGL ES 3.1 */ \
        .writeMask = ~0,    /* default in OpenGL ES 3.1 */ \
        .reference = 0,     /* default in OpenGL ES 3.1 */ \
    }, \
    .back = { \
        .compareMask = ~0,  /* default in OpenGL ES 3.1 */ \
        .writeMask = ~0,    /* default in OpenGL ES 3.1 */ \
        .reference = 0,     /* default in OpenGL ES 3.1 */ \
    }, \
    .minDepthBounds = 0.0f, /* default in OpenGL ES 3.1 */ \
    .maxDepthBounds = 1.0f  /* default in OpenGL ES 3.1 */ \

#define QO_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, \
    .rasterizationSamples = 1, \
    .pSampleMask = NULL

#define QO_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_DEFAULTS \
    .blendEnable = false, \
    .colorWriteMask = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | \
                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)

#define QO_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, \
    .attachmentCount = 1, \
    .pAttachments = (VkPipelineColorBlendAttachmentState []) { \
       { QO_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_DEFAULTS }, \
    }, \
    .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f} /* default in OpenGL ES 3.1 */

#define QO_COMMAND_BUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_CREATE_INFO, \
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY

#define QO_COMMAND_BUFFER_BEGIN_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO

#define QO_ATTACHMENT_DESCRIPTION_DEFAULTS \
    .samples = 1, \
    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, \
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE, \
    .initialLayout = VK_IMAGE_LAYOUT_GENERAL, \
    .finalLayout = VK_IMAGE_LAYOUT_GENERAL

#define QO_SUBPASS_DESCRIPTION_DEFAULTS \
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, \
    .inputAttachmentCount = 0, \
    .pInputAttachments = NULL, \
    .pResolveAttachments = NULL, \
    .pDepthStencilAttachment = NULL, \
    .preserveAttachmentCount = 0, \
    .pPreserveAttachments = NULL

#define QO_FRAMEBUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, \
    .layers = 1

#define QO_RENDER_PASS_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, \
    .dependencyCount = 0, \
    .pDependencies = NULL

#define QO_IMAGE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, \
    .imageType = VK_IMAGE_TYPE_2D, \
    .tiling = VK_IMAGE_TILING_OPTIMAL, \
    .usage = 0, \
    .mipLevels = 1, \
    .arrayLayers = 1, \
    .samples = 1

#define QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, \
    .subresourceRange = { \
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, \
        .baseMipLevel = 0, \
        .levelCount = 1, \
        .baseArrayLayer = 0, \
        .layerCount = 1, \
    }

#define QO_SHADER_CREATE_INFO_DEFAULTS \
    .sType =VK_STRUCTURE_TYPE_SHADER_CREATE_INFO

VkMemoryRequirements qoGetBufferMemoryRequirements(VkDevice dev, VkBuffer buffer);
VkMemoryRequirements qoGetImageMemoryRequirements(VkDevice dev, VkImage image);

VkResult qoBindBufferMemory(VkDevice device, VkBuffer buffer,
                            VkDeviceMemory mem, VkDeviceSize offset);
VkResult qoBindImageMemory(VkDevice device, VkImage img,
                           VkDeviceMemory mem, VkDeviceSize offset);

#ifdef DOXYGEN
VkDeviceMemory qoAllocMemory(VkDevice dev, ...);
#else
#define qoAllocMemory(dev, ...) \
    __qoAllocMemory(dev, \
        &(VkMemoryAllocInfo) { \
            QO_MEMORY_ALLOC_INFO_DEFAULTS, \
            ##__VA_ARGS__ , \
        })
#endif

#ifdef DOXYGEN
VkDeviceMemory
qoAllocMemoryFromRequirements(VkDevice dev,
                              const VkMemoryRequirements *mem_reqs,
                              const VkMemoryAllocInfo *va_args override_info);
#else
#define qoAllocMemoryFromRequirements(dev, mem_reqs, ...) \
    __qoAllocMemoryFromRequirements((dev), (mem_reqs), \
        &(VkMemoryAllocInfo) { \
            QO_MEMORY_ALLOC_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDeviceMemory
qoAllocBufferMemory(VkDevice dev, VkBuffer buffer,
                    const VkMemoryAllocInfo *va_args override_info);
#else
#define qoAllocBufferMemory(dev, buffer, ...) \
    __qoAllocBufferMemory((dev), (buffer), \
        &(VkMemoryAllocInfo) { \
            QO_MEMORY_ALLOC_INFO_DEFAULTS, \
            ##__VA_ARGS__ , \
        })
#endif

#ifdef DOXYGEN
VkDeviceMemory
qoAllocImageMemory(VkDevice dev, VkImage image,
                    const VkMemoryAllocInfo *va_args override_info);
#else
#define qoAllocImageMemory(dev, image, ...) \
    __qoAllocImageMemory((dev), (image), \
        &(VkMemoryAllocInfo) { \
            QO_MEMORY_ALLOC_INFO_DEFAULTS, \
            ##__VA_ARGS__ , \
        })
#endif

void *qoMapMemory(VkDevice dev, VkDeviceMemory mem,
                  VkDeviceSize offset, VkDeviceSize size,
                  VkMemoryMapFlags flags);

#ifdef DOXYGEN
VkBuffer qoCreateBuffer(VkDevice dev, ...);
#else
#define qoCreateBuffer(dev, ...) \
    __qoCreateBuffer(dev, \
        &(VkBufferCreateInfo) { \
            QO_BUFFER_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__ , \
        })
#endif

#ifdef DOXYGEN
VkBufferView qoCreateBufferView(VkDevice dev, ...);
#else
#define qoCreateBufferView(dev, ...) \
    __qoCreateBufferView(dev, \
        &(VkBufferViewCreateInfo) { \
            QO_BUFFER_VIEW_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__ , \
        })
#endif

#ifdef DOXYGEN
VkPipelineCacheCreateInfo qoCreatePipelineCache(VkDevice dev, ...);
#else
#define qoCreatePipelineCache(dev, ...) \
    __qoCreatePipelineCache(dev, \
        &(VkPipelineCacheCreateInfo) { \
            QO_PIPELINE_CACHE_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
    })
#endif

#ifdef DOXYGEN
VkPipelineLayout qoCreatePipelineLayout(VkDevice dev, ...);
#else
#define qoCreatePipelineLayout(dev, ...) \
    __qoCreatePipelineLayout(dev, \
        &(VkPipelineLayoutCreateInfo) { \
            QO_PIPELINE_LAYOUT_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
    })
#endif

#ifdef DOXYGEN
VkSampler qoCreateSampler(VkDevice dev, ...);
#else
#define qoCreateSampler(dev, ...) \
    __qoCreateSampler(dev, \
        &(VkSamplerCreateInfo) { \
            QO_SAMPLER_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDescriptorSetLayout qoCreateDescriptorSetLayout(VkDevice dev, ...);
#else
#define qoCreateDescriptorSetLayout(dev, ...) \
    __qoCreateDescriptorSetLayout(dev, \
        &(VkDescriptorSetLayoutCreateInfo) { \
            QO_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDescriptorSet qoAllocateDescriptorSet(VkDevice dev, ...);
#else
#define qoAllocateDescriptorSet(dev, ...) \
    __qoAllocateDescriptorSet(dev, \
        &(VkDescriptorSetAllocateInfo) { \
            QO_DESCRIPTOR_SET_ALLOCATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkCommandBuffer qoCreateCommandBuffer(VkDevice dev, VkCommandPool pool, ...);
#else
#define qoCreateCommandBuffer(dev, pool, ...) \
    __qoCreateCommandBuffer(dev, pool, \
        &(VkCommandBufferCreateInfo) { \
            QO_COMMAND_BUFFER_CREATE_INFO_DEFAULTS, \
            .commandPool = pool, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkResult qoBeginCommandBuffer(VkCommandBuffer cmd, ...);
#else
#define qoBeginCommandBuffer(cmd, ...) \
    __qoBeginCommandBuffer(cmd, \
        &(VkCommandBufferBeginInfo) { \
            QO_COMMAND_BUFFER_BEGIN_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
    })
#endif

#ifdef DOXYGEN
VkResult qoEndCommandBuffer(VkCommandBuffer cmd);
#else
#define qoEndCommandBuffer(cmd) __qoEndCommandBuffer(cmd)
#endif

#ifdef DOXYGEN
VkFramebuffer qoCreateFramebuffer(VkDevice dev, ...);
#else
#define qoCreateFramebuffer(dev, ...) \
    __qoCreateFramebuffer(dev, \
        &(VkFramebufferCreateInfo) { \
            QO_FRAMEBUFFER_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkRenderPass qoCreateRenderPass(dev, ...);
#else
#define qoCreateRenderPass(dev, ...) \
    __qoCreateRenderPass(dev, \
        &(VkRenderPassCreateInfo) { \
            QO_RENDER_PASS_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
    })
#endif

#ifdef DOXYGEN
VkImage qoCreateImage(VkDevice dev, ...);
#else
#define qoCreateImage(dev, ...) \
    __qoCreateImage(dev, \
        &(VkImageCreateInfo) { \
            QO_IMAGE_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__ , \
        })
#endif

#ifdef DOXYGEN
VkImageView qoCreateImageView(VkDevice dev, ...);
#else
#define qoCreateImageView(dev, ...) \
    __qoCreateImageView(dev, \
        &(VkImageViewCreateInfo) { \
            QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
    })
#endif

#ifdef DOXYGEN
VkShader qoCreateShader(VkDevice dev, ...);
#else
#define qoCreateShader(dev, ...) \
    __qoCreateShader(dev, \
        &(QoShaderCreateInfo) { \
            .pNext = NULL, \
            ##__VA_ARGS__, \
        })
#endif

void qoEnumeratePhysicalDevices(VkInstance instance, uint32_t *count, VkPhysicalDevice *physical_devices);
void qoGetPhysicalDeviceProperties(VkPhysicalDevice physical_dev, VkPhysicalDeviceProperties *properties);
void qoGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physical_dev, VkPhysicalDeviceMemoryProperties *mem_props);
VkResult qoQueueSubmit(VkQueue queue, uint32_t cmdBufferCount, const VkCommandBuffer *cmdBuffers, VkFence fence);
VkDeviceMemory __qoAllocMemory(VkDevice dev, const VkMemoryAllocInfo *info);
VkDeviceMemory __qoAllocMemoryFromRequirements(VkDevice dev, const VkMemoryRequirements *mem_reqs, const VkMemoryAllocInfo *override_info);
VkDeviceMemory __qoAllocBufferMemory(VkDevice dev, VkBuffer buffer, const VkMemoryAllocInfo *override_info);
VkDeviceMemory __qoAllocImageMemory(VkDevice dev, VkImage image, const VkMemoryAllocInfo *override_info);
VkBuffer __qoCreateBuffer(VkDevice dev, const VkBufferCreateInfo *info);
VkBufferView __qoCreateBufferView(VkDevice dev, const VkBufferViewCreateInfo *info);
VkPipelineCache __qoCreatePipelineCache(VkDevice dev, const VkPipelineCacheCreateInfo *info);
VkPipelineLayout __qoCreatePipelineLayout(VkDevice dev, const VkPipelineLayoutCreateInfo *info);
VkSampler __qoCreateSampler(VkDevice dev, const VkSamplerCreateInfo *info);
VkDescriptorSetLayout __qoCreateDescriptorSetLayout(VkDevice dev, const VkDescriptorSetLayoutCreateInfo *info);
VkDescriptorSet __qoAllocateDescriptorSet(VkDevice dev, const VkDescriptorSetAllocateInfo *info);
VkCommandBuffer __qoCreateCommandBuffer(VkDevice dev, VkCommandPool pool, const VkCommandBufferCreateInfo *info);
VkResult __qoBeginCommandBuffer(VkCommandBuffer cmd, const VkCommandBufferBeginInfo *info);
VkResult __qoEndCommandBuffer(VkCommandBuffer cmd);
VkFramebuffer __qoCreateFramebuffer(VkDevice dev, const VkFramebufferCreateInfo *info);
VkRenderPass __qoCreateRenderPass(VkDevice dev, const VkRenderPassCreateInfo *info);

VkPipeline qoCreateGraphicsPipeline(VkDevice dev,
                                    VkPipelineCache pipeline_cache,
                                    const QoExtraGraphicsPipelineCreateInfo *info);
VkImage __qoCreateImage(VkDevice dev, const VkImageCreateInfo *info);
VkImageView __qoCreateImageView(VkDevice dev, const VkImageViewCreateInfo *info);
VkShader __qoCreateShader(VkDevice dev, const QoShaderCreateInfo *info);

#ifdef __cplusplus
}
#endif
