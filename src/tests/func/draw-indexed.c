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

#include "src/tests/func/draw-indexed-spirv.h"

struct params {
    const void *index_data;
    size_t index_data_size;
    bool use_restart;
    uint32_t start_instance;
    uint32_t num_instances;
    VkIndexType index_type;
};

static void
test(void)
{
    VkPipeline pipeline, restart_pipeline;
    VkBuffer buffer;
    void *map;

    size_t buffer_size = 4096;

    buffer = qoCreateBuffer(t_device, .size = buffer_size,
                            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    map = qoMapMemory(t_device, mem, 0, buffer_size, 0);
    qoBindBufferMemory(t_device, buffer, mem, 0);

#define HEX_COLOR(v, f)                         \
    {                                           \
        ((v) >> 16) / 255.0,                    \
        ((v & 0xff00) >> 8) / 255.0,            \
        ((v & 0xff)) / 255.0,                   \
        f                                       \
    }

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        layout(location = 1) in vec4 a_color;
        layout(location = 0) out vec4 v_color;
        void main()
        {
            gl_Position = a_position + vec4(0, a_color.a, 0, 0);
            v_color = vec4(a_color.rgb, 1.0);
        }
    );

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device);

    pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .vertexShader = vs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo) {
                QO_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_DEFAULTS,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                .primitiveRestartEnable = false,
            },
            .layout = pipeline_layout,
            .renderPass = t_render_pass,
            .subpass = 0,
        }});

    restart_pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .vertexShader = vs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo) {
                QO_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_DEFAULTS,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                .primitiveRestartEnable = true,
            },
            .layout = pipeline_layout,
            .renderPass = t_render_pass,
            .subpass = 0,
        }});

    static const struct {
        float vertices[256][2];
        float colors[5][4];
    } vertex_data = {
        .vertices = {
            [100] =
            { -0.6,  0.2 },
            { -0.4, -0.2 },
            { -0.2,  0.2 },
            { -0.0, -0.2 },
            {  0.2,  0.2 },
            {  0.4, -0.2 },
            {  0.6,  0.2 }
        },
        .colors = {
            HEX_COLOR(0xa6d49f, -0.72),
            HEX_COLOR(0xe06d06, -0.24),
            HEX_COLOR(0xc59849,  0.24),
            HEX_COLOR(0x9cb380,  0.72),
        }
    };

    memcpy(map, &vertex_data, sizeof(vertex_data));

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = t_render_pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = HEX_COLOR(0x522a27, 1.0) } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { buffer, buffer },
                           (VkDeviceSize[]) { 0, sizeof(vertex_data.vertices) });

    const struct params *params = t_user_data;
    memcpy(map + 1024, params->index_data, params->index_data_size);
    vkCmdBindIndexBuffer(t_cmd_buffer, buffer, 1024, params->index_type);

    if (params->use_restart)
        vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, restart_pipeline);
    else
        vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // Tests instanced, index rendering with negative base vertex and non-zero
    // start index.

    const uint32_t first_index = 1;
    const uint32_t vertex_count = 7;
    const int32_t vertex_offset = -1;

    vkCmdDrawIndexed(t_cmd_buffer, vertex_count, params->num_instances,
                     first_index, vertex_offset, params->start_instance);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}

// Tests uint16 index type
static const uint16_t index_data16[] = { 50, 101, 102, 103, 104, 105, 106, 107 };

test_define {
    .name = "func.draw-index16",
    .start = test,
    .user_data = &(struct params) {
        .index_data = index_data16,
        .index_data_size = sizeof(index_data16),
        .use_restart = false,
        .start_instance = 0,
        .num_instances = 2,
        .index_type = VK_INDEX_TYPE_UINT16
    }
};

// Tests restart index
static const uint16_t index_data16_restart[] = { 50, 101, 102, 103, ~0, 105, 106, 107 };

test_define {
    .name = "func.draw-index16-restart",
    .start = test,
    .user_data = &(struct params) {
        .index_data = index_data16_restart,
        .index_data_size = sizeof(index_data16_restart),
        .use_restart = true,
        .start_instance = 2,
        .num_instances = 1,
        .index_type = VK_INDEX_TYPE_UINT16
    }
};

// Tests uint32 index type
static const uint32_t index_data32_restart[] = { 50, 101, 102, 103, ~0, 105, 106, 107 };

test_define {
    .name = "func.draw-index32-restart",
    .start = test,
    .user_data = &(struct params) {
        .index_data = index_data32_restart,
        .index_data_size = sizeof(index_data32_restart),
        .use_restart = true,
        .start_instance = 3,
        .num_instances = 1,
        .index_type = VK_INDEX_TYPE_UINT32
    }
};
