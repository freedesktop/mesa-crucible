// Copyright 2019 Intel Corporation
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

#include "src/tests/bench/multiview-spirv.h"

#include <math.h>
#include <time.h>

static const int width = 1024;
static const int height = 1024;

static uint64_t
gettime_ns()
{
    struct timespec current;
    int ret = clock_gettime(CLOCK_MONOTONIC, &current);
    t_assert (ret >= 0);
    if (ret < 0)
        return 0;

    return (uint64_t) current.tv_sec * 1000000000ULL + current.tv_nsec;
}

static VkImage
test_view_count(VkBuffer position_buffer, VkBuffer color_buffer,
                unsigned view_count, unsigned triangle_count, unsigned run_count)
{
    VkRenderPass pass = qoCreateRenderPass(t_device,
        .pNext = &(VkRenderPassMultiviewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = (uint32_t[]) { (1 << view_count) - 1 },
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
        QO_EXTENSION GL_EXT_scalar_block_layout : enable

        layout(location = 0) in vec4 in_position;
        layout(location = 1) in vec4 in_color;

        layout(location = 0) out vec4 v_color;

        layout(set = 0, binding = 0, std430) uniform UBO {
            mat4 m;
            int d[16];
        } ubo;

        void main()
        {
            float dd = ubo.d[gl_ViewIndex % 16] - (gl_ViewIndex % 16);
            gl_Position = in_position + dd;
            v_color = in_color;
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

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 1,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPipelineLayout layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = sizeof(float) * 4,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
            {
                .binding = 1,
                .stride = sizeof(float) * 4,
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
        .arrayLayers = view_count,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        });

    VkDescriptorSet set;

    VkResult result = vkAllocateDescriptorSets(t_device,
        &(VkDescriptorSetAllocateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = t_descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &set_layout,
        }, &set);
    t_assert(result == VK_SUCCESS);
    
    /* We just want to force non-trivial work to happen, but not
     * change the result as we check each view will look exactly the
     * same.
     */
    struct {
        float m[16];
        int d[16];
    } ubo_data = {
        .m = { 1, 0, 0, 0,
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1, },
        .d = { 0, 1,  2,  3,  4,  5,  6,  7,
               8, 9, 10, 11, 12, 13, 14, 15, },
    };

    VkBuffer ubo_buf = qoCreateBuffer(t_device,
        .size = sizeof(ubo_data),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkDeviceMemory ubo_mem = qoAllocBufferMemory(t_device, ubo_buf,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, ubo_buf, ubo_mem, 0);

    void *ubo_mapped = NULL;
    vkMapMemory(t_device, ubo_mem, 0, sizeof(ubo_data), 0, &ubo_mapped);
    memcpy(ubo_mapped, &ubo_data, sizeof(ubo_data));
    ubo_mapped = NULL;
    vkUnmapMemory(t_device, ubo_mem);

    vkUpdateDescriptorSets(t_device,
        /*writeCount*/ 1,
        (VkWriteDescriptorSet[]) {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &(VkDescriptorBufferInfo) {
                    .buffer = ubo_buf,
                    .offset = 0,
                    .range = sizeof(ubo_data),
                },
            },
        }, 0, NULL);

    VkDeviceMemory image_mem = qoAllocImageMemory(t_device, image);
    qoBindImageMemory(t_device, image, image_mem, 0);

    VkImageView image_view = qoCreateImageView(t_device,
        QO_IMAGE_VIEW_CREATE_INFO_DEFAULTS,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .subresourceRange.layerCount = view_count);

    unsigned vertex_count = triangle_count * 3;
       
    VkFramebuffer framebuffer = qoCreateFramebuffer(t_device,
        .renderPass = pass,
        .width = width,
        .height = height,
        .layers = 1,
        .attachmentCount = 1,
        .pAttachments = &image_view);

    vkResetCommandBuffer(t_cmd_buffer, 0);
    qoBeginCommandBuffer(t_cmd_buffer);

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

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
        (VkBuffer[]) { position_buffer, color_buffer },
        (VkDeviceSize[]) { 0, 0 });

    vkCmdBindDescriptorSets(t_cmd_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        layout, 0, 1,
        &set, 0, NULL);

    vkCmdDraw(t_cmd_buffer, vertex_count, 1, 0, 0);
    vkCmdEndRenderPass(t_cmd_buffer);

    qoEndCommandBuffer(t_cmd_buffer);

    for (unsigned run = 0; run < run_count; run++) {
        qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
        qoQueueWaitIdle(t_queue);
    }

    vkResetDescriptorPool(t_device, t_descriptor_pool, 0);

    return image;
}

static void
test()
{
    t_require_ext("VK_KHR_multiview");

    VkPhysicalDeviceMultiviewProperties multiview_props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES,
    };
    VkPhysicalDeviceProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &multiview_props,
    };
    vkGetPhysicalDeviceProperties2(t_physical_dev, &props);

    t_assert(multiview_props.maxMultiviewViewCount >= 6);

    const unsigned run_count = 1 << 7;
    const unsigned triangle_count = 1 << 17;
    const unsigned vertex_count = triangle_count * 3;

    /* Set up the data to be used by all tests.  Fill the area with
     * non-overlapping triangles.
     */
    unsigned position_data_size = vertex_count * sizeof(float) * 4;
    float *position_data = calloc(1, position_data_size);

    unsigned color_data_size = vertex_count * sizeof(float) * 4;
    float *color_data = calloc(1, color_data_size);

    const unsigned triangles_per_row = ceil(sqrt(triangle_count));
    const float unit = 2.0f / triangles_per_row;
    float x = -1.0f;
    float y = -1.0f;

    for (unsigned i = 0; i < triangle_count; i++) {
        unsigned base = i * 3 * 4;

        position_data[base +  0] = x + unit/2.0f;
        position_data[base +  1] = y;
        position_data[base +  2] = 0.0f;
        position_data[base +  3] = 1.0f;

        position_data[base +  4] = x;
        position_data[base +  5] = y + unit;
        position_data[base +  6] = 0.0f;
        position_data[base +  7] = 1.0f;

        position_data[base +  8] = x + unit;
        position_data[base +  9] = y + unit;
        position_data[base + 10] = 0.0f;
        position_data[base + 11] = 1.0f;

        const float r = 0.0f;
        const float g = x/2.0f + 0.5f;
        const float b = -y/2.0f + 0.5f;
        const float a = 1.0f;
        
        color_data[base +  0] = r;
        color_data[base +  1] = g;
        color_data[base +  2] = b;
        color_data[base +  3] = a;

        color_data[base +  4] = r;
        color_data[base +  5] = g;
        color_data[base +  6] = b;
        color_data[base +  7] = a;

        color_data[base +  8] = r;
        color_data[base +  9] = g;
        color_data[base + 10] = b;
        color_data[base + 11] = a;

        if (((i + 1) % triangles_per_row) == 0) {
            x = -1.0f;
            y += unit;
        } else {
            x += unit;
        }
    }

    /* Create buffers to be exposed to the pipeline. */
    VkBuffer position_buffer = qoCreateBuffer(t_device,
        .size = position_data_size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory position_mem = qoAllocBufferMemory(t_device, position_buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *position_map = qoMapMemory(t_device, position_mem, 0, position_data_size, 0);
    qoBindBufferMemory(t_device, position_buffer, position_mem, 0);
    memcpy(position_map, position_data, position_data_size);

    free(position_data);   

    VkBuffer color_buffer = qoCreateBuffer(t_device,
        .size = color_data_size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory color_mem = qoAllocBufferMemory(t_device, color_buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *color_map = qoMapMemory(t_device, color_mem, 0, color_data_size, 0);
    qoBindBufferMemory(t_device, color_buffer, color_mem, 0);
    memcpy(color_map, color_data, color_data_size);

    free(color_data);

    cru_image_t *reference = NULL;

    for (unsigned i = 1; i <= multiview_props.maxMultiviewViewCount; i++) {
        uint64_t start = gettime_ns();

        VkImage image = test_view_count(position_buffer, color_buffer, i, triangle_count, run_count);

        uint64_t end = gettime_ns();

        logi("Average time from %u runs for %u views and %u triangles: %f seconds",
            run_count, i, triangle_count,
            ((double)(end - start) / 1000000000.0) / run_count);

        /* Basic self check against the case 1.  In this benchmark all
         * layers are the same, so everything can be compared with
         * that.  This allows easily tweaking the parameters and still
         * perform some verification on the result.
         *
         * TODO: This doesn't ensure correctness.  Ideally we could
         * make all layers different and pre-render the reference for
         * each.
         */
        if (i == 1) {
            reference = t_new_cru_image_from_vk_image(t_device, t_queue, image,
                VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, width, height,
                /*miplevel*/ 0, /*array_slice*/ 0);

            string_t reference_name = STRING_INIT;
            string_printf(&reference_name, "%s.reference.png", t_name);
            cru_image_write_file(reference, string_data(&reference_name));
            string_finish(&reference_name);

        } else {
            for (unsigned j = 0; j < i; j++) {
                cru_image_t *actual = t_new_cru_image_from_vk_image(t_device, t_queue, image,
                    VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, width, height,
                    /*miplevel*/ 0, /*array_slice*/ j);

                if (!cru_image_compare(actual, reference)) {
                    loge("wrong rendering for multiview with %u views in layer %u", i, j);

                    string_t reference_name = STRING_INIT;
                    string_printf(&reference_name, "%s.reference.png", t_name);
                    cru_image_write_file(reference, string_data(&reference_name));

                    string_t actual_name = STRING_INIT;
                    string_printf(&actual_name, "%s.actual.%u.%u.png", t_name, i, j);
                    cru_image_write_file(actual, string_data(&actual_name));

                    loge("dumped %s and %s images", string_data(&reference_name),
                                                    string_data(&actual_name));

                    string_finish(&reference_name);
                    string_finish(&actual_name);
                    
                    t_end(TEST_RESULT_FAIL);
                }
            }          
        }
    }
}

test_define {
    .name = "bench.multiview",
    .start = test,
    .no_image = true,
};
