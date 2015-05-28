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

#include <crucible/cru.h>

static void
create_pipeline(VkDevice device, VkPipeline *pipeline,
                VkPipelineLayout pipeline_layout)
{
    static const char vs_source[] = GLSL(330,
        layout(location = 0) in vec4 a_position;
        layout(location = 1) in vec4 a_color;
        layout(set = 0, binding = 0) uniform block1 {
            vec4 color;
        } u1;
        layout(set = 0, binding = 1) uniform block2 {
            vec4 color;
        } u2;
        layout(set = 1, binding = 0) uniform block3 {
            vec4 color;
        } u3;
        out vec4 v_color;
        void main()
        {
            gl_Position = a_position;
            v_color = a_color + u1.color + u2.color + u3.color;
        }
    );

    static const char fs_source[] = GLSL(330,
        out vec4 f_color;
        in vec4 v_color;
        layout(set = 0, binding = 0) uniform sampler2D tex;
        void main()
        {
            f_color = v_color + texture2D(tex, vec2(0.1, 0.1));
        }
    );

    VkShader vs;
    vkCreateShader(t_device,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .codeSize = sizeof(vs_source),
            .pCode = vs_source,
            .flags = 0,
        },
        &vs);

    VkShader fs;
    vkCreateShader(t_device,
        &(VkShaderCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO,
            .codeSize = sizeof(fs_source),
            .pCode = fs_source,
            .flags = 0,
        },
        &fs);

    VkPipelineVertexInputCreateInfo vi_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_CREATE_INFO,
        .bindingCount = 2,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .strideInBytes = 16, // TODO: What is this? rgba_f32? xyzw_f32?
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
            },
            {
                .binding = 1,
                .strideInBytes = 0,
                .stepRate = VK_VERTEX_INPUT_STEP_RATE_VERTEX,
            },
        },
        .attributeCount = 2,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0,
            },
            {
                .location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offsetInBytes = 0,
            },
        },
    };

    struct cru_GraphicsPipelineCreateInfo cru_info = {
        .sType = CRU_STRUCTURE_TYPE_PIPELINE_CREATE_INFO,
        .pNext = &vi_create_info,

        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .vertex_shader = vs,
        .fragment_shader = fs,
    };

    cru_CreateGraphicsPipeline(t_device,
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &cru_info,
            .flags = 0,
            .layout = pipeline_layout
        },
        pipeline);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_PIPELINE,
                                    *pipeline);
}

static void
test(void)
{
    VkDescriptorSetLayout set_layout[2];
    vkCreateDescriptorSetLayout(t_device,
        &(VkDescriptorSetLayoutCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .count = 3,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .count = 2,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .count = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .count = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL,
                },
            },
        },
        &set_layout[0]);

    vkCreateDescriptorSetLayout(t_device,
        &(VkDescriptorSetLayoutCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .count = 1,
            .pBinding = (VkDescriptorSetLayoutBinding[]) {
                {
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .count = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL,
                },
            },
        },
        &set_layout[1]);

    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(t_device,
        &(VkPipelineLayoutCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .descriptorSetCount = 2,
            .pSetLayouts = set_layout,
        },
        &pipeline_layout);

    VkPipeline pipeline;
    create_pipeline(t_device, &pipeline, pipeline_layout);

    uint32_t set_count = 1;
    VkDescriptorSet set[2];
    vkAllocDescriptorSets(t_device, /*pool*/ 0,
                          VK_DESCRIPTOR_SET_USAGE_STATIC,
                          2, set_layout, set, &set_count);

    VkBuffer buffer;
    vkCreateBuffer(t_device,
        &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 1024,
            .usage = VK_BUFFER_USAGE_GENERAL,
            .flags = 0
            },
        &buffer);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_BUFFER,
                                    buffer);

    VkMemoryRequirements buffer_reqs;
    size_t buffer_reqs_size = sizeof(buffer_reqs);
    vkGetObjectInfo(t_device, VK_OBJECT_TYPE_BUFFER, buffer,
                    VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS,
                    &buffer_reqs_size, &buffer_reqs);

    VkBuffer vertex_buffer;
    vkCreateBuffer(t_device,
        &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 1024,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .flags = 0,
        },
        &vertex_buffer);

    VkMemoryRequirements vb_reqs;
    size_t vb_reqs_size = sizeof(vb_reqs);
    vkGetObjectInfo(t_device, VK_OBJECT_TYPE_BUFFER, vertex_buffer,
                    VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS,
                    &vb_reqs_size, &vb_reqs);

    VkBuffer image_buffer;
    vkCreateBuffer(t_device,
        &(VkBufferCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 4 * t_width * t_height,
            .usage = VK_BUFFER_USAGE_TRANSFER_DESTINATION_BIT,
            .flags = 0
        },
        &image_buffer);

    VkMemoryRequirements ib_reqs;
    size_t ib_reqs_size = sizeof(ib_reqs);
    vkGetObjectInfo(t_device, VK_OBJECT_TYPE_BUFFER, image_buffer,
                    VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS,
                    &ib_reqs_size, &ib_reqs);

    size_t mem_size = ib_reqs.size + 2048 + 16 * 16 * 4;

    VkDeviceMemory mem;
    vkAllocMemory(t_device,
        &(VkMemoryAllocInfo) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO,
            .allocationSize = mem_size,
            .memProps = VK_MEMORY_PROPERTY_HOST_DEVICE_COHERENT_BIT,
            .memPriority = VK_MEMORY_PRIORITY_NORMAL,
        },
        &mem);

    size_t image_mem_size = ib_reqs.size;
    VkDeviceMemory image_mem;
    vkAllocMemory(t_device,
        &(VkMemoryAllocInfo) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO,
            .allocationSize = image_mem_size,
            .memProps = VK_MEMORY_PROPERTY_HOST_DEVICE_COHERENT_BIT,
            .memPriority = VK_MEMORY_PRIORITY_NORMAL,
        },
        &image_mem);

    void *map;
    vkMapMemory(t_device, mem, 0, mem_size, 0, &map);
    memset(map, 192, mem_size);

    void *image_map;
    vkMapMemory(t_device, image_mem, 0, image_mem_size, 0, &image_map);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER, buffer,
                            /*index*/ 0, mem, 128);

    static const float color[12] = {
        0.0, 0.2, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.5, 0.5
    };
    memcpy(map + 128 + 16, color, sizeof(color));
    VkBufferView buffer_view[3];
    vkCreateBufferView(t_device,
        &(VkBufferViewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .buffer = buffer,
            .viewType = VK_BUFFER_VIEW_TYPE_RAW,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = 16,
            .range = 64
        },
        &buffer_view[0]);

    vkCreateBufferView(t_device,
        &(VkBufferViewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .buffer = buffer,
            .viewType = VK_BUFFER_VIEW_TYPE_RAW,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = 32,
            .range = 64
        },
        &buffer_view[1]);

    vkCreateBufferView(t_device,
        &(VkBufferViewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .buffer = buffer,
            .viewType = VK_BUFFER_VIEW_TYPE_RAW,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = 48,
            .range = 64,
        },
        &buffer_view[2]);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER,
                            vertex_buffer, /*index*/ 0, mem, 1024);

    static const float vertex_data[] = {
        // Triangle coordinates
        -0.5, -0.5, 0.0, 1.0,
         0.5, -0.5, 0.0, 1.0,
         0.0,  0.5, 0.0, 1.0,

         // Color
         1.0,  0.0, 0.0, 0.2,
    };

    memcpy(map + 1024, vertex_data, sizeof(vertex_data));

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_BUFFER, image_buffer,
                            /*index*/ 0, image_mem, 0);

    const uint32_t texture_width = 16, texture_height = 16;

    VkImage texture;
    vkCreateImage(t_device,
        &(VkImageCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = {
                .width = texture_width,
                .height = texture_height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arraySize = 1,
            .samples = 1,
            .tiling = VK_IMAGE_TILING_LINEAR,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
            .flags = 0,
        },
        &texture);
    t_cleanup_push_vk_object(t_device, VK_OBJECT_TYPE_IMAGE, texture);

    vkQueueBindObjectMemory(t_queue, VK_OBJECT_TYPE_IMAGE, texture,
                            /*index*/ 0, mem, 2048 + 256 * 256 * 4);

    VkImageView tex_view;
    vkCreateImageView(t_device,
        &(VkImageViewCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .channels = {
                VK_CHANNEL_SWIZZLE_R,
                VK_CHANNEL_SWIZZLE_G,
                VK_CHANNEL_SWIZZLE_B,
                VK_CHANNEL_SWIZZLE_A
            },
            .subresourceRange = {
                .aspect = VK_IMAGE_ASPECT_COLOR,
                .baseMipLevel = 0,
                .mipLevels = 1,
                .baseArraySlice = 0,
                .arraySize = 1
            },
            .minLod = 0
        },
        &tex_view);

    VkSampler sampler;
    vkCreateSampler(t_device,
        &(VkSamplerCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_TEX_FILTER_LINEAR,
            .minFilter = VK_TEX_FILTER_LINEAR,
            .mipMode = VK_TEX_MIPMAP_MODE_NEAREST,
            .addressU = VK_TEX_ADDRESS_CLAMP,
            .addressV = VK_TEX_ADDRESS_CLAMP,
            .addressW = VK_TEX_ADDRESS_CLAMP,
            .mipLodBias = 0,
            .maxAnisotropy = 0,
            .compareOp = VK_COMPARE_OP_GREATER,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_TRANSPARENT_BLACK,
           },
       &sampler);

   vkUpdateDescriptors(t_device,
        set[0], 3,
        (const void * []) {
            &(VkUpdateBuffers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_BUFFERS,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .arrayIndex = 0,
                .binding = 0,
                .count = 2,
                .pBufferViews = (VkBufferViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view[0],
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view[1],
                    },
                },
            },
            &(VkUpdateImages) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_IMAGES,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .binding = 2,
                .count = 1,
                .pImageViews = (VkImageViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ATTACH_INFO,
                        .view = tex_view,
                        .layout = VK_IMAGE_LAYOUT_GENERAL,
                    },
                },
            },
            &(const VkUpdateSamplers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_SAMPLERS,
                .binding = 3,
                .count = 1,
                .pSamplers = (const VkSampler[]) { sampler }
            },
        });

    vkUpdateDescriptors(t_device,
        set[1], 1,
        (const void * []) {
            &(VkUpdateBuffers) {
                .sType = VK_STRUCTURE_TYPE_UPDATE_BUFFERS,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .arrayIndex = 0,
                .count = 1,
                .pBufferViews = (VkBufferViewAttachInfo[]) {
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_ATTACH_INFO,
                        .view = buffer_view[2]
                    },
                },
            },
        });

    VkRenderPass pass;
    vkCreateRenderPass(t_device,
        &(VkRenderPassCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .renderArea = {{0, 0}, {t_width, t_height}},
            .colorAttachmentCount = 1,
            .extent = { },
            .sampleCount = 1,
            .layers = 1,
            .pColorFormats = (VkFormat[]) { VK_FORMAT_R8G8B8A8_UNORM },
            .pColorLayouts = (VkImageLayout[]) { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
            .pColorLoadOps = (VkAttachmentLoadOp[]) { VK_ATTACHMENT_LOAD_OP_CLEAR },
            .pColorStoreOps = (VkAttachmentStoreOp[]) { VK_ATTACHMENT_STORE_OP_STORE },
            .pColorLoadClearValues = (VkClearColor[]) {
                {
                    .color = { .floatColor = { 1.0, 0.0, 0.0, 1.0 } },
                    .useRawValue = false,
                },
            },
            .depthStencilFormat = VK_FORMAT_UNDEFINED,
        },
        &pass);

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBegin) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
        });
    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 2,
                           (VkBuffer[]) { vertex_buffer, vertex_buffer },
                           (VkDeviceSize[]) { 0, 3 * 4 * sizeof(float) });
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 0, 1,
                            &set[0], 0, NULL);
    vkCmdBindDescriptorSets(t_cmd_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 1, 1,
                            &set[1], 0, NULL);
    vkCmdDraw(t_cmd_buffer, 0, 3, 0, 1);
    vkCmdEndRenderPass(t_cmd_buffer, pass);
}

cru_define_test {
    .name = "func.first",
    .start = test,
};
