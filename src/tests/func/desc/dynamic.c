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

#include "tapi/t.h"

#include "dynamic-spirv.h"

struct params {
    VkDescriptorType descriptor_type;
};

static VkPipeline
create_pipeline(VkDevice device,
                VkPipelineLayout pipeline_layout, VkRenderPass pass)
{
    const struct params *params = t_user_data;

    VkShaderModule vs;
    if (params->descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
            layout(location = 0) in vec4 a_position;
            layout(std140, set = 0, binding = 0) uniform block1 {
                vec4 color;
                vec4 offset;
            } u1;
            layout(location = 0) flat out vec4 v_color;
            void main()
            {
                gl_Position = a_position + u1.offset;
                v_color = u1.color;
            });
    } else {
        vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
            layout(location = 0) in vec4 a_position;
            layout(std140, set = 0, binding = 0) uniform block2 {
                uint i;
            } u1;
            layout(std140, set = 0, binding = 1) buffer block1 {
                vec4 color;
                vec4 offset;
            } s1[2];
            layout(location = 0) flat out vec4 v_color;
            void main()
            {
                gl_Position = a_position + s1[u1.i].offset;
                v_color = s1[u1.i].color;
            });
    }

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;
        layout(location = 0) flat in vec4 v_color;
        void main()
        {
            f_color = v_color;
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 16,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        },
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0,
            },
        },
    };

    return qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_create_info,
            .flags = 0,
            .layout = pipeline_layout,
            .renderPass = pass,
            .subpass = 0,
        }});
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
    VkRenderPass pass = qoCreateRenderPass(t_device,
        .attachmentCount = 1,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            },
        },
        .subpassCount = 1,
        .pSubpasses = (VkSubpassDescription[]) {
            {
                QO_SUBPASS_DESCRIPTION_DEFAULTS,
                .colorAttachmentCount = 1,
                .pColorAttachments = (VkAttachmentReference[]) {
                    {
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    },
                },
            }
        });

    const struct params *params = t_user_data;

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 2,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .binding = 1,
                    .descriptorType = params->descriptor_type,
                    .descriptorCount = 2,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipeline pipeline = create_pipeline(t_device, pipeline_layout, pass);

    VkDescriptorSet set =
        qoAllocateDescriptorSet(t_device, .pSetLayouts = &set_layout);

    VkBuffer buffer = qoCreateBuffer(t_device, .size = 4096);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    void *map = qoMapMemory(t_device, mem, 0, 4096, 0);
    memset(map, 192, 4096);

    qoBindBufferMemory(t_device, buffer, mem, 0);

    static const float color[6][4] = {
        HEX_COLOR(0xfaff81),
        { -0.3, -0.3, 0.0, 0.0 },
        HEX_COLOR(0xffc53a),
        {  0.0,  0.0, 0.0, 0.0 },
        HEX_COLOR(0xe06d06),
        {  0.3,  0.3, 0.0, 0.0 },
    };
    memcpy(map, color, sizeof(color));

    uint32_t vertex_offset = 1024;
    static const float vertex_data[] = {
        // Triangle coordinates
        -0.5, -0.5, 0.0, 1.0,
         0.5, -0.5, 0.0, 1.0,
        -0.5,  0.5, 0.0, 1.0,
         0.5,  0.5, 0.0, 1.0,
    };

    memcpy(map + vertex_offset, vertex_data, sizeof(vertex_data));

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 1,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = params->descriptor_type,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = buffer,
                    .offset = 0,
                    .range = sizeof(color),
                },
            },
        }, 0, NULL);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = HEX_COLOR(0x161032) } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { buffer },
                           (VkDeviceSize[]) { vertex_offset });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    uint32_t dynamic_offsets[1] = { 0 };
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 1, dynamic_offsets);
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    dynamic_offsets[0] = 32;
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 1, dynamic_offsets);
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    dynamic_offsets[0] = 64;
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 1, dynamic_offsets);
    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.desc.dynamic.storage-buffer",
    .start = test,
    .user_data = &(struct params) {
        .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    }
};

test_define {
    .name = "func.desc.dynamic.uniform-buffer",
    .start = test,
    .user_data = &(struct params) {
        .descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    }
};
