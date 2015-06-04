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

#define QO_BUFFER_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO

#define QO_DYNAMIC_VP_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_VP_STATE_CREATE_INFO, \
    .viewportAndScissorCount = 0

#define QO_DYNAMIC_RS_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_RS_STATE_CREATE_INFO, \
    .depthBias = 0.0f, \
    .depthBiasClamp = 0.0f, \
    .slopeScaledDepthBias = 0.0f, \
    .pointSize = 1.0f, \
    .pointFadeThreshold = 1.0f, \
    .lineWidth = 1.0f

#define QO_DYNAMIC_CB_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_CB_STATE_CREATE_INFO, \
    .blendConst = {0.0f, 0.0f, 0.0f, 0.0f} /* default in OpenGL ES 3.1 */

#define QO_DYNAMIC_DS_STATE_CREATE_INFO_DEFAULTS \
    .sType = VK_STRUCTURE_TYPE_DYNAMIC_DS_STATE_CREATE_INFO, \
    .minDepth = 0.0f,       /* default in OpenGL ES 3.1 */ \
    .maxDepth = 1.0f,       /* default in OpenGL ES 3.1 */ \
    .stencilReadMask = ~0,  /* default in OpenGL ES 3.1 */ \
    .stencilWriteMask = ~0, /* default in OpenGL ES 3.1 */ \
    .stencilFrontRef = 0,   /* default in OpenGL ES 3.1 */ \
    .stencilBackRef = 0     /* default in OpenGL ES 3.1 */

VkMemoryRequirements qoObjectGetMemoryRequirements(VkDevice dev, VkObjectType obj_type, VkObject obj);
VkMemoryRequirements qoBufferGetMemoryRequirements(VkDevice dev, VkBuffer buffer);
VkMemoryRequirements qoImageGetMemoryRequirements(VkDevice dev, VkImage image);

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

VkBuffer __qoCreateBuffer(VkDevice dev, const VkBufferCreateInfo *info);
VkDynamicVpState __qoCreateDynamicViewportState(VkDevice dev, const VkDynamicVpStateCreateInfo *info);
VkDynamicRsState __qoCreateDynamicRasterState(VkDevice dev, const VkDynamicRsStateCreateInfo *info);
VkDynamicCbState __qoCreateDynamicColorBlendState(VkDevice dev, const VkDynamicCbStateCreateInfo *info);
VkDynamicDsState __qoCreateDynamicDepthStencilState(VkDevice dev, const VkDynamicDsStateCreateInfo *info);

#ifdef __cplusplus
}
#endif
