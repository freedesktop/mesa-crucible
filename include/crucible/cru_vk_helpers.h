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

#include <crucible/vk_wrapper.h>
#include <stdbool.h>

#define CRU_DEFAULT_PIPELINE_IA_STATE_CREATE_INFO               \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_IA_STATE_CREATE_INFO,   \
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,            \
    .disableVertexReuse = false,                                \
    .primitiveRestartEnable = false,                            \
    .primitiveRestartIndex = 0

#define CRU_DEFAULT_PIPELINE_RS_STATE_CREATE_INFO               \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RS_STATE_CREATE_INFO,   \
    .depthClipEnable = false,                                   \
    .rasterizerDiscardEnable = false,                           \
    .fillMode = VK_FILL_MODE_SOLID,                             \
    .cullMode = VK_CULL_MODE_NONE,                              \
    .frontFace = VK_FRONT_FACE_CCW

#define CRU_DEFAULT_PIPELINE_MS_STATE_CREATE_INFO               \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MS_STATE_CREATE_INFO,   \
    .samples = 1,                                               \
    .multisampleEnable = false,                                 \
    .sampleMask = 0xffffffff

#define CRU_DEFAULT_PIPELINE_CB_ATTACHMENT_STATE                \
    .blendEnable = false,                                       \
    .format = VK_FORMAT_R8G8B8A8_UNORM,                         \
    .channelWriteMask = (VK_CHANNEL_R_BIT | VK_CHANNEL_G_BIT |  \
                         VK_CHANNEL_B_BIT | VK_CHANNEL_A_BIT)

#define CRU_DEFAULT_PIPELINE_CB_STATE_CREATE_INFO               \
    .sType = VK_STRUCTURE_TYPE_PIPELINE_CB_STATE_CREATE_INFO,   \
    .attachmentCount = 1,                                       \
    .pAttachments = (VkPipelineCbAttachmentState []) {          \
       { CRU_DEFAULT_PIPELINE_CB_ATTACHMENT_STATE },            \
    }

#define CRU_STRUCTURE_TYPE_PIPELINE_CREATE_INFO 107684

// This structure provides some extra info for cru_CreateGraphicsPipeline
// that it can use to help fill out defaults.  This allows you to set a lot
// of pieces of the pipeline creation state with one simple structure as
// long as you are ok with getting defaults for everything else.
//
// This structure must immediately follow the GraphicsPipelineCreateInfo
// structure.  In other words, it must be second in the chain.
struct cru_GraphicsPipelineCreateInfo {
    VkStructureType sType; // Must be CRU_STRUCTURE_TYPE_PIPELINE_CREATE_INFO
    const void *pNext;

    VkPrimitiveTopology topology;
    VkShader vertex_shader;
    VkShader fragment_shader;
};

VkResult cru_CreateGraphicsPipeline(VkDevice device, const VkGraphicsPipelineCreateInfo* pCreateInfo, VkPipeline* pPipeline);
