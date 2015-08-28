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

#include <stdlib.h>

#include "tapi/t.h"
#include "util/cru_format.h"

#include "stencil_triangles-spirv.h"

typedef struct test_params {
    VkClearDepthStencilValue clear_value;
    uint32_t stencil_ref;
    VkCompareOp stencil_compare_op;
    VkStencilOp stencil_pass_op;
    VkStencilOp stencil_fail_op;
} test_params_t;

/// \brief Clear the stencil buffer by drawing a quad.
///
/// (2015-08-27) We clear the stencil buffer by drawing a quad instead of using
/// vkCmdClearDepthStencil* or VK_ATTACHMENT_LOAD_OP_CLEAR because stencil
/// clears do not yet work in Mesa.
static void
clear_stencil_with_quad(void)
{
    const test_params_t *params = t_user_data;

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device,
        t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            .vertexShader = qoCreateShaderGLSL(t_device, VERTEX,
                void main()
                {
                    // Draw a full quad.
                    switch (gl_VertexID) {
                    case 0: gl_Position = vec4(-1, -1, 0, 1); break;
                    case 1: gl_Position = vec4(+1, -1, 0, 1); break;
                    case 2: gl_Position = vec4(+1, +1, 0, 1); break;
                    case 3: gl_Position = vec4(-1, +1, 0, 1); break;
                    }
                }
            ),
            .fragmentShader = qoCreateShaderGLSL(t_device, FRAGMENT,
                void main() {}
            ),
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .bindingCount = 0,
                .attributeCount = 0,
            },
            .pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo) {
                QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
                .stencilTestEnable = true,
                .front = (VkStencilOpState) {
                    .stencilCompareOp = VK_COMPARE_OP_ALWAYS,
                    .stencilPassOp = VK_STENCIL_OP_REPLACE,
                },
                .back = (VkStencilOpState) {
                    .stencilCompareOp = VK_COMPARE_OP_ALWAYS,
                    .stencilPassOp = VK_STENCIL_OP_REPLACE,
                },
            },
            .layout = QO_NULL_PIPELINE_LAYOUT,
        }}
    );

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .attachmentCount = 1,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_S8_UINT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorCount = 0,
                .depthStencilAttachment = {
                    .attachment = 1,
                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                },
            },
        });

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
        },
        VK_RENDER_PASS_CONTENTS_INLINE);
    vkCmdBindVertexBuffers(t_cmd_buffer,
                           /*startBinding*/ 0, /*bindingCount*/ 0,
                           NULL, NULL);
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDynamicDepthStencilState(t_cmd_buffer,
        qoCreateDynamicDepthStencilState(t_device,
            QO_DYNAMIC_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
            .stencilReadMask = 0xff,
            .stencilWriteMask = 0xff,
            .stencilFrontRef = params->clear_value.stencil,
            .stencilBackRef = params->clear_value.stencil));
    vkCmdDraw(t_cmd_buffer,
              /*firstVertex*/ 0, /*vertexCount*/ 4,
              /*firstInstance*/ 0, /*instanceCount*/ 1);
    vkCmdEndRenderPass(t_cmd_buffer);
}

static void
draw_triangle(void)
{
    const test_params_t *params = t_user_data;

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device,
        t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            .vertexShader = qoCreateShaderGLSL(t_device, VERTEX,
                out vec4 v_color;

                // Draw 3 overlapping triangles.
                void main()
                {
                    switch (gl_VertexID) {
                    case 0: gl_Position = vec4(-0.6, -0.6, 0, 1); break;
                    case 1: gl_Position = vec4(+0.6, -0.6, 0, 1); break;
                    case 2: gl_Position = vec4( 0.0, +0.6, 0, 1); break;
                    }

                    gl_Position.x += 0.2 * (gl_InstanceID - 1);
                    gl_Position.y += 0.2 * (gl_InstanceID - 1);

                    // blue, green, yellow
                    switch (gl_InstanceID) {
                    case 0: v_color = vec4(0.2,  0.2, 1.0, 1.0); break;
                    case 1: v_color = vec4(0.2,  1.0, 0.2, 1.0); break;
                    case 2: v_color = vec4(1.0,  1.0, 0.2, 1.0); break;
                    }
                }
            ),
            .fragmentShader = qoCreateShaderGLSL(t_device, FRAGMENT,
                in vec4 v_color;
                out vec4 f_color;

                void main()
                {
                    f_color = v_color;
                }
            ),
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .bindingCount = 0,
                .attributeCount = 0,
            },
            .pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo) {
                QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
                .stencilTestEnable = true,
                .front = (VkStencilOpState) {
                    .stencilCompareOp = params->stencil_compare_op,
                    .stencilPassOp = params->stencil_pass_op,
                    .stencilFailOp = params->stencil_fail_op,
                },
                .back = (VkStencilOpState) {
                    .stencilCompareOp = params->stencil_compare_op,
                    .stencilPassOp = params->stencil_pass_op,
                    .stencilFailOp = params->stencil_fail_op,
                },
            },
            .layout = QO_NULL_PIPELINE_LAYOUT,
        }}
    );

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .attachmentCount = 2,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            },
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_S8_UINT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorCount = 1,
                .colorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    },
                },
                .depthStencilAttachment = {
                    .attachment = 1,
                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                },
            },
        });

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .attachmentCount = 1,
            .pAttachmentClearValues = (VkClearValue[]) {
                { .color = { .f32 = { 0.2, 0.2, 0.2, 1.0 } } },
            },
        },
        VK_RENDER_PASS_CONTENTS_INLINE);
    vkCmdBindVertexBuffers(t_cmd_buffer,
                           /*startBinding*/ 0, /*bindingCount*/ 0,
                           NULL, NULL);
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDynamicDepthStencilState(t_cmd_buffer,
        qoCreateDynamicDepthStencilState(t_device,
            QO_DYNAMIC_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
            .stencilReadMask = 0xff,
            .stencilWriteMask = 0xff,
            .stencilFrontRef = params->stencil_ref,
            .stencilBackRef = params->stencil_ref));
    vkCmdDraw(t_cmd_buffer,
              /*firstVertex*/ 0, /*vertexCount*/ 3,
              /*firstInstance*/ 0, /*instanceCount*/ 3);
    vkCmdEndRenderPass(t_cmd_buffer);
}

static void
test(void)
{
    clear_stencil_with_quad();
    draw_triangle();

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, QO_NULL_FENCE);
}

#include "stencil_triangles_gen.c"
