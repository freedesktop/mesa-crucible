// Copyright 2015 Intel Corporation
// Copyright 2017 Valve Corporation
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

#include "util/simple_pipeline.h"
#include "util/misc.h"

#include "tapi/t.h"

#include "simple_pipeline-spirv.h"

void
run_simple_pipeline(VkShaderModule fs, void *push_constants,
                    size_t push_constants_size)
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
                .preserveAttachmentCount = 1,
                .pPreserveAttachments = (uint32_t[]) { 0 },
            }
        });

    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
        layout(location = 0) in vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    VkPipelineVertexInputStateCreateInfo vi_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = (VkVertexInputBindingDescription[]) {
            {
                .binding = 0,
                .stride = 8,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            },
        },
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[]) {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0
            },
        }
    };

    VkPipelineLayout layout;
    if (push_constants_size) {
        VkPushConstantRange constants = {
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = push_constants_size
        };

        layout = qoCreatePipelineLayout(t_device,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &constants);
    } else {
        layout = qoCreatePipelineLayout(t_device, .pushConstantRangeCount = 0);
    }

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
            .pVertexInputState = &vi_info,
            .renderPass = pass,
            .layout = layout,
            .subpass = 0,
        }});

    const float vertices[] = {
        -1.0, 1.0,
        1.0, 1.0,
        -1.0, -1.0,
        1.0, -1.0
    };

    VkBuffer vb = qoCreateBuffer(t_device, .size =  sizeof(vertices),
                                 .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceMemory vb_mem = qoAllocBufferMemory(t_device, vb,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, vb, vb_mem, 0);

    void *vb_map = qoMapMemory(t_device, vb_mem, 0, sizeof(vertices), 0);

    memcpy(vb_map, vertices, sizeof(vertices));

    vkCmdBeginRenderPass(t_cmd_buffer,
        &(VkRenderPassBeginInfo) {
            .renderPass = pass,
            .framebuffer = t_framebuffer,
            .renderArea = { { 0, 0 }, { t_width, t_height } },
            .clearValueCount = 1,
            .pClearValues = (VkClearValue[]) {
                { .color = { .float32 = {1.0, 0.0, 0.0, 1.0} } },
            }
        }, VK_SUBPASS_CONTENTS_INLINE);

    if (push_constants_size) {
        vkCmdPushConstants(t_cmd_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, push_constants_size, push_constants);
    }

    vkCmdBindVertexBuffers(t_cmd_buffer, 0, 1, (VkBuffer[]) { vb },
                           (VkDeviceSize[]) { 0 });

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDraw(t_cmd_buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
}


void run_simple_compute_pipeline(VkShaderModule cs,
                                 const simple_compute_pipeline_options_t *opts)
{
    bool has_push_constants = opts->push_constants_size > 0;
    bool has_storage = opts->storage_size > 0;

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(t_device,
        .bindingCount = 1,
        .pBindings = (VkDescriptorSetLayoutBinding[]) {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .pImmutableSamplers = NULL,
            },
        });

    VkPushConstantRange constant_range = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = opts->push_constants_size,
    };

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
        .setLayoutCount = has_storage ? 1 : 0,
        .pSetLayouts = has_storage ? &set_layout : NULL,
        .pushConstantRangeCount = has_push_constants ? 1 : 0,
        .pPushConstantRanges = has_push_constants ? &constant_range : NULL);

    VkPipeline pipeline;
    VkResult result = vkCreateComputePipelines(t_device, t_pipeline_cache, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = cs,
                .pName = "main",
            },
            .flags = 0,
            .layout = pipeline_layout
        }, NULL, &pipeline);
    t_assert(result == VK_SUCCESS);
    t_cleanup_push_vk_pipeline(t_device, pipeline);

    VkDescriptorSet set = qoAllocateDescriptorSet(t_device,
        .descriptorPool = t_descriptor_pool,
        .pSetLayouts = &set_layout);

    VkBuffer storage_buf;
    VkDeviceMemory storage_mem;
    if (has_storage) {
        storage_buf = qoCreateBuffer(t_device,
            .size = opts->storage_size,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        storage_mem = qoAllocBufferMemory(t_device, storage_buf,
            .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        qoBindBufferMemory(t_device, storage_buf, storage_mem, 0);

        // Copy storage to buffer.
        void *storage_ptr = NULL;
        VkResult result = vkMapMemory(t_device, storage_mem, 0,
                                      opts->storage_size, 0, &storage_ptr);
        t_assert(result == VK_SUCCESS);
        memcpy(storage_ptr, opts->storage, opts->storage_size);
        vkUnmapMemory(t_device, storage_mem);

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
                        .buffer = storage_buf,
                        .offset = 0,
                        .range = opts->storage_size,
                    },
                },
            }, 0, NULL);
    }

    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(t_cmd_buffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipeline_layout, 0, 1,
        &set, 0, NULL);

    if (has_push_constants) {
        vkCmdPushConstants(t_cmd_buffer, pipeline_layout,
            VK_SHADER_STAGE_COMPUTE_BIT, 0,
            opts->push_constants_size, opts->push_constants);
    }

    vkCmdDispatch(t_cmd_buffer, MAX(1, opts->x_count),
                  MAX(1, opts->y_count), MAX(1, opts->z_count));

    qoEndCommandBuffer(t_cmd_buffer);
    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    // Copy storage back.
    if (has_storage) {
        void *storage_ptr = NULL;
        VkResult result = vkMapMemory(t_device, storage_mem, 0,
                                      opts->storage_size, 0, &storage_ptr);
        t_assert(result == VK_SUCCESS);
        memcpy(opts->storage, storage_ptr, opts->storage_size);
        vkUnmapMemory(t_device, storage_mem);
    }
}

