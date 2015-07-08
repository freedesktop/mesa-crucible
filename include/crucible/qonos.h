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
///     VkDynamicDsState state =
///         qoCreateDynamicDepthStencilState(device, .stencilWriteMask = 0x17);
///
///     // Example 2:
///     // Create state using vkCreateDynamicDepthStencilState, but use
///     // QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS to set sensible defaults.
///     VkDynamicDsState state;
///     VkDynamicDsStateCreateInfo info = {
///         QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS,
///         .stencilWriteMask = 0x17,
///     };
///     vkCreateDynamicDepthStencilState(device, &info, &state);
///
///     // Example 3:
///     // Create state using the raw Vulkan API.
///     VkDynamicDsState state;
///     VkDynamicDsStateCreateInfo info = {
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

#include <crucible/vk_wrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QoExtraGraphicsPipelineCreateInfo_ {
    VkGraphicsPipelineCreateInfo *pNext;
    VkPrimitiveTopology topology;
    VkShader vertexShader;
    VkShader fragmentShader;
} QoExtraGraphicsPipelineCreateInfo;

typedef struct QoShaderCreateInfo_ {
    void *pNext;
    size_t spirvSize;
    const void *pSpirv;
    size_t glslSize;
    const char *pGlsl;
} QoShaderCreateInfo;

#define QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS \
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

#define QO_MEMORY_ALLOC_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO, \
    .memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT

#define QO_BUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO

#define QO_BUFFER_VIEW_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO

#define QO_PIPELINE_LAYOUT_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, \
    .descriptorSetCount = 0, \
    .pSetLayouts = NULL

#define QO_SAMPLER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO

#define QO_DESCRIPTOR_SET_LAYOUT_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO

#define QO_PIPELINE_IA_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_IA_STATE_CREATE_INFO, \
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, \
    .primitiveRestartEnable = false

#define QO_PIPELINE_RS_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RS_STATE_CREATE_INFO, \
    .depthClipEnable = false, \
    .rasterizerDiscardEnable = false, \
    .fillMode = VK_FILL_MODE_SOLID, \
    .cullMode = VK_CULL_MODE_NONE, \
    .frontFace = VK_FRONT_FACE_CCW

#define QO_PIPELINE_MS_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MS_STATE_CREATE_INFO, \
    .samples = 1, \
    .multisampleEnable = false, \
    .sampleMask = 0xffffffff

#define QO_PIPELINE_CB_ATTACHMENT_STATE_DEFAULTS \
    .blendEnable = false, \
    .format = VK_FORMAT_R8G8B8A8_UNORM, \
    .channelWriteMask = (VK_CHANNEL_R_BIT | VK_CHANNEL_G_BIT | \
                         VK_CHANNEL_B_BIT | VK_CHANNEL_A_BIT)

#define QO_PIPELINE_CB_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_CB_STATE_CREATE_INFO, \
    .attachmentCount = 1, \
    .pAttachments = (VkPipelineCbAttachmentState []) { \
       { QO_PIPELINE_CB_ATTACHMENT_STATE_DEFAULTS }, \
    }

#define QO_DYNAMIC_VP_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_VP_STATE_CREATE_INFO, \
    .viewportAndScissorCount = 0

#define QO_DYNAMIC_RS_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_RS_STATE_CREATE_INFO, \
    .depthBias = 0.0f, \
    .depthBiasClamp = 0.0f, \
    .slopeScaledDepthBias = 0.0f, \
    .lineWidth = 1.0f

#define QO_DYNAMIC_CB_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_CB_STATE_CREATE_INFO, \
    .blendConst = {0.0f, 0.0f, 0.0f, 0.0f} /* default in OpenGL ES 3.1 */

#define QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_DS_STATE_CREATE_INFO, \
    .minDepthBounds = 0.0f, /* default in OpenGL ES 3.1 */ \
    .maxDepthBounds = 1.0f, /* default in OpenGL ES 3.1 */ \
    .stencilReadMask = ~0,  /* default in OpenGL ES 3.1 */ \
    .stencilWriteMask = ~0, /* default in OpenGL ES 3.1 */ \
    .stencilFrontRef = 0,   /* default in OpenGL ES 3.1 */ \
    .stencilBackRef = 0     /* default in OpenGL ES 3.1 */

#define QO_CMD_BUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO, \
    .level = VK_CMD_BUFFER_LEVEL_PRIMARY

#define QO_CMD_BUFFER_BEGIN_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO

#define QO_FRAMEBUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, \
    .colorAttachmentCount = 1, \
    .pColorAttachments = NULL, \
    .pDepthStencilAttachment = NULL, \
    .sampleCount = 1, \
    .layers = 1

#define QO_RENDER_PASS_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, \
    .colorAttachmentCount = 1, \
    .sampleCount = 1, \
    .layers = 1, \
    .extent = {0}, \
    .depthStencilFormat = VK_FORMAT_UNDEFINED, \
    .depthStencilLayout = 0

#define QO_IMAGE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, \
    .imageType = VK_IMAGE_TYPE_2D, \
    .tiling = VK_IMAGE_TILING_OPTIMAL, \
    .usage = VK_IMAGE_USAGE_GENERAL, \
    .mipLevels = 1, \
    .arraySize = 1, \
    .samples = 1

#define QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, \
    .minLod = 0.0f, \
    .channels = { \
        VK_CHANNEL_SWIZZLE_R, \
        VK_CHANNEL_SWIZZLE_G, \
        VK_CHANNEL_SWIZZLE_B, \
        VK_CHANNEL_SWIZZLE_A, \
    }, \
    .subresourceRange = { \
        .aspect = VK_IMAGE_ASPECT_COLOR, \
        .baseMipLevel = 0, \
        .mipLevels = 1, \
        .baseArraySlice = 0, \
        .arraySize = 1, \
    }

#define QO_COLOR_ATTACHMENT_VIEW_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_COLOR_ATTACHMENT_VIEW_CREATE_INFO, \
    .mipLevel = 0, \
    .baseArraySlice = 0, \
    .arraySize = 1, \
    .msaaResolveImage = 0, \
    .msaaResolveSubResource = {0}

#define QO_DEPTH_STENCIL_VIEW_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DEPTH_STENCIL_VIEW_CREATE_INFO, \
    .mipLevel = 0, \
    .baseArraySlice = 0, \
    .arraySize = 1

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
VkDynamicVpStateCreateInfo qoCreateDynamicVpState(VkDevice dev, ...);
#else
#define qoCreateDynamicViewportState(dev, ...) \
    __qoCreateDynamicViewportState(dev, \
        &(VkDynamicVpStateCreateInfo) { \
            QO_DYNAMIC_VP_STATE_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDynamicRsStateCreateInfo qoCreateDynamicRasterState(VkDevice dev, ...);
#else
#define qoCreateDynamicRasterState(dev, ...) \
    __qoCreateDynamicRasterState(dev, \
        &(VkDynamicRsStateCreateInfo) { \
            QO_DYNAMIC_RS_STATE_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDynamicCbStateCreateInfo qoCreateDynamicColorBlendState(VkDevice dev, ...);
#else
#define qoCreateDynamicColorBlendState(dev, ...) \
    __qoCreateDynamicColorBlendState(dev, \
        &(VkDynamicCbStateCreateInfo) { \
            QO_DYNAMIC_CB_STATE_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDynamicDsStateCreateInfo qoCreateDynamicDepthStencilState(VkDevice dev, ...);
#else
#define qoCreateDynamicDepthStencilState(dev, ...) \
    __qoCreateDynamicDepthStencilState(dev, \
        &(VkDynamicDsStateCreateInfo) { \
            QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkCmdBuffer qoCreateCommandBuffer(VkDevice dev, ...);
#else
#define qoCreateCommandBuffer(dev, ...) \
    __qoCreateCommandBuffer(dev, \
        &(VkCmdBufferCreateInfo) { \
            QO_CMD_BUFFER_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkResult qoBeginCommandBuffer(VkCmdBuffer cmd, ...);
#else
#define qoBeginCommandBuffer(cmd, ...) \
    __qoBeginCommandBuffer(cmd, \
        &(VkCmdBufferBeginInfo) { \
            QO_CMD_BUFFER_BEGIN_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
    })
#endif

#ifdef DOXYGEN
VkResult qoEndCommandBuffer(VkCmdBuffer cmd);
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
VkColorAttachmentView qoCreateColorAttachmentView(VkDevice dev, ...);
#else
#define qoCreateColorAttachmentView(dev, ...) \
    __qoCreateColorAttachmentView(dev, \
        &(VkColorAttachmentViewCreateInfo) { \
            QO_COLOR_ATTACHMENT_VIEW_CREATE_INFO_DEFAULTS, \
            ##__VA_ARGS__, \
        })
#endif

#ifdef DOXYGEN
VkDepthStencilView qoCreateDepthStencilView(VkDevice dev, ...);
#else
#define qoCreateDepthStencilView(dev, ...) \
    __qoCreateDepthStencilView(dev, \
        &(VkDepthStencilViewCreateInfo) { \
            QO_DEPTH_STENCIL_VIEW_CREATE_INFO_DEFAULTS, \
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

VkResult qoQueueSubmit(VkQueue queue, uint32_t cmdBufferCount, const VkCmdBuffer *cmdBuffers, VkFence fence);
VkDeviceMemory __qoAllocMemory(VkDevice dev, const VkMemoryAllocInfo *info);
VkBuffer __qoCreateBuffer(VkDevice dev, const VkBufferCreateInfo *info);
VkBufferView __qoCreateBufferView(VkDevice dev, const VkBufferViewCreateInfo *info);
VkPipelineLayout __qoCreatePipelineLayout(VkDevice dev, const VkPipelineLayoutCreateInfo *info);
VkSampler __qoCreateSampler(VkDevice dev, const VkSamplerCreateInfo *info);
VkDescriptorSetLayout __qoCreateDescriptorSetLayout(VkDevice dev, const VkDescriptorSetLayoutCreateInfo *info);
VkResult qoAllocDescriptorSets(VkDevice dev, VkDescriptorPool descriptorPool,
                               VkDescriptorSetUsage usage, uint32_t count,
                               const VkDescriptorSetLayout *layouts,
                               VkDescriptorSet *sets);
VkDynamicVpState __qoCreateDynamicViewportState(VkDevice dev, const VkDynamicVpStateCreateInfo *info);
VkDynamicRsState __qoCreateDynamicRasterState(VkDevice dev, const VkDynamicRsStateCreateInfo *info);
VkDynamicCbState __qoCreateDynamicColorBlendState(VkDevice dev, const VkDynamicCbStateCreateInfo *info);
VkDynamicDsState __qoCreateDynamicDepthStencilState(VkDevice dev, const VkDynamicDsStateCreateInfo *info);
VkCmdBuffer __qoCreateCommandBuffer(VkDevice dev, const VkCmdBufferCreateInfo *info);
VkResult __qoBeginCommandBuffer(VkCmdBuffer cmd, const VkCmdBufferBeginInfo *info);
VkResult __qoEndCommandBuffer(VkCmdBuffer cmd);
VkFramebuffer __qoCreateFramebuffer(VkDevice dev, const VkFramebufferCreateInfo *info);
VkRenderPass __qoCreateRenderPass(VkDevice dev, const VkRenderPassCreateInfo *info);

VkPipeline qoCreateGraphicsPipeline(VkDevice dev, const QoExtraGraphicsPipelineCreateInfo *info);
VkImage __qoCreateImage(VkDevice dev, const VkImageCreateInfo *info);
VkImageView __qoCreateImageView(VkDevice dev, const VkImageViewCreateInfo *info);
VkColorAttachmentView __qoCreateColorAttachmentView(VkDevice dev, const VkColorAttachmentViewCreateInfo *info);
VkDepthStencilView __qoCreateDepthStencilView(VkDevice dev, const VkDepthStencilViewCreateInfo *info);
VkShader __qoCreateShader(VkDevice dev, const QoShaderCreateInfo *info);

#ifdef __cplusplus
}
#endif
