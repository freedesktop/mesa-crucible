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

#include <crucible/cru_vk_helpers.h>

struct vk_struct {
    VkStructureType sType;
    const struct vk_struct *pNext;
};

static const void *
_find_struct_in_chain(const void *start, VkStructureType sType)
{
    for (const struct vk_struct *s = start; s; s = s->pNext) {
        if (s->sType == sType)
            return s;
    }

    return NULL;
}

// Less typing!
#define find_struct_in_chain(chain, type) \
    _find_struct_in_chain((chain), VK_STRUCTURE_TYPE_##type)

#define next_shader_info(chain) \
    find_struct_in_chain((chain), PIPELINE_SHADER_STAGE_CREATE_INFO)

VkResult
cru_CreateGraphicsPipeline(VkDevice device,
                           const VkGraphicsPipelineCreateInfo* pCreateInfo,
                           VkPipeline* pPipeline)
{
    // We need to make a copy so that we can change the pNext pointer
    VkGraphicsPipelineCreateInfo pipeline_info = *pCreateInfo;

    VkPipelineIaStateCreateInfo ia_info;
    VkPipelineRsStateCreateInfo rs_info;
    VkPipelineMsStateCreateInfo ms_info;
    VkPipelineCbStateCreateInfo cb_info;
    VkPipelineShaderStageCreateInfo vs_info;
    VkPipelineShaderStageCreateInfo fs_info;

    const struct cru_GraphicsPipelineCreateInfo *cru_info = pipeline_info.pNext;
    if (cru_info && cru_info->sType == CRU_STRUCTURE_TYPE_PIPELINE_CREATE_INFO)
        pipeline_info.pNext = cru_info->pNext;
    else
        cru_info = NULL;

    if (!find_struct_in_chain(pCreateInfo, PIPELINE_IA_STATE_CREATE_INFO)) {
        ia_info = (VkPipelineIaStateCreateInfo) {
            QO_PIPELINE_IA_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        if (cru_info)
            ia_info.topology = cru_info->topology;
        pipeline_info.pNext = &ia_info;
    }

    if (!find_struct_in_chain(pCreateInfo, PIPELINE_RS_STATE_CREATE_INFO)) {
        rs_info = (VkPipelineRsStateCreateInfo) {
            QO_PIPELINE_RS_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &rs_info;
    }

    if (!find_struct_in_chain(pCreateInfo, PIPELINE_MS_STATE_CREATE_INFO)) {
        ms_info = (VkPipelineMsStateCreateInfo) {
            QO_PIPELINE_MS_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &ms_info;
    }

    if (!find_struct_in_chain(pCreateInfo, PIPELINE_CB_STATE_CREATE_INFO)) {
        cb_info = (VkPipelineCbStateCreateInfo) {
            QO_PIPELINE_CB_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &cb_info;
    }

    if (cru_info != NULL) {
        // Look for vertex or fragment shaders in the chain
        bool has_fs = false, has_vs = false;
        const VkPipelineShaderStageCreateInfo *shader_info;
        for (shader_info = next_shader_info(pipeline_info.pNext);
             shader_info != NULL;
             shader_info = next_shader_info(shader_info->pNext)) {
            switch (shader_info->shader.stage) {
            case VK_SHADER_STAGE_VERTEX:
                has_vs = true;
                break;
            case VK_SHADER_STAGE_FRAGMENT:
                has_fs = true;
                break;
            default:
                break;
            }
        }

        if (!has_vs) {
            vs_info = (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = pipeline_info.pNext,
                .shader = {
                    .stage = VK_SHADER_STAGE_VERTEX,
                    .shader = cru_info->vertex_shader,
                    .linkConstBufferCount = 0,
                    .pLinkConstBufferInfo = NULL,
                    .pSpecializationInfo = NULL,
                }
            };
            pipeline_info.pNext = &vs_info;
        }

        if (!has_fs) {
            fs_info = (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = pipeline_info.pNext,
                .shader = {
                    .stage = VK_SHADER_STAGE_FRAGMENT,
                    .shader = cru_info->fragment_shader,
                    .linkConstBufferCount = 0,
                    .pLinkConstBufferInfo = NULL,
                    .pSpecializationInfo = NULL,
                }
            };
            pipeline_info.pNext = &fs_info;
        }
    }

    return vkCreateGraphicsPipeline(device, &pipeline_info, pPipeline);
}
