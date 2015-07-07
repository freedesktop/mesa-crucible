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

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <libpng16/png.h>

#include <crucible/cru.h>

#include "dynamic-spirv.h"

static void
create_pipeline(VkDevice device, VkPipeline *pipeline,
                VkPipelineLayout pipeline_layout)
{
    VkPipelineIaStateCreateInfo ia_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_IA_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .primitiveRestartEnable = false,
    };

    VkShader vs = qoCreateShaderGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(set = 0, binding = 0) uniform block1 {
            vec4 color;
            vec4 offset;
        } u1;
        flat out vec4 v_color;
        void main()
        {
            gl_Position = a_position + u1.offset;
            v_color = u1.color;
        }
    );

    VkShader fs = qoCreateShaderGLSL(t_device, FRAGMENT,
        out vec4 f_color;
        flat in vec4 v_color;
        void main()
        {
            f_color = v_color;
        }
    );

    VkPipelineShaderStageCreateInfo vs_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &ia_create_info,
        .shader = {
            .stage = VK_SHADER_STAGE_VERTEX,
            .shader = vs,
            .linkConstBufferCount = 0,
            .pLinkConstBufferInfo = NULL,
            .pSpecializationInfo = NULL,
        },
    };

    VkPipelineShaderStageCreateInfo fs_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &vs_create_info,
        .shader = {
            .stage = VK_SHADER_STAGE_FRAGMENT,
            .shader = fs,
            .linkConstBufferCount = 0,
            .pLinkConstBufferInfo = NULL,
            .pSpecializationInfo = NULL,
        }
    };

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = &fs_create_info,
        .bindingCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 16,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
            },
        },
        .attributeCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0,
            },
        },
    };

    VkPipelineRsStateCreateInfo rs_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RS_STATE_CREATE_INFO,
        .pNext = &vi_create_info,
        .depthClipEnable = true,
        .rasterizerDiscardEnable = false,
        .fillMode = VK_FILL_MODE_SOLID,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CCW,
    };

    VkPipelineDsStateCreateInfo ds_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DS_STATE_CREATE_INFO,
        .pNext = &rs_create_info,
        .format = VK_FORMAT_UNDEFINED,
    };

    VkPipelineCbStateCreateInfo cb_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CB_STATE_CREATE_INFO,
        .pNext = &ds_create_info,
        .attachmentCount = 1,
        .pAttachments = (VkPipelineCbAttachmentState []) {
            { .channelWriteMask = VK_CHANNEL_A_BIT |
              VK_CHANNEL_R_BIT | VK_CHANNEL_G_BIT | VK_CHANNEL_B_BIT },
        }
    };

    vkCreateGraphicsPipeline(t_device,
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &cb_create_info,
            .flags = 0,
            .layout = pipeline_layout
        },
        pipeline);
    t_cleanup_push_vk_pipeline(t_device, *pipeline);
}

#define HEX_COLOR(v)                            \
    {                                           \
        ((v) >> 16) / 255.0,                    \
        ((v & 0xff00) >> 8) / 255.0,            \
        ((v & 0xff)) / 255.0, 1.0               \
    }

static void
test(void)
{
    VkDescriptorSetLayout set_layout[1];

    set_layout[0] = qoCreateDescriptorSetLayout(t_device,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .arraySize = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .descriptorSetCount = 1,
        .pSetLayouts = set_layout);

    VkPipeline pipeline;
    create_pipeline(t_device, &pipeline, pipeline_layout);

    VkDescriptorSet set[1];
    qoAllocDescriptorSets(t_device, /*pool*/ 0,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          1, set_layout, set);

    VkBuffer buffer = qoCreateBuffer(t_device, .size = 4096,
                                     .usage = VK_BUFFER_USAGE_GENERAL);

    VkMemoryRequirements buffer_reqs =
       qoBufferGetMemoryRequirements(t_device, buffer);

    VkDeviceMemory mem = qoAllocMemory(t_device,
                                       .allocationSize = buffer_reqs.size);
    void *map = qoMapMemory(t_device, mem, 0, 4096, 0);
    memset(map, 192, 4096);

    qoQueueBindBufferMemory(t_queue, buffer, /*index*/ 0, mem, 0);

    static const float color[6][4] = {
        HEX_COLOR(0xfaff81),
        { -0.3, -0.3, 0.0, 0.0 },
        HEX_COLOR(0xffc53a),
        {  0.0,  0.0, 0.0, 0.0 },
        HEX_COLOR(0xe06d06),
        {  0.3,  0.3, 0.0, 0.0 },
    };
    memcpy(map, color, sizeof(color));

    VkBufferView buffer_view = qoCreateBufferView(t_device,
        .buffer = buffer,
        .viewType = VK_BUFFER_VIEW_TYPE_RAW,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .range = sizeof(color));

    uint32_t vertex_offset = 1024;
    static const float vertex_data[] = {
        // Triangle coordinates
        -0.5, -0.5, 0.0, 1.0,
         0.5, -0.5, 0.0, 1.0,
        -0.5,  0.5, 0.0, 1.0,
         0.5,  0.5, 0.0, 1.0,
    };

    memcpy(map + vertex_offset, vertex_data, sizeof(vertex_data));

    VkDynamicVpState vp_state = qoCreateDynamicViewportState(t_device,
        .viewportAndScissorCount = 1,
        .pViewports = (VkViewport[]) {
            {
                .originX = 0,
                .originY = 0,
                .width = t_width,
                .height = t_height,
                .minDepth = 0,
                .maxDepth = 1
            },
        },
        .pScissors = (VkRect2D[]) {
            {{  0,  0 }, {t_width, t_height}},
        }
    );

    VkDynamicRsState rs_state = qoCreateDynamicRasterState(t_device);
    VkDynamicCbState cb_state = qoCreateDynamicColorBlendState(t_device);

    vkUpdateDescriptors(t_device,
        set[0], 1,
        (const void * []) {
            &(VkUpdateBuffers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_BUFFERS,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .arrayIndex = 0,
                .binding = 0,
                .count = 1,
                .pBufferViews = (VkBufferViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view,
                    },
                },
            },
        });

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .renderArea = {{0, 0}, {t_width, t_height}},
        .pColorFormats = (VkFormat[]) { VK_FORMAT_R8G8B8A8_UNORM },
        .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_CLEAR },
        .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE },
        .pColorLoadClearValues = (VkClearColor[]) {
            {
                .color = { .floatColor = HEX_COLOR(0x161032) },
                .useRawValue = false,
            },
        });

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBegin) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
        });
    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { buffer },
                           (VkDeviceSize[]) { vertex_offset });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_VIEWPORT, vp_state);
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_RASTER, rs_state);
    vkCmdBindDynamicStateObject(t_cmd_buffer,
                                VK_STATE_BIND_POINT_COLOR_BLEND, cb_state);

    uint32_t dynamic_offsets[1] = { 0 };
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 1,
                            &set[0], 1, dynamic_offsets);
    vkCmdDraw(t_cmd_buffer, 0, 4, 0, 1);

    dynamic_offsets[0] = 32;
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 1,
                            &set[0], 1, dynamic_offsets);
    vkCmdDraw(t_cmd_buffer, 0, 4, 0, 1);

    dynamic_offsets[0] = 64;
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 1,
                            &set[0], 1, dynamic_offsets);
    vkCmdDraw(t_cmd_buffer, 0, 4, 0, 1);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, 0);
}

cru_define_test {
    .name = "func.desc.dynamic",
    .start = test,
};
