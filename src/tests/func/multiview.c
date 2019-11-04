// Copyright 2018 Intel Corporation
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
#include "util/string.h"
#include "util/cru_format.h"

#include "multiview-spirv.h"

// Use VK_KHR_multiview to write various triangles to different views
// with different positions.

struct params {
    unsigned view_count;
    unsigned view_mask;
};

static void
test_multiview()
{
    t_require_ext("VK_KHR_multiview");

    const struct params *params = t_user_data;

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .pNext = &(VkRenderPassMultiviewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = (uint32_t[]) { params->view_mask },
        },
        .attachmentCount = 1,
        .pAttachments = (VkAttachmentDescription[]) {
            {
                QO_ATTACHMENT_DESCRIPTION_DEFAULTS,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
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

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        QO_EXTENSION GL_EXT_multiview : enable

        layout(location = 0) in vec4 in_position;
        layout(location = 1) in vec4 in_color;

        layout(location = 0) out vec4 v_color;
        layout(location = 1) out float v_check;

        vec4 displacement[6] = vec4[](
            vec4( 0.0,  0.0, 0, 0),
            vec4( 0.2,  0.2, 0, 0),
            vec4( 0.4,  0.3, 0, 0),
            vec4(-0.2, -0.2, 0, 0),
            vec4(-0.4,  0.3, 0, 0),
            vec4( 0.4, -0.3, 0, 0)
        );

        void main()
        {
            gl_Position = in_position + displacement[gl_ViewIndex];
            v_color = in_color;
            v_check = 22;
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) in vec4 v_color;
        layout(location = 1) in float v_check;

        layout(location = 0) out vec4 f_color;

        void main()
        {
            if (v_check != 22)
                f_color = vec4(1, 0, 0, 1);
            else
                f_color = v_color;
        }
    );

    const int width = 128;
    const int height = 128;

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 1,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPipelineLayout layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    static const struct {
        float vertices[12][4];
        float colors[12][4];
    } vertex_data = {
        .vertices = {
            {    0, -0.5, 0, 1 },
            {  0.5,  0.0, 0, 1 },
            {    0,  0.0, 0, 1 },

            {    0, -0.5, 0, 1 },
            {    0,  0.0, 0, 1 },
            { -0.5,  0.0, 0, 1 },

            {    0,  0.5, 0, 1 },
            {    0,  0.0, 0, 1 },
            {  0.5,  0.0, 0, 1 },

            {    0,  0.5, 0, 1 },
            {    0,  0.0, 0, 1 },
            { -0.5,  0.0, 0, 1 },
        },
        .colors = {
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },

            { 0, 1, 0, 1 },
            { 0, 1, 0, 1 },
            { 0, 1, 0, 1 },

            { 0, 0, 1, 1 },
            { 0, 0, 1, 1 },
            { 0, 0, 1, 1 },

            { 1, 0, 1, 1 },
            { 1, 0, 1, 1 },
            { 1, 0, 1, 1 },
        },
    };

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = sizeof(vertex_data.vertices[0]),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
            {
                .binding = 1,
                .stride = sizeof(vertex_data.colors[0]),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        },

        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0,
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = 0,
            },
        },
    };

    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, t_pipeline_cache,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .vertexShader = vs,
            .fragmentShader = fs,
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo) {
                QO_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_DEFAULTS,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            },
            .pViewportState = &(VkPipelineViewportStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = &(VkViewport) { 0, 0, width, height, 0, 1 },
                .scissorCount = 1,
                .pScissors = &(VkRect2D) { { 0, 0 }, { width, height } },
            },
            .pVertexInputState = &vi_info,
            .renderPass = pass,
            .layout = layout,
            .subpass = 0,
        }});

    VkImage image = qoCreateImage(t_device,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .mipLevels = 1,
        .arrayLayers = params->view_count,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        });

    VkDeviceMemory image_mem = qoAllocImageMemory(t_device, image);
    qoBindImageMemory(t_device, image, image_mem, 0);

    VkImageView image_view = qoCreateImageView(t_device,
        QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .subresourceRange.layerCount = params->view_count);

    VkBuffer vertex_buffer = qoCreateBuffer(t_device,
        .size = sizeof(vertex_data),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vertex_mem = qoAllocBufferMemory(t_device, vertex_buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *vertex_map = qoMapMemory(t_device, vertex_mem, 0, sizeof(vertex_data), 0);
    qoBindBufferMemory(t_device, vertex_buffer, vertex_mem, 0);
    memcpy(vertex_map, &vertex_data, sizeof(vertex_data));

    VkFramebuffer framebuffer = qoCreateFramebuffer(t_device,
        .renderPass = pass,
        .width = width,
        .height = height,
        .layers = 1,
        .attachmentCount = 1,
        .pAttachments = &image_view);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = pass,
            .framebuffer = framebuffer,
            .renderArea = { { 0, 0 }, { width, height } },
            .clearValueCount = 1,
            .pClearValues = &(VkClearValue) { .color = { .float32 = {0.0, 0.0, 0.0, 1.0} } },
        }, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, sizeof(vertex_data.vertices) });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(t_cmd_buffer, 12, 1, 0, 0);
    vkCmdEndRenderPass(t_cmd_buffer);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    test_result_t result = TEST_RESULT_PASS;

    for (int i = 0; i < params->view_count; i++) {
        string_t ref_name = STRING_INIT;
        if ((params->view_mask & (1 << i)) == 0)
            string_printf(&ref_name, "func.multiview.ref.empty.png");
        else
            string_printf(&ref_name, "func.multiview.ref.%d.png", i);
        cru_image_t *ref = t_new_cru_image_from_filename(string_data(&ref_name));
        string_finish(&ref_name);

        cru_image_t *actual = t_new_cru_image_from_vk_image(t_device,
            t_queue, image, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_ASPECT_COLOR_BIT, width, height,
            /*miplevel*/ 0, /*array_slice*/i);

        t_dump_image_f(actual, "actual.%d.png", i);

        if (!cru_image_compare(actual, ref)) {
            loge("actual and reference images for view %d differ", i);
            result = TEST_RESULT_FAIL;

            string_t actual_name = STRING_INIT;
            string_printf(&actual_name, "%s.actual.%d.png", t_name, i);
            cru_image_write_file(actual, string_data(&actual_name));
            string_finish(&actual_name);
        }
    }

    t_end(result);
}

test_define {
    .name = "func.multiview.count_2",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 2,
        .view_mask = (1 << 2) - 1,
    }
};

test_define {
    .name = "func.multiview.count_2.masked_0",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 2,
        .view_mask = (1 << 0),
    }
};

test_define {
    .name = "func.multiview.count_2.masked_1",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 2,
        .view_mask = (1 << 1),
    }
};

test_define {
    .name = "func.multiview.count_6",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 6,
        .view_mask = (1 << 6) - 1,
    }
};

test_define {
    .name = "func.multiview.count_6.masked_0_2",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 6,
        .view_mask = (1 << 0) | (1 << 2),
    }
};

test_define {
    .name = "func.multiview.count_6.masked_1_3_5",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 6,
        .view_mask = (1 << 1) | (1 << 3) | (1 << 5),
    }
};

test_define {
    .name = "func.multiview.count_6.masked_3_4",
    .start = test_multiview,
    .no_image = true,
    .user_data = &(struct params) {
        .view_count = 6,
        .view_mask = (1 << 3) | (1 << 4),
    }
};
