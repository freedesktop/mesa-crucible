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
#include "qonos_pipeline-spirv.h"
#include "tapi/t_cleanup.h"
#include "tapi/t_result.h"

VkPipeline
qoCreateGraphicsPipeline(VkDevice device,
                         VkPipelineCache pipeline_cache,
                         const QoExtraGraphicsPipelineCreateInfo *extra)
{
    VkGraphicsPipelineCreateInfo pipeline_info;
    VkPipelineInputAssemblyStateCreateInfo ia_info;
    VkPipelineRasterStateCreateInfo rs_info;
    VkPipelineMultisampleStateCreateInfo ms_info;
    VkPipelineColorBlendStateCreateInfo cb_info;
    VkPipelineShaderStageCreateInfo stage_info[VK_SHADER_STAGE_NUM];
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

    if (pipeline_info.pRasterState == NULL) {
        rs_info = (VkPipelineRasterStateCreateInfo) {
            QO_PIPELINE_RASTER_STATE_CREATE_INFO_DEFAULTS,
        };
        pipeline_info.pRasterState = &rs_info;
    }

    if (pipeline_info.pMultisampleState == NULL) {
        ms_info = (VkPipelineMultisampleStateCreateInfo) {
            QO_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_DEFAULTS,
        };
        pipeline_info.pMultisampleState = &ms_info;
    }

    if (pipeline_info.pColorBlendState == NULL) {
        cb_info = (VkPipelineColorBlendStateCreateInfo) {
            QO_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_DEFAULTS,
        };
        pipeline_info.pColorBlendState = &cb_info;
    }

    // Look for vertex or fragment shaders in the chain
    bool has_fs = false, has_vs = false;
    for (unsigned i = 0; i < pipeline_info.stageCount; i++) {
        switch (pipeline_info.pStages[i].stage) {
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

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
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

    if (pipeline_info.pVertexInputState == NULL) {
        /* They should be using one of our shaders if they use this */
        assert(!has_vs || !has_fs);
        pipeline_info.pVertexInputState = &vi_info;
    }

    if (!has_vs || !has_fs) {
        /* Make a copy of the shader stages so that we can modify it */
        assert(pipeline_info.stageCount < VK_SHADER_STAGE_NUM);
        memcpy(stage_info, pipeline_info.pStages,
               pipeline_info.stageCount * sizeof(*pipeline_info.pStages));
        pipeline_info.pStages = stage_info;
    }

    if (!has_vs) {
        VkShader vs = extra->vertexShader;

        if (!vs.handle) {
            vs = qoCreateShaderGLSL(device, VERTEX,
                layout(location = 0) in vec4 a_position;
                layout(location = 1) in vec4 a_color;
                out vec4 v_color;
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
                .stage = VK_SHADER_STAGE_VERTEX,
                .shader = vs,
                .pSpecializationInfo = NULL,
            };
    }

    if (!has_fs) {
        VkShader fs = extra->fragmentShader;

        if (!fs.handle) {
            fs = qoCreateShaderGLSL(device, FRAGMENT,
                out vec4 f_color;
                in vec4 v_color;
                void main()
                {
                    f_color = v_color;
                }
            );
        }

        stage_info[pipeline_info.stageCount++] =
            (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT,
                .shader = fs,
                .pSpecializationInfo = NULL,
            };
    }

    result = vkCreateGraphicsPipelines(device, pipeline_cache,
                                       1, &pipeline_info, &pipeline);

    t_assert(result == VK_SUCCESS);
    t_assert(pipeline.handle);
    t_cleanup_push_vk_pipeline(device, pipeline);

    return pipeline;
}
