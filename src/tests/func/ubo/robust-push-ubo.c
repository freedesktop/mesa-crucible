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
#include "util/misc.h"

#include "robust-push-ubo-spirv.h"

#define UBO_PAD_SIZE (4 * 4 * 15)
#define UBO_BIND_SIZE (UBO_PAD_SIZE + 4 * 4)

static VkPipeline
create_pipeline(VkDevice device, VkPipelineLayout pipeline_layout)
{
    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        void main()
        {
            vec2 pos = vec2(float(gl_VertexIndex & 1),
                            float(gl_VertexIndex >> 1));
            gl_Position = vec4(vec2(-1) + 2 * pos, 0.0f, 1.0f);
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_color;

        layout(set = 0, binding = 0) uniform block1 {
            // Ensure that the two colors are in different 32B blocks
            vec4 pad[15];
            vec4 color1;
            vec4 color2;
        } u;

        void main()
        {
            f_color = u.color1 + u.color2;
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL,
    };

    return qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pVertexInputState = &vi_create_info,
            .flags = 0,
            .layout = pipeline_layout,
            .renderPass = t_render_pass,
            .subpass = 0,
        }});
}

static VkBuffer
create_buffer(uint32_t bind_offset)
{
    const VkDeviceSize buffer_size = 4096;
    const VkDeviceSize ubo_align =
        t_physical_dev_props->limits.minUniformBufferOffsetAlignment;
    assert(bind_offset % ubo_align == 0);

    VkBuffer buffer = qoCreateBuffer(t_device, .size = buffer_size);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    qoBindBufferMemory(t_device, buffer, mem, /*offset*/ 0);

    void *map = qoMapMemory(t_device, mem, /*offset*/ 0,
                            buffer_size, /*flags*/ 0);

    uint32_t offset = 0;

    assert(offset + bind_offset <= buffer_size);
    memset(map + offset, 0, bind_offset);
    offset += bind_offset;

    assert(offset + UBO_PAD_SIZE <= buffer_size);
    memset(map + offset, 0, UBO_PAD_SIZE);
    offset += UBO_PAD_SIZE;

    const float colors[8] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    };
    assert(offset + sizeof(colors) <= buffer_size);
    memcpy(map + offset, colors, sizeof(colors));
    offset += sizeof(colors);

    return buffer;
}

static void
test(void)
{
    const uint32_t bind_offset = 512;

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 1,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipeline pipeline = create_pipeline(t_device, pipeline_layout);

    VkBuffer buffer = create_buffer(bind_offset);

    VkDescriptorSet set = qoAllocateDescriptorSet(t_device,
        .descriptorPool = t_descriptor_pool,
        .pSetLayouts = &set_layout);

    vkUpdateDescriptorSets(t_device, 1, /* writeCount */
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = (VkDescriptorBufferInfo[]) {
                    {
                        .buffer = buffer,
                        .offset = bind_offset,
                        .range = UBO_BIND_SIZE,
                    }
                },
            },
        }, 0, NULL);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 0, NULL);
    vkCmdDraw(t_cmd_buffer, /*vertexCount*/ 4, /*instanceCount*/ 1,
              /*firstVertex*/ 0, /*firstInstance*/ 0);
    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.ubo.robust-push-ubo",
    .start = test,
    .image_filename = "32x32-green.ref.png",
    .robust_buffer_access = true,
};

static void
test_dynamic(void)
{
    const uint32_t bind_offset = 512;

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
            .bindingCount = 1,
            .pBindings = (VkDescriptorSetLayoutBinding[]) {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
            });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipeline pipeline = create_pipeline(t_device, pipeline_layout);

    VkBuffer buffer = create_buffer(bind_offset);

    VkDescriptorSet set = qoAllocateDescriptorSet(t_device,
        .descriptorPool = t_descriptor_pool,
        .pSetLayouts = &set_layout);

    vkUpdateDescriptorSets(t_device, 1, /* writeCount */
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .pBufferInfo = (VkDescriptorBufferInfo[]) {
                    {
                        .buffer = buffer,
                        .offset = 0,
                        .range = UBO_BIND_SIZE,
                    }
                },
            },
        }, 0, NULL);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = { 1.0, 0.0, 0.0, 1.0 } } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 1, (uint32_t[]) { bind_offset });
    vkCmdDraw(t_cmd_buffer, /*vertexCount*/ 4, /*instanceCount*/ 1,
              /*firstVertex*/ 0, /*firstInstance*/ 0);
    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

test_define {
    .name = "func.ubo.robust-push-ubo-dynamic",
    .start = test_dynamic,
    .image_filename = "32x32-green.ref.png",
    .robust_buffer_access = true,
};
