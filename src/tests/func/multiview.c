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

// Use VK_KHR_multiview to write a triangle to two different views
// with different positions.

static void
test_multiview(unsigned view_count, unsigned view_mask)
{
    t_require_ext("VK_KHR_multiview");

    VkRenderPass pass = qoCreateRenderPass(t_device,
        .pNext = &(VkRenderPassMultiviewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = (uint32_t[]) { view_mask },
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

        layout(location = 0) out vec4 v_color;

        vec4 positions[3] = vec4[](
            vec4(   0, -0.5, 0, 1),
            vec4( 0.7,  0.5, 0, 1),
            vec4(-0.7,  0.5, 0, 1)
        );

        vec4 colors[3] = vec4[](
            vec4(0, 1, 1, 1),
            vec4(0, 1, 0, 1),
            vec4(0, 0, 1, 1)
        );

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
            gl_Position = displacement[gl_ViewIndex] + positions[gl_VertexIndex];
            v_color = colors[gl_VertexIndex];
        }
    );

    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        layout(location = 0) in vec4 v_color;
        layout(location = 0) out vec4 f_color;

        void main()
        {
            f_color = v_color;
        }
    );

    const int width = 128;
    const int height = 128;

    VkPipelineLayout layout = qoCreatePipelineLayout(t_device);

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
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            },
            .pViewportState = &(VkPipelineViewportStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = &(VkViewport) { 0, 0, width, height, 0, 1 },
                .scissorCount = 1,
                .pScissors = &(VkRect2D) { { 0, 0 }, { width, height } },
            },
            .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            },
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
        .arrayLayers = view_count,
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
        .subresourceRange.layerCount = view_count);

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

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(t_cmd_buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(t_cmd_buffer);

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    test_result_t result = TEST_RESULT_PASS;

    for (int i = 0; i < view_count; i++) {
        string_t ref_name = STRING_INIT;
        if ((view_mask & (1 << i)) == 0)
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


static void
test_multiview_count_2(void)
{
    test_multiview(2, 0x3);
}

test_define {
    .name = "func.multiview.count_2",
    .start = test_multiview_count_2,
    .no_image = true,
};


static void
test_multiview_count_2_masked_0(void)
{
    test_multiview(2, (1 << 0));
}

test_define {
    .name = "func.multiview.count_2.masked_0",
    .start = test_multiview_count_2_masked_0,
    .no_image = true,
};


static void
test_multiview_count_2_masked_1(void)
{
    test_multiview(2, (1 << 1));
}

test_define {
    .name = "func.multiview.count_2.masked_1",
    .start = test_multiview_count_2_masked_1,
    .no_image = true,
};


static void
test_multiview_count_6(void)
{
    test_multiview(6, (1 << 6) - 1);
}

test_define {
    .name = "func.multiview.count_6",
    .start = test_multiview_count_6,
    .no_image = true,
};


static void
test_multiview_count_6_masked_0_2(void)
{
    test_multiview(6, (1 << 0) | (1 << 2));
}

test_define {
    .name = "func.multiview.count_6.masked_0_2",
    .start = test_multiview_count_6_masked_0_2,
    .no_image = true,
};


static void
test_multiview_count_6_masked_1_3_5(void)
{
    test_multiview(6, (1 << 1) | (1 << 3) | (1 << 5));
}

test_define {
    .name = "func.multiview.count_6.masked_1_3_5",
    .start = test_multiview_count_6_masked_1_3_5,
    .no_image = true,
};


static void
test_multiview_count_6_masked_3_4(void)
{
    test_multiview(6, (1 << 3) | (1 << 4));
}

test_define {
    .name = "func.multiview.count_6.masked_3_4",
    .start = test_multiview_count_6_masked_3_4,
    .no_image = true,
};
