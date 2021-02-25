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

#include "src/tests/func/ssbo/interleave-spirv.h"

#define HEX_COLOR(v)                            \
    {                                           \
        ((v) >> 16) / 255.0,                    \
        ((v & 0xff00) >> 8) / 255.0,            \
        ((v & 0xff)) / 255.0, 1.0               \
    }

static void
test(void)
{
    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 1,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec2 data;
        layout(set = 0, binding = 0, std140) buffer Storage {
           vec4 arr[];
        } ssbo;

        void main()
        {
           if (gl_VertexIndex % 2 == 0) {
              ssbo.arr[gl_VertexIndex / 2].xz = data;
           } else {
              ssbo.arr[gl_VertexIndex / 2].yw = data;
           }
           gl_Position = vec4(data, 0, 1);
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) out vec4 f_out;
        void main()
        {
            f_out = vec4(0.0, 1.0, 0.0, 1.0);
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 8,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        },
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_UINT,
                .offset = 0,
            },
        },
    };

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
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
            .renderPass = t_render_pass,
            .subpass = 0,
        }});

    VkDescriptorSet set =
        qoAllocateDescriptorSet(t_device,
                                .descriptorPool = t_descriptor_pool,
                                .pSetLayouts = &set_layout);

    VkBuffer buffer_in = qoCreateBuffer(t_device, .size = 4096);

    VkDeviceMemory mem_in = qoAllocBufferMemory(t_device, buffer_in,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint32_t *map_in = qoMapMemory(t_device, mem_in, 0, 4096, 0);
    for (unsigned i = 0; i < 1024; i++)
        map_in[i] = i;

    qoBindBufferMemory(t_device, buffer_in, mem_in, 0);

    VkBuffer buffer_out = qoCreateBuffer(t_device, .size = 4096);
    VkDeviceMemory mem_out = qoAllocBufferMemory(t_device, buffer_out,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, buffer_out, mem_out, 0);

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 1,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = buffer_out,
                    .offset = 0,
                    .range = 4096,
                },
            },
        }, 0, NULL);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = HEX_COLOR(0x161032) } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1,
                           (VkBuffer[]) { buffer_in },
                           (VkDeviceSize[]) { 0 });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout, 0, 1,
                            &set, 0, NULL);
    vkCmdDraw(t_cmd_buffer, 512, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    t_skipf("Skipping, since proper execution of this test depends on "
            "https://github.com/KhronosGroup/glslang/issues/94");

    uint32_t *map_out = qoMapMemory(t_device, mem_out, 0, 4096, 0);
    for (unsigned i = 0; i < 256; i++) {
        t_assertf(map_out[i * 4 + 0] == i * 4 + 0 &&
                  map_out[i * 4 + 1] == i * 4 + 2 &&
                  map_out[i * 4 + 2] == i * 4 + 1 &&
                  map_out[i * 4 + 3] == i * 4 + 3,
                  "buffer mismatch at uvec4 %d: found (%u, %u, %u, %u), "
                  "expected (%u, %u, %u, %u)", i,
                  map_out[i * 4 + 0], map_out[i * 4 + 1],
                  map_out[i * 4 + 2], map_out[i * 4 + 3],
                  i * 4 + 0, i * 4 + 2, i * 4 + 1, i * 4 + 3);
    }
    t_pass();
}

test_define {
    .name = "func.ssbo.interleave",
    .start = test,
    .image_filename = "32x32-green.ref.png",
};
