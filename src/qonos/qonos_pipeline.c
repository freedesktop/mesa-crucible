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

#include <assert.h>
#include <string.h>

#include "qonos/qonos.h"
#include "src/qonos/qonos_pipeline-spirv.h"
#include "tapi/t_cleanup.h"
#include "tapi/t_data.h"
#include "tapi/t_result.h"

#define NUM_SHADER_STAGES 6

VkPipeline
qoCreateGraphicsPipeline(VkDevice device,
                         VkPipelineCache pipeline_cache,
                         const QoExtraGraphicsPipelineCreateInfo *extra)
{
    VkGraphicsPipelineCreateInfo pipeline_info;
    VkPipelineInputAssemblyStateCreateInfo ia_info;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo vp_info;
    VkPipelineRasterizationStateCreateInfo rs_info;
    VkPipelineMultisampleStateCreateInfo ms_info;
    VkPipelineDepthStencilStateCreateInfo ds_info;
    VkPipelineColorBlendAttachmentState cb_att;
    VkPipelineColorBlendStateCreateInfo cb_info;
    VkPipelineShaderStageCreateInfo stage_info[NUM_SHADER_STAGES];
    // Enough dynamic states to cover the 32 bits available in
    // QoExtraGraphicsPipelineCreateInfo.
    VkDynamicState dynamic_states[32];
    VkPipelineDynamicStateCreateInfo dy_info;
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

    if (pipeline_info.pInputAssemblyState == NULL) {
        ia_info = (VkPipelineInputAssemblyStateCreateInfo) {
            QO_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_DEFAULTS,
            .topology = extra->topology,
        };
        pipeline_info.pInputAssemblyState = &ia_info;
    }

    if (pipeline_info.pRasterizationState == NULL) {
        rs_info = (VkPipelineRasterizationStateCreateInfo) {
            QO_PIPELINE_RASTERIZATION_STATE_CREATE_INFO_DEFAULTS,
        };
        pipeline_info.pRasterizationState = &rs_info;
    }

    if (!pipeline_info.pRasterizationState->rasterizerDiscardEnable &&
        pipeline_info.pViewportState == NULL) {
        vp_info = (VkPipelineViewportStateCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        if (!(extra->dynamicStates & (1u << VK_DYNAMIC_STATE_VIEWPORT))) {
            viewport = (VkViewport) {
                0.0, 0.0,
                t_width, t_height,
                0.0, 1.0
            };
            vp_info.pViewports = &viewport;
        }

        if (!(extra->dynamicStates & (1u << VK_DYNAMIC_STATE_SCISSOR))) {
            scissor = (VkRect2D) {
                { 0, 0 },
                {t_width, t_height }
            };
            vp_info.pScissors = &scissor;
        }

        pipeline_info.pViewportState = &vp_info;
    }

    if (pipeline_info.pMultisampleState == NULL) {
        ms_info = (VkPipelineMultisampleStateCreateInfo) {
            QO_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_DEFAULTS,
        };
        pipeline_info.pMultisampleState = &ms_info;
    }

    if (pipeline_info.pDepthStencilState == NULL) {
        ds_info = (VkPipelineDepthStencilStateCreateInfo) {
            QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
        };
        pipeline_info.pDepthStencilState = &ds_info;
    }

    if (pipeline_info.pColorBlendState == NULL) {
        cb_att = (VkPipelineColorBlendAttachmentState) {
            QO_PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_DEFAULTS,
        };
        cb_info = (VkPipelineColorBlendStateCreateInfo) {
            QO_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_DEFAULTS,
            .attachmentCount = 1,
            .pAttachments = &cb_att,
        };
        pipeline_info.pColorBlendState = &cb_info;
    }

    if (pipeline_info.pDynamicState == NULL) {
        int count = 0;
        for (int s = 0; s < 32; s++) {
            if (extra->dynamicStates & (1u << s))
                dynamic_states[count++] = s;
        }

        if (count > 0) {
           dy_info = (VkPipelineDynamicStateCreateInfo) {
               .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
               .dynamicStateCount = count,
               .pDynamicStates = dynamic_states,
           };
           pipeline_info.pDynamicState = &dy_info;
        }
    }

    // Look for vertex or fragment shaders in the chain
    bool has_fs = false, has_vs = false;
    for (unsigned i = 0; i < pipeline_info.stageCount; i++) {
        switch (pipeline_info.pStages[i].stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            has_vs = true;
            break;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            has_fs = true;
            break;
        default:
            break;
        }
    }

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 8,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            },
            {
                .binding = 1,
                .stride = 16,
                .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
            }
        },
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0
            }
        }
    };

    if (pipeline_info.pVertexInputState == NULL) {
        /* They should be using one of our shaders if they use this */
        assert(!has_vs || !has_fs);
        pipeline_info.pVertexInputState = &vi_info;
    }

    if (!has_vs || !has_fs || extra->geometryShader != VK_NULL_HANDLE) {
        /* Make a copy of the shader stages so that we can modify it */
        assert(pipeline_info.stageCount < NUM_SHADER_STAGES);
        memcpy(stage_info, pipeline_info.pStages,
               pipeline_info.stageCount * sizeof(*pipeline_info.pStages));
        pipeline_info.pStages = stage_info;
    }

    if (!has_vs) {
        VkShaderModule vs = extra->vertexShader;

        if (!vs != VK_NULL_HANDLE) {
            vs = qoCreateShaderModuleGLSL(device, VERTEX,
                layout(location = 0) in vec4 a_position;
                layout(location = 1) in vec4 a_color;
                layout(location = 0) out vec4 v_color;
                void main()
                {
                    gl_Position = a_position;
                    v_color = a_color;
                }
            );
        }

        stage_info[pipeline_info.stageCount++] =
            (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vs,
                .pName = "main",
                .pSpecializationInfo = NULL,
            };
    }

    if (extra->geometryShader != VK_NULL_HANDLE) {
        // We're assuming here that they didn't try to set the geometry
        // shader both ways (through extra and normally).
        stage_info[pipeline_info.stageCount++] =
            (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
                .module = extra->geometryShader,
                .pName = "main",
                .pSpecializationInfo = NULL,
            };
    }

    if (!has_fs) {
        VkShaderModule fs = extra->fragmentShader;

        if (!fs != VK_NULL_HANDLE) {
            fs = qoCreateShaderModuleGLSL(device, FRAGMENT,
                layout(location = 0) out vec4 f_color;
                layout(location = 0) in vec4 v_color;
                void main()
                {
                    f_color = v_color;
                }
            );
        }

        stage_info[pipeline_info.stageCount++] =
            (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fs,
                .pName = "main",
                .pSpecializationInfo = NULL,
            };
    }

    result = vkCreateGraphicsPipelines(device, pipeline_cache,
                                       1, &pipeline_info, NULL, &pipeline);

    t_assert(result == VK_SUCCESS);
    t_assert(pipeline != VK_NULL_HANDLE);
    t_cleanup_push_vk_pipeline(device, pipeline);

    return pipeline;
}
