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

static void
draw_triangle(void)
{
    const test_params_t *params = t_user_data;

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
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorCount = 1,
                .pColorAttachments = (VkAttachmentReference[]) {
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

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device,
        t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            .vertexShader = qoCreateShaderGLSL(t_device, VERTEX,
                layout(location = 0) out vec4 v_color;

                // Draw 3 overlapping triangles.
                void main()
                {
                    if (gl_VertexID == 0) {
                        gl_Position = vec4(-0.6, -0.6, 0, 1);
                    } else if (gl_VertexID == 1) {
                        gl_Position = vec4(+0.6, -0.6, 0, 1);
                    } else if (gl_VertexID == 2) {
                        gl_Position = vec4( 0.0, +0.6, 0, 1);
                    }
                    // XXX: spirv_to_nir doesn't handle switch yet
                    // switch (gl_VertexID) {
                    // case 0: gl_Position = vec4(-0.6, -0.6, 0, 1); break;
                    // case 1: gl_Position = vec4(+0.6, -0.6, 0, 1); break;
                    // case 2: gl_Position = vec4( 0.0, +0.6, 0, 1); break;
                    // }

                    gl_Position.x += 0.2 * (gl_InstanceID - 1);
                    gl_Position.y += 0.2 * (gl_InstanceID - 1);

                    // blue, green, yellow
                    if (gl_InstanceID == 0) {
                        v_color = vec4(0.2,  0.2, 1.0, 1.0);
                    } else if (gl_InstanceID == 1) {
                        v_color = vec4(0.2,  1.0, 0.2, 1.0);
                    } else if (gl_InstanceID == 2) {
                        v_color = vec4(1.0,  1.0, 0.2, 1.0);
                    }
                    // XXX: spirv_to_nir doesn't handle switch yet
                    // switch (gl_InstanceID) {
                    // case 0: v_color = vec4(0.2,  0.2, 1.0, 1.0); break;
                    // case 1: v_color = vec4(0.2,  1.0, 0.2, 1.0); break;
                    // case 2: v_color = vec4(1.0,  1.0, 0.2, 1.0); break;
                    // }
                }
            ),
            .fragmentShader = qoCreateShaderGLSL(t_device, FRAGMENT,
                layout(location = 0) in vec4 v_color;
                layout(location = 0) out vec4 f_color;

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
                .vertexBindingDescriptionCount = 0,
                .vertexAttributeDescriptionCount = 0,
            },
            .pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo) {
                QO_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO_DEFAULTS,
                .stencilTestEnable = true,
                .front = (VkStencilOpState) {
                    .compareOp = params->stencil_compare_op,
                    .passOp = params->stencil_pass_op,
                    .failOp = params->stencil_fail_op,
                },
                .back = (VkStencilOpState) {
                    .compareOp = params->stencil_compare_op,
                    .passOp = params->stencil_pass_op,
                    .failOp = params->stencil_fail_op,
                },
            },
            .layout = VK_NULL_HANDLE,
            .renderPass = pass,
            .subpass = 0,
        }}
    );

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 2,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 0.2, 0.2, 0.2, 1.0 } } },
                { .depthStencil = params->clear_value },
            },
        },
        VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindVertexBuffers(t_cmd_buffer,
                           /*startBinding*/ 0, /*bindingCount*/ 0,
                           NULL, NULL);
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdSetStencilCompareMask(t_cmd_buffer,
                               VK_STENCIL_FACE_FRONT_BIT |
                               VK_STENCIL_FACE_BACK_BIT,
                               0xff);
    vkCmdSetStencilWriteMask(t_cmd_buffer,
                             VK_STENCIL_FACE_FRONT_BIT |
                             VK_STENCIL_FACE_BACK_BIT,
                             0xff);
    vkCmdSetStencilReference(t_cmd_buffer,
                             VK_STENCIL_FACE_FRONT_BIT |
                             VK_STENCIL_FACE_BACK_BIT,
                             params->stencil_ref);
    vkCmdDraw(t_cmd_buffer,
              /*vertexCount*/ 3, /*instanceCount*/ 3,
              /*firstVertex*/ 0, /*firstInstance*/ 0);
    vkCmdEndRenderPass(t_cmd_buffer);
}

static void
test(void)
{
    draw_triangle();
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

#include "stencil_triangles_gen.c"
