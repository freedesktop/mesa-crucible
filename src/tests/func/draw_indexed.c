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

static inline uint32_t
align_u32(uint32_t value, uint32_t alignment)
{
   return (value + alignment - 1) & ~(alignment - 1);
}

#define CRU_PIPELINE_DEFAULTS \
    .primitiveRestartEnable = false

struct cru_pipeline_template {
    bool primitiveRestartEnable;
    uint32_t primitiveRestartIndex;
};

static VkPipeline
test_create_solid_color_pipeline(struct cru_pipeline_template *t)
{
    VkPipelineIaStateCreateInfo ia_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_IA_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .disableVertexReuse = false,
        .primitiveRestartEnable = t->primitiveRestartEnable,
        .primitiveRestartIndex = t->primitiveRestartIndex
    };

    static const char vs_source[] =
        GLSL(330,
             layout(location = 0) in vec4 a_position;
             layout(location = 1) in vec4 a_color;
             out vec4 v_color;
             void main()
             {
                 gl_Position = a_position + vec4(0, a_color.a, 0, 0);
                 v_color = vec4(a_color.rgb, 1.0);
             });

    static const char fs_source[] =
        GLSL(330,
             out vec4 f_color;
             in vec4 v_color;
             void main()
             {
                 f_color = v_color;
             });

    VkShader vs = qoCreateShader(t_device, .pCode = vs_source,
                                 .codeSize = sizeof(vs_source));
    VkShader fs = qoCreateShader(t_device, .pCode = fs_source,
                                 .codeSize = sizeof(fs_source));

    VkPipelineShaderStageCreateInfo vs_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &ia_create_info,
        .shader = {
            .stage = VK_SHADER_STAGE_VERTEX,
            .shader = vs,
            .linkConstBufferCount = 0,
            .pLinkConstBufferInfo = NULL,
            .pSpecializationInfo = NULL
        }
    };

    VkPipelineShaderStageCreateInfo fs_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = &vs_create_info,
        .shader = {
            .stage = VK_SHADER_STAGE_FRAGMENT,
            .shader = fs,
            .linkConstBufferCount = 0,
            .pLinkConstBufferInfo = NULL,
            .pSpecializationInfo = NULL
        }
    };

    VkPipelineVertexInputCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_CREATE_INFO,
        .pNext = &fs_create_info,
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

    VkPipelineRsStateCreateInfo rs_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RS_STATE_CREATE_INFO,
        .pNext = &vi_create_info,

        .depthClipEnable = true,
        .rasterizerDiscardEnable = false,
        .fillMode = VK_FILL_MODE_SOLID,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CCW
    };

    VkPipelineDsStateCreateInfo ds_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DS_STATE_CREATE_INFO,
        .pNext = &rs_create_info,
        .format = VK_FORMAT_UNDEFINED,
        .depthTestEnable = false,
        .depthWriteEnable = false,
        .depthCompareOp = VK_COMPARE_OP_GREATER
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

    VkPipeline pipeline;
    vkCreateGraphicsPipeline(t_device,
                             &(VkGraphicsPipelineCreateInfo) {
                                 .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                 .pNext = &cb_create_info,
                                 .flags = 0,
                                 .layout = VK_NULL_HANDLE
                             },
                             &pipeline);

    return pipeline;
}

static void
test(void)
{
    VkPipeline pipeline, restart_pipeline;
    VkBuffer buffer;
    VkDeviceMemory mem;
    void *map;
    VkFramebuffer framebuffer;
    VkRenderPass pass;

    buffer = qoCreateBuffer(t_device, .size = 4096,
                            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    VkMemoryRequirements requirements =
        qoBufferGetMemoryRequirements(t_device, buffer);

    mem = qoAllocMemory(t_device, .allocationSize = requirements.size);
    map = qoMapMemory(t_device, mem, 0, requirements.size, 0);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER,
                            buffer, 0, mem, 0);

    vkCreateFramebuffer(t_device,
                        &(VkFramebufferCreateInfo) {
                            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                .colorAttachmentCount = 1,
                                .pColorAttachments = (VkColorAttachmentBindInfo[]) {
                                {
                                    .view = t_image_color_view,
                                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                }
                            },
                            .pDepthStencilAttachment = NULL,
                            .sampleCount = 1,
                            .width = t_width,
                            .height = t_height,
                            .layers = 1
                        },
                        &framebuffer);

#define HEX_COLOR(v, f)                         \
    {                                           \
        ((v) >> 16) / 255.0,                    \
        ((v & 0xff00) >> 8) / 255.0,            \
        ((v & 0xff)) / 255.0,                   \
        f                                       \
    }

    vkCreateRenderPass(t_device,
                       &(VkRenderPassCreateInfo) {
                           .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                           .renderArea = { { 0, 0 }, { t_width, t_height } },
                           .colorAttachmentCount = 1,
                           .extent = { },
                           .sampleCount = 1,
                           .layers = 1,
                           .pColorFormats = (VkFormat[]) { VK_FORMAT_R8G8B8A8_UNORM },
                           .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
                           .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_CLEAR },
                           .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE },
                           .pColorLoadClearValues = (VkClearColor[]) {
                               { .color = { .floatColor = HEX_COLOR(0x522a27, 1.0) }, .useRawValue = false }
                           },
                           .depthStencilFormat = VK_FORMAT_UNDEFINED,
                           .depthStencilLayout = 0,
                           .depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                           .depthLoadClearValue = 0.5,
                           .depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                       },
                       &pass);

    struct cru_pipeline_template t0 = {
        CRU_PIPELINE_DEFAULTS,
    };
    pipeline = test_create_solid_color_pipeline(&t0);

    struct cru_pipeline_template t1 = {
        CRU_PIPELINE_DEFAULTS,
        .primitiveRestartEnable = true,
        .primitiveRestartIndex  = 104
    };
    restart_pipeline = test_create_solid_color_pipeline(&t1);

    static const struct {
        float vertices[256][2];
        float colors[5][4];
    } vertex_data = {
        .vertices = {
            [100] =
            { -0.6,  0.1 },
            { -0.4, -0.1 },
            { -0.2,  0.1 },
            { -0.0, -0.1 },
            {  0.2,  0.1 },
            {  0.4, -0.1 },
            {  0.6,  0.1 }
        },
        .colors = {
            HEX_COLOR(0xa6d49f, -0.8),
            HEX_COLOR(0xe06d06, -0.5),
            HEX_COLOR(0xc59849, -0.2),
            HEX_COLOR(0x9cb380,  0.1),
            HEX_COLOR(0xa6d49f,  0.4),
        }
    };

    memcpy(map, &vertex_data, sizeof(vertex_data));

    vkCmdBeginRenderPass(t_cmd_buffer,
                         &(VkRenderPassBegin) {
                             .renderPass = pass,
                             .framebuffer = framebuffer
                         });

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { buffer, buffer },
                           (VkDeviceSize[]) { 0, sizeof(vertex_data.vertices) });

   vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

   uint8_t index_data8[] = { 50, 101, 102, 103, 104, 105, 106, 107 };
   memcpy(map + 1024, index_data8, sizeof(index_data8));
   vkCmdBindIndexBuffer(t_cmd_buffer, buffer, 1024, VK_INDEX_TYPE_UINT8);

   /* Tests instanced, index rendering with negative base vertex and non-zero
    * start index. */
   vkCmdDrawIndexed(t_cmd_buffer, 1, 7, -1, 0, 2);

   /* Tests restart index */
   vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, restart_pipeline);
   vkCmdDrawIndexed(t_cmd_buffer, 1, 7, -1, 2, 1);

   /* Tests uint16 index type */
   uint16_t index_data16[] = { 50, 101, 102, 103, 104, 105, 106, 107 };
   memcpy(map + 1024 + 64, index_data16, sizeof(index_data16));
   vkCmdBindIndexBuffer(t_cmd_buffer, buffer, 1024 + 64, VK_INDEX_TYPE_UINT16);
   vkCmdDrawIndexed(t_cmd_buffer, 1, 7, -1, 3, 1);

   /* Tests uint32 index type */
   uint32_t index_data32[] = { 50, 101, 102, 103, 104, 105, 106, 107 };
   memcpy(map + 1024 + 128, index_data32, sizeof(index_data32));
   vkCmdBindIndexBuffer(t_cmd_buffer, buffer, 1024 + 128, VK_INDEX_TYPE_UINT32);
   vkCmdDrawIndexed(t_cmd_buffer, 1, 7, -1, 4, 1);

   vkCmdEndRenderPass(t_cmd_buffer, pass);
}

cru_define_test {
    .name = "func.draw_indexed",
    .start = test
};
