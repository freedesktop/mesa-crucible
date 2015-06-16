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
#include <crucible/cru_test.h>
#include <assert.h>

#include "qonos_pipeline-spirv.h"

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

VkPipeline
qoCreateGraphicsPipeline(VkDevice device,
                         const QoExtraGraphicsPipelineCreateInfo *extra)
{
    VkGraphicsPipelineCreateInfo pipeline_info;
    VkPipelineIaStateCreateInfo ia_info;
    VkPipelineRsStateCreateInfo rs_info;
    VkPipelineMsStateCreateInfo ms_info;
    VkPipelineCbStateCreateInfo cb_info;
    VkPipelineShaderStageCreateInfo vs_info;
    VkPipelineShaderStageCreateInfo fs_info;
    VkPipeline pipeline;
    VkResult result;

    if (extra->pNext) {
        // We must make a copy so that we can change the pNext pointer.
        pipeline_info = *(extra->pNext);
    } else {
        pipeline_info = (VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = NULL,
        };
    };

    if (!find_struct_in_chain(pipeline_info.pNext,
                              PIPELINE_IA_STATE_CREATE_INFO)) {
        ia_info = (VkPipelineIaStateCreateInfo) {
            QO_PIPELINE_IA_STATE_CREATE_INFO_DEFAULTS,
            .topology = extra->topology,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &ia_info;
    }

    if (!find_struct_in_chain(pipeline_info.pNext,
                              PIPELINE_RS_STATE_CREATE_INFO)) {
        rs_info = (VkPipelineRsStateCreateInfo) {
            QO_PIPELINE_RS_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &rs_info;
    }

    if (!find_struct_in_chain(pipeline_info.pNext,
                              PIPELINE_MS_STATE_CREATE_INFO)) {
        ms_info = (VkPipelineMsStateCreateInfo) {
            QO_PIPELINE_MS_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &ms_info;
    }

    if (!find_struct_in_chain(pipeline_info.pNext,
                              PIPELINE_CB_STATE_CREATE_INFO)) {
        cb_info = (VkPipelineCbStateCreateInfo) {
            QO_PIPELINE_CB_STATE_CREATE_INFO_DEFAULTS,
            .pNext = pipeline_info.pNext,
        };
        pipeline_info.pNext = &cb_info;
    }

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

    VkPipelineVertexInputCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_CREATE_INFO,
        .bindingCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 8,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX
            },
            {
                .binding = 1,
                .strideInBytes = 16,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_INSTANCE
            }
        },
        .attributeCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offsetInBytes = 0
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0
            }
        }
    };

    if (!find_struct_in_chain(pipeline_info.pNext,
                              PIPELINE_VERTEX_INPUT_CREATE_INFO)) {
        /* They should be using one of our shaders if they use this */
        assert(!has_vs || !has_fs);
        vi_info.pNext = pipeline_info.pNext,
        pipeline_info.pNext = &vi_info;
    }

    if (!has_vs) {
        VkShader default_vs = extra->vertexShader ? extra->vertexShader :
            qoCreateShaderGLSL(t_device, VERTEX,
                layout(location = 0) in vec4 a_position;
                layout(location = 1) in vec4 a_color;
                out vec4 v_color;
                void main()
                {
                    gl_Position = a_position;
                    v_color = a_color;
                }
            );

        vs_info = (VkPipelineShaderStageCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = pipeline_info.pNext,
            .shader = {
                .stage = VK_SHADER_STAGE_VERTEX,
                .shader = default_vs,
                .linkConstBufferCount = 0,
                .pLinkConstBufferInfo = NULL,
                .pSpecializationInfo = NULL,
            }
        };
        pipeline_info.pNext = &vs_info;
    }

    if (!has_fs) {
        VkShader default_fs = extra->fragmentShader ? extra->fragmentShader :
            qoCreateShaderGLSL(t_device, FRAGMENT,
                out vec4 f_color;
                in vec4 v_color;
                void main()
                {
                    f_color = v_color;
                }
            );

        fs_info = (VkPipelineShaderStageCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = pipeline_info.pNext,
            .shader = {
                .stage = VK_SHADER_STAGE_FRAGMENT,
                .shader = default_fs,
                .linkConstBufferCount = 0,
                .pLinkConstBufferInfo = NULL,
                .pSpecializationInfo = NULL,
            }
        };
        pipeline_info.pNext = &fs_info;
    }

    result = vkCreateGraphicsPipeline(device, &pipeline_info, &pipeline);

    t_assert(result == VK_SUCCESS);
    t_assert(pipeline);
    t_cleanup_push_vk_pipeline(t_device, pipeline);

    return pipeline;
}
