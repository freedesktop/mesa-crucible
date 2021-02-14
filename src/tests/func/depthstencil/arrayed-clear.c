// Copyright 2016 Intel Corporation
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

/// \file
/// \brief Test texturing from a cleared arrayed depth buffer.
///
//
#include "tapi/t.h"
#include <stdio.h>
#include "src/tests/func/depthstencil/arrayed-clear-spirv.h"

#define CLEAR_VALUE 0.4f
#define INIT_VALUE 0.19f

static void
test(void)
{
    // Create and begin the Renderpass that clears the depth buffer
    // array and transitions it to a samplable layout.
    VkAttachmentDescription fc_desc = { 
	    .format = VK_FORMAT_D32_SFLOAT,
	    .samples = VK_SAMPLE_COUNT_1_BIT,
	    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    VkRenderPass pass = qoCreateRenderPass(t_device,
	    .attachmentCount = 1,
	    .pAttachments = &fc_desc,
	    .subpassCount = 1,
	    .pSubpasses = (VkSubpassDescription[]) { {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.pDepthStencilAttachment = &(VkAttachmentReference){
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	        }
	    }},
	    .dependencyCount = 1,
	    .pDependencies = &(VkSubpassDependency) {
		    .srcSubpass = 0,
		    .dstSubpass = VK_SUBPASS_EXTERNAL,
		    .srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		    .dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		    .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		    .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
	    });

    // Create the arrayed image that will be cleared
    VkImage depth_test = qoCreateImage(t_device,
            .format = VK_FORMAT_D32_SFLOAT,
	    .arrayLayers = 2,
            .extent = {
                .width = 1,
                .height = 1,
                .depth = 1,
            },
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkDeviceMemory depth_mem1 = qoAllocImageMemory(t_device, depth_test,
        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    qoBindImageMemory(t_device, depth_test, depth_mem1, 0);

    VkImageView depth_test_view = qoCreateImageView(t_device,
		    .viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
		    .format = VK_FORMAT_D32_SFLOAT,
		    .image = depth_test,
		    .subresourceRange = {
		    	.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.levelCount = 1,
			.layerCount = 2,
			});

    VkFramebuffer fb = qoCreateFramebuffer(t_device, 
		    .renderPass = pass,
		    .attachmentCount = 1,
		    .pAttachments = &depth_test_view,
		    .width = 1, .height = 1, .layers = 2);

    // Perform the clear
    vkCmdBeginRenderPass(t_cmd_buffer, &(VkRenderPassBeginInfo) {
		    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		    .renderPass = pass,
		    .framebuffer = fb,
		    .renderArea = (VkRect2D){.extent = (VkExtent2D){
		       .width = 1, .height = 1
		       },},
		    .clearValueCount = 1,
		    .pClearValues = (VkClearValue[]) {
			    {.depthStencil = {.depth = CLEAR_VALUE}},
		    	}
		    },
		    VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(t_cmd_buffer);

    // The next set of steps will sample from the depth buffer layers and
    // store the results.

    // Create a buffer to hold UBO data, the sample location will be hard-coded
    VkBuffer buffer = qoCreateBuffer(t_device, .size = 4096, .usage =
		    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkDeviceMemory buffer_mem = qoAllocBufferMemory(t_device, buffer, 
        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    qoBindBufferMemory(t_device, buffer, buffer_mem, 0);

    // Initialize the buffer floats to a known value
    float *buffer_map = (float*)qoMapMemory(t_device, buffer_mem,
                                       /*offset*/ 0, 2*sizeof(float), 0);
    buffer_map[0] = INIT_VALUE;
    buffer_map[1] = INIT_VALUE;

    // Create shaders to read and write
    VkShaderModule vs = qoCreateShaderModuleGLSL(t_device, VERTEX,
	layout (set=0, binding = 0) uniform sampler2DArray depth_tex;
        layout (std430, binding = 1) buffer buf_output
	{
	    vec2 buf_out;
        };
        void main()
        {
	    vec4 color1 =  texelFetch(depth_tex, ivec3(0,0,0),0);
	    vec4 color2 =  texelFetch(depth_tex, ivec3(0,0,1),0);
	    buf_out.x = color1.x;
	    buf_out.y = color2.x;
        }
    );
    VkShaderModule fs = qoCreateShaderModuleGLSL(t_device, FRAGMENT,
        void main()
        {
        }
    );

    // Create a pipeline layout to describe the layout of the shader resources
    VkSampler sampler = qoCreateSampler(t_device,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0,
        .maxAnisotropy = 1.0,
        .compareOp = VK_COMPARE_OP_GREATER,
        .minLod = 0,
        .maxLod = 0,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);

    VkDescriptorSetLayout ds_layout = qoCreateDescriptorSetLayout(t_device,
		    .bindingCount = 2,
		    .pBindings = (VkDescriptorSetLayoutBinding[]) {
		    {
		    	.binding = 0,
			.descriptorType =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = &sampler,
		    },
		    {
		    	.binding = 1,
			.descriptorType =
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		    }});

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(t_device,
		    .setLayoutCount = 1,
		    .pSetLayouts = &ds_layout);

    // Create a descriptor set that the shaders will access
    VkDescriptorSet desc_set =
        qoAllocateDescriptorSet(t_device,
                                .descriptorPool = t_descriptor_pool,
                                .pSetLayouts = &ds_layout);
    vkUpdateDescriptorSets(t_device, 2, (VkWriteDescriptorSet[]) {
	    {
	    	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		    .dstSet = desc_set,
		    .dstBinding = 0,
		    .dstArrayElement = 0,
		    .descriptorCount = 1,
		    .descriptorType =
		    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		    .pImageInfo = &(VkDescriptorImageInfo) {
		    	.imageView = depth_test_view,
			.imageLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		       },
	    },
	    {
	    	    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	     	    .dstSet = desc_set,
		    .dstBinding = 1,
		    .dstArrayElement = 0,
		    .descriptorCount = 1,
		    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		    .pBufferInfo = &(VkDescriptorBufferInfo) {
		    	.buffer = buffer,
			.offset = 0,
			.range = 2*sizeof(float),
		    },
	    },
		    }, 0, NULL);


    // Create a single-subpass render pass that has no attachments
    VkRenderPass fetch_pass = qoCreateRenderPass(t_device,
	    .subpassCount = 1,
	    .pSubpasses = (VkSubpassDescription[]) {
	    {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	    }});

    // Create an empty framebuffer
    VkFramebuffer fetch_fb = qoCreateFramebuffer(t_device, 
		    .renderPass = fetch_pass,
		    .width = 1, .height = 1);

    // Create a graphics pipeline that will hold the shaders and other fixed
    // function state (unused)
    VkPipeline pipeline = qoCreateGraphicsPipeline(t_device, VK_NULL_HANDLE,
        &(QoExtraGraphicsPipelineCreateInfo) {
            QO_EXTRA_GRAPHICS_PIPELINE_CREATE_INFO_DEFAULTS,
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            .vertexShader = vs,
            .fragmentShader = fs,
            .dynamicStates = (1 << VK_DYNAMIC_STATE_VIEWPORT) |
                             (1 << VK_DYNAMIC_STATE_SCISSOR),
            .pNext =
        &(VkGraphicsPipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .layout = pipeline_layout,
            .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            },
            .pColorBlendState = &(VkPipelineColorBlendStateCreateInfo) {
		    .sType =
		    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,},
            .renderPass = fetch_pass,
            .subpass = 0,
        }});

    // Perform the sample and write
    vkCmdBeginRenderPass(t_cmd_buffer, &(VkRenderPassBeginInfo) {
		    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		    .renderPass = fetch_pass,
		    .framebuffer = fetch_fb,
		    .renderArea = (VkRect2D){.extent = (VkExtent2D){
		       .width = 1, .height = 1
		       },},
		    },
		    VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		    pipeline);
    vkCmdBindDescriptorSets(t_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		    pipeline_layout, 0, 1, &desc_set, 0, NULL);

    vkCmdSetViewport(t_cmd_buffer, 0, 1,
		     &(VkViewport) {
		       .width = 1,
		       .height = 1,
		     });

    vkCmdSetScissor(t_cmd_buffer, 0, 1,
		     &(VkRect2D) {
		       .extent.width = 1,
		       .extent.height = 1,
		     });
    vkCmdDraw(t_cmd_buffer, 1, 1, 0, 0);

    vkCmdEndRenderPass(t_cmd_buffer);
    vkEndCommandBuffer(t_cmd_buffer);

    qoQueueSubmit(t_queue, 1, &t_cmd_buffer, VK_NULL_HANDLE);
    qoQueueWaitIdle(t_queue);

    // Compare the expected and actual values
    t_assertf(CLEAR_VALUE == buffer_map[0] &&
              CLEAR_VALUE == buffer_map[1], "\n"
	    "layer %d: Expected %.1f, Actual %.1f\n"
	    "layer %d: Expected %.1f, Actual %.1f",
	    0, CLEAR_VALUE, buffer_map[0],
	    1, CLEAR_VALUE, buffer_map[1]);
    t_pass();
}

test_define {
    .name = "func.depthstencil.arrayed_clear",
    .start = test,
    .no_image = true,
};

