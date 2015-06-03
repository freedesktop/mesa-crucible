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
/// parameter.
///
/// To provide finer control of vkCreateFoo's wrapping, Qonos defines two
/// additional functions: qoCreateFooS() and qoFooCreateInfo(). Unlike
/// qoCreateFoo(), the signature of qoCreateFooS() contains the info struct
/// parameter. Function qoFooCreateInfo() creates the info struct, populated
/// with the same default values as qoCreateFoo().
///
/// For example, the following are approximately equivalent:
///
///     // Example 1
///     //
///     // Create state using the raw Vulkan API.
///     VkDynamicDsState state1;
///     VkDynamicDsStateCreateInfo info1 = {
///             .sType = VK_STRUCTURE_TYPE_DYNAMIC_DS_STATE_CREATE_INFO,
///             .minDepth = 0.0f,           // OpenGL default
///             .maxDepth = 1.0f,           // OpenGL default
///             .stencilReadMask = ~0,      // OpenGL default
///             .stencilWriteMask = 0x17,   // NOT default
///             .stencilFrontRef = 0,       // OpenGL default
///             .stencilBackRef = 0,        // OpenGL default
///     };
///     vkCreateDynamicDepthStencilState(device, &info1, &state1);
///
///     // Example 2
///     //
///     // Create state using qoCreateDynamicDepthStencilState. We need only
///     // set a named parameter if it deviates from its default value.
///     VkDynamicDsState state2 =
///         qoCreateDynamicDepthStencilState(device, .stencilWriteMask = 0x17);
///
///     // Example 3
///     //
///     // Create state using functions qoDynamicDsStateCreateInfo and
///     // qoCreateDynamicDepthStencilStateS. As for
///     // qoCreateDynamicDepthStencilState, we need only set a named parameter if
///     // it deviates from its default value.
///     VkDynamicDsStateCreateInfo info3 = qoDynamicDsStateCreateInfo(.stencilWriteMask = 0x17);
///     VkDynamicDsState state3 = qoCreateDynamicDepthStencilStateS(device, &info3);

#pragma once

#include <crucible/vk_wrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

VkMemoryRequirements qoObjectGetMemoryRequirements(VkDevice dev, VkObjectType obj_type, VkObject obj);
VkMemoryRequirements qoBufferGetMemoryRequirements(VkDevice dev, VkBuffer buffer);
VkMemoryRequirements qoImageGetMemoryRequirements(VkDevice dev, VkImage image);
VkBuffer qoCreateBufferS(VkDevice dev, const VkBufferCreateInfo *info);
VkDynamicVpState qoCreateDynamicViewportStateS(VkDevice dev, const VkDynamicVpStateCreateInfo *info);
VkDynamicRsState qoCreateDynamicRasterStateS(VkDevice dev, const VkDynamicRsStateCreateInfo *info);
VkDynamicCbState qoCreateDynamicColorBlendStateS(VkDevice dev, const VkDynamicCbStateCreateInfo *info);
VkDynamicDsState qoCreateDynamicDepthStencilStateS(VkDevice dev, const VkDynamicDsStateCreateInfo *info);

#define qoCreateBuffer(dev, ...) qoCreateBufferS(dev, &qoBufferCreateInfo(__VA_ARGS__))
#define qoCreateDynamicViewportState(dev, ...) qoCreateDynamicViewportStateS(dev, &qoDynamicVpStateCreateInfo(__VA_ARGS__))
#define qoCreateDynamicRasterState(dev, ...) qoCreateDynamicRasterStateS(dev, &qoDynamicRsStateCreateInfo(__VA_ARGS__))
#define qoCreateDynamicColorBlendState(dev, ...) qoCreateDynamicColorBlendStateS(dev, &qoDynamicCbStateCreateInfo(__VA_ARGS__))
#define qoCreateDynamicDepthStencilState(dev, ...) qoCreateDynamicDepthStencilStateS(dev, &qoDynamicDsStateCreateInfo(__VA_ARGS__))

#ifdef DOXYGEN
VkBufferCreateInfo qoBufferCreateInfo(...);
#else
#define qoBufferCreateInfo(...) \
  ((VkBufferCreateInfo) { \
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, \
            __VA_ARGS__  \
  })
#endif

#ifdef DOXYGEN
VkDynamicVpStateCreateInfo qoDynamicVpStateCreateInfo(...);
#else
#define qoDynamicVpStateCreateInfo(...) \
    ((VkDynamicVpStateCreateInfo) { \
        .sType = VK_STRUCTURE_TYPE_DYNAMIC_VP_STATE_CREATE_INFO, \
        .viewportAndScissorCount = 0, \
        __VA_ARGS__ \
     })
#endif

#ifdef DOXYGEN
VkDynamicRsStateCreateInfo qoDynamicRsStateCreateInfo(...);
#else
#define qoDynamicRsStateCreateInfo(...) \
    ((VkDynamicRsStateCreateInfo) { \
        .sType = VK_STRUCTURE_TYPE_DYNAMIC_RS_STATE_CREATE_INFO, \
        .depthBias = 0.0f, \
        .depthBiasClamp = 0.0f, \
        .slopeScaledDepthBias = 0.0f, \
        .pointSize = 1.0f, \
        .pointFadeThreshold = 1.0f, \
        .lineWidth = 1.0f, \
        __VA_ARGS__ \
     })
#endif

#ifdef DOXYGEN
VkDynamicCbStateCreateInfo qoDynamicCbStateCreateInfo(...);
#else
#define qoDynamicCbStateCreateInfo(...) \
    ((VkDynamicCbStateCreateInfo) { \
        .sType = VK_STRUCTURE_TYPE_DYNAMIC_CB_STATE_CREATE_INFO, \
        .blendConst = {0.0f, 0.0f, 0.0f, 0.0f}, /* default in OpenGL ES 3.1 */ \
        __VA_ARGS__ \
     })
#endif

#ifdef DOXYGEN
VkDynamicDsStateCreateInfo qoDynamicDsStateCreateInfo(...);
#else
#define qoDynamicDsStateCreateInfo(...) \
    ((VkDynamicDsStateCreateInfo) { \
        .sType = VK_STRUCTURE_TYPE_DYNAMIC_DS_STATE_CREATE_INFO, \
        .minDepth = 0.0f,       /* default in OpenGL ES 3.1 */ \
        .maxDepth = 1.0f,       /* default in OpenGL ES 3.1 */ \
        .stencilReadMask = ~0,  /* default in OpenGL ES 3.1 */ \
        .stencilWriteMask = ~0, /* default in OpenGL ES 3.1 */ \
        .stencilFrontRef = 0,   /* default in OpenGL ES 3.1 */ \
        .stencilBackRef = 0,    /* default in OpenGL ES 3.1 */ \
        __VA_ARGS__ \
     })
#endif

#ifdef __cplusplus
}
#endif
