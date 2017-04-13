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

#include "semaphore-fd-spirv.h"

struct test_context {
    VkDevice device;
    VkQueue queue;
    VkBuffer buffer;
};

/* This is odd so we start and end on the same queue */
#define NUM_HASH_ITERATIONS 513

#define LOCAL_WORKGROUP_SIZE 1024
#define GLOBAL_WORKGROUP_SIZE 512

#define TEST_BUFFER_SIZE (2 * 4 * LOCAL_WORKGROUP_SIZE)

static void
init_context(struct test_context *ctx, float priority)
{
    VkResult result = vkCreateDevice(t_physical_dev,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &(VkDeviceQueueCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = (float[]) { priority },
            },
        }, NULL, &ctx->device);
    t_assert(result == VK_SUCCESS);
    t_cleanup_push_vk_device(ctx->device, NULL);

    vkGetDeviceQueue(ctx->device, 0, 0, &ctx->queue);

    ctx->buffer = qoCreateBuffer(ctx->device, .size = TEST_BUFFER_SIZE,
        .pNext = &(VkExternalMemoryBufferCreateInfoKHX) {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHX,
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
        });
}

static void
cpu_process_data(uint32_t *data)
{
    for (unsigned k = 0; k < LOCAL_WORKGROUP_SIZE; k++) {
        uint32_t *x = &data[k * 2 + 0];
        uint32_t *y = &data[k * 2 + 1];
        for (unsigned i = 0; i < NUM_HASH_ITERATIONS; i++) {
            for (unsigned j = 0; j < GLOBAL_WORKGROUP_SIZE; j++) {
                if ((i & 1) == 0) {
                    *x = (*x ^ *y) * 0x01000193 + 0x0050230f;
                } else {
                    *y = (*y ^ *x) * 0x01000193 + 0x0071f80c;
                }
            }
        }
    }
}

static VkCommandBuffer
create_command_buffer(struct test_context *ctx, int parity)
{
    VkResult result;

    VkShaderModule cs;
    if (parity == 0) {
        cs = qoCreateShaderModuleGLSL(ctx->device, COMPUTE,
            layout(set = 0, binding = 0, std430) buffer Storage {
               ivec2 data[];
            } ssbo;

            layout (local_size_x = 1024) in;

            void main()
            {
                ivec2 data = ssbo.data[gl_LocalInvocationID.x];
                data.x = data.x ^ data.y;
                data.x = data.x * 0x01000193 + 0x0050230f;
                ssbo.data[gl_LocalInvocationID.x].x = data.x;
            }
        );
    } else {
        cs = qoCreateShaderModuleGLSL(ctx->device, COMPUTE,
            layout(set = 0, binding = 0, std430) buffer Storage {
               ivec2 data[];
            } ssbo;

            layout (local_size_x = 1024) in;

            void main()
            {
                ivec2 data = ssbo.data[gl_LocalInvocationID.x];
                data.y = data.y ^ data.x;
                data.y = data.y * 0x01000193 + 0x0071f80c;
                ssbo.data[gl_LocalInvocationID.x].y = data.y;
            }
        );
    }

    VkDescriptorSetLayout set_layout = qoCreateDescriptorSetLayout(ctx->device,
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

    VkPipelineLayout pipeline_layout = qoCreatePipelineLayout(ctx->device,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout);

    VkPipeline pipeline;
    result = vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1,
        &(VkComputePipelineCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = NULL,
            .stage = {
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = cs,
                .pName = "main",
            },
            .flags = 0,
            .layout = pipeline_layout
        }, NULL, &pipeline);
    t_assert(result == VK_SUCCESS);
    t_cleanup_push_vk_pipeline(ctx->device, pipeline);

    VkDescriptorPool descriptor_pool;
    result = vkCreateDescriptorPool(ctx->device,
        &(VkDescriptorPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &(VkDescriptorPoolSize) {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
            },
        }, NULL, &descriptor_pool);
    t_assert(result == VK_SUCCESS);
    t_cleanup_push_vk_descriptor_pool(ctx->device, descriptor_pool);

    VkDescriptorSet set = qoAllocateDescriptorSet(ctx->device,
        .descriptorPool = descriptor_pool,
        .pSetLayouts = &set_layout);

    vkUpdateDescriptorSets(ctx->device,
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
                    .buffer = ctx->buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE,
                },
            },
        }, 0, NULL);

    VkCommandPool cmd_pool;
    result = vkCreateCommandPool(ctx->device,
        &(VkCommandPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
        }, NULL, &cmd_pool);
    t_assert(result == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(ctx->device, cmd_pool);

    VkCommandBuffer cmd_buffer = qoAllocateCommandBuffer(ctx->device, cmd_pool);

    qoBeginCommandBuffer(cmd_buffer,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(cmd_buffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline_layout, 0, 1,
                            &set, 0, NULL);

    vkCmdPipelineBarrier(cmd_buffer,
                         VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, NULL,
                         1, &(VkBufferMemoryBarrier) {
                            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                            .srcAccessMask = 0,
                            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT |
                                             VK_ACCESS_SHADER_WRITE_BIT,
                            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHX,
                            .dstQueueFamilyIndex = 0,
                            .buffer = ctx->buffer,
                            .offset = 0,
                            .size = VK_WHOLE_SIZE,
                         },
                         0, NULL);

    for (unsigned j = 0; j < GLOBAL_WORKGROUP_SIZE; j++) {
        vkCmdDispatch(cmd_buffer, 1, 1, 1);

        vkCmdPipelineBarrier(cmd_buffer,
                             VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0, NULL,
                             1, &(VkBufferMemoryBarrier) {
                                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                .srcAccessMask = VK_ACCESS_SHADER_READ_BIT |
                                                 VK_ACCESS_SHADER_WRITE_BIT,
                                .dstAccessMask = VK_ACCESS_SHADER_READ_BIT |
                                                 VK_ACCESS_SHADER_WRITE_BIT,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHX,
                                .dstQueueFamilyIndex = 0,
                                .buffer = ctx->buffer,
                                .offset = 0,
                                .size = VK_WHOLE_SIZE,
                             },
                             0, NULL);
    }

    vkCmdPipelineBarrier(cmd_buffer,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, NULL,
                         1, &(VkBufferMemoryBarrier) {
                            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                            .srcAccessMask = VK_ACCESS_SHADER_READ_BIT |
                                             VK_ACCESS_SHADER_WRITE_BIT,
                            .dstAccessMask = 0,
                            .srcQueueFamilyIndex = 0,
                            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL_KHX,
                            .buffer = ctx->buffer,
                            .offset = 0,
                            .size = VK_WHOLE_SIZE,
                         },
                         0, NULL);

    qoEndCommandBuffer(cmd_buffer);

    return cmd_buffer;
}

static void
copy_memory(struct test_context *ctx,
            VkDeviceMemory dst, VkAccessFlags dst_access,
            VkDeviceMemory src, VkAccessFlags src_access, VkDeviceSize size)
{
    VkBuffer src_buf = qoCreateBuffer(ctx->device, .size = size);
    VkBuffer dst_buf = qoCreateBuffer(ctx->device, .size = size);

    qoBindBufferMemory(ctx->device, src_buf, src, 0);
    qoBindBufferMemory(ctx->device, dst_buf, dst, 0);

    VkCommandPool cmd_pool;
    VkResult result = vkCreateCommandPool(ctx->device,
        &(VkCommandPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
        }, NULL, &cmd_pool);
    t_assert(result == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(ctx->device, cmd_pool);

    VkCommandBuffer cmd_buffer = qoAllocateCommandBuffer(ctx->device, cmd_pool);

    qoBeginCommandBuffer(cmd_buffer);

    vkCmdPipelineBarrier(cmd_buffer,
                         0,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, NULL,
                         2, (VkBufferMemoryBarrier[]) {
                            {
                                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                .srcAccessMask = src_access,
                                .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .buffer = src_buf,
                                .offset = 0,
                                .size = VK_WHOLE_SIZE,
                            },
                            {
                                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                .srcAccessMask = 0,
                                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .buffer = dst_buf,
                                .offset = 0,
                                .size = VK_WHOLE_SIZE,
                            }
                         },
                         0, NULL);

    vkCmdCopyBuffer(cmd_buffer, src_buf, dst_buf, 1,
        &(VkBufferCopy) {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        });

    vkCmdPipelineBarrier(cmd_buffer,
                         0,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, NULL,
                         2, (VkBufferMemoryBarrier[]) {
                            {
                                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                                .dstAccessMask = 0,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .buffer = src_buf,
                                .offset = 0,
                                .size = VK_WHOLE_SIZE,
                            },
                            {
                                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                .dstAccessMask = dst_access,
                                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                .buffer = dst_buf,
                                .offset = 0,
                                .size = VK_WHOLE_SIZE,
                            }
                         },
                         0, NULL);

    qoEndCommandBuffer(cmd_buffer);

    qoQueueSubmit(ctx->queue, 1, &cmd_buffer, VK_NULL_HANDLE);
}

static void
init_memory_contents(struct test_context *ctx,
                     uint32_t *data, VkDeviceMemory memory)
{
    /* First, set up the CPU pointer */
    for (unsigned i = 0; i < LOCAL_WORKGROUP_SIZE; i++) {
        data[i * 2 + 0] = i * 37;
        data[i * 2 + 1] = 0;
    }

    VkDeviceMemory tmp_mem =
        qoAllocMemory(ctx->device,
                      .allocationSize = TEST_BUFFER_SIZE,
                      .memoryTypeIndex = 0 /* TODO */);

    void *tmp_map = qoMapMemory(ctx->device, tmp_mem, 0, TEST_BUFFER_SIZE, 0);
    memcpy(tmp_map, data, TEST_BUFFER_SIZE);
    vkFlushMappedMemoryRanges(ctx->device, 1,
        &(VkMappedMemoryRange) {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = tmp_mem,
            .offset = 0,
            .size = TEST_BUFFER_SIZE,
        });
    vkUnmapMemory(ctx->device, tmp_mem);

    copy_memory(ctx,
                memory, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                tmp_mem, VK_ACCESS_HOST_WRITE_BIT,
                TEST_BUFFER_SIZE);
}

static void
check_memory_contents(struct test_context *ctx,
                      uint32_t *data, VkDeviceMemory memory)
{
    /* First, do the computation on the CPU */
    cpu_process_data(data);

    VkDeviceMemory tmp_mem =
        qoAllocMemory(ctx->device,
                      .allocationSize = TEST_BUFFER_SIZE,
                      .memoryTypeIndex = 0 /* TODO */);

    copy_memory(ctx,
                tmp_mem, VK_ACCESS_HOST_READ_BIT,
                memory, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                TEST_BUFFER_SIZE);
    vkQueueWaitIdle(ctx->queue);

    void *tmp_map = qoMapMemory(ctx->device, tmp_mem, 0, TEST_BUFFER_SIZE, 0);
    vkInvalidateMappedMemoryRanges(ctx->device, 1,
        &(VkMappedMemoryRange) {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = tmp_mem,
            .offset = 0,
            .size = TEST_BUFFER_SIZE,
        });

    t_assert(memcmp(data, tmp_map, TEST_BUFFER_SIZE) == 0);

    vkUnmapMemory(ctx->device, tmp_mem);
}

/* A simplified form to test the test and make sure everything works as
 * intended in the single-device case.
 */
static void
test_sanity(void)
{
    struct test_context ctx;
    init_context(&ctx, 1.0);

    VkMemoryRequirements buffer_reqs =
        qoGetBufferMemoryRequirements(ctx.device, ctx.buffer);

    VkDeviceMemory mem =
        qoAllocMemoryFromRequirements(ctx.device, &buffer_reqs,
            .memoryTypeIndex = t_mem_type_index_for_mmap);

    qoBindBufferMemory(ctx.device, ctx.buffer, mem, 0);

    uint32_t cpu_data[TEST_BUFFER_SIZE / 4];
    init_memory_contents(&ctx, cpu_data, mem);

    VkCommandBuffer cmd_buffer1 = create_command_buffer(&ctx, 0);
    VkCommandBuffer cmd_buffer2 = create_command_buffer(&ctx, 1);

    for (unsigned i = 0; i < NUM_HASH_ITERATIONS; i++) {
        if ((i & 1) == 0) {
            qoQueueSubmit(ctx.queue, 1, &cmd_buffer1, VK_NULL_HANDLE);
        } else {
            qoQueueSubmit(ctx.queue, 1, &cmd_buffer2, VK_NULL_HANDLE);
        }
    }

    check_memory_contents(&ctx, cpu_data, mem);
}

test_define {
    .name = "func.sync.semaphore-fd.sanity",
    .start = test_sanity,
    .no_image = true,
};

static void
test_opaque_fd(void)
{
    t_require_ext("VK_KHX_external_memory");
    t_require_ext("VK_KHX_external_memory_fd");
    t_require_ext("VK_KHX_external_semaphore");
    t_require_ext("VK_KHX_external_semaphore_fd");

    struct test_context ctx1, ctx2;
    init_context(&ctx1, 1.0);
    init_context(&ctx2, 0.0);

#define GET_FUNCTION_PTR(name, device) \
    PFN_vk##name name = (PFN_vk##name)vkGetDeviceProcAddr(device, "vk"#name)

    GET_FUNCTION_PTR(GetMemoryFdKHX, ctx1.device);
    GET_FUNCTION_PTR(GetSemaphoreFdKHX, ctx1.device);
    GET_FUNCTION_PTR(ImportSemaphoreFdKHX, ctx2.device);

#undef GET_FUNCTION_PTR

    VkMemoryRequirements buffer_reqs =
        qoGetBufferMemoryRequirements(ctx1.device, ctx1.buffer);

    VkDeviceMemory mem1 =
        qoAllocMemoryFromRequirements(ctx1.device, &buffer_reqs,
            .memoryTypeIndex = t_mem_type_index_for_mmap,
            .pNext = &(VkExportMemoryAllocateInfoKHX) {
                .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHX,
                .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
            });

    int fd;
    VkResult result =
        GetMemoryFdKHX(ctx1.device, mem1,
                       VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
                       &fd);
    t_assert(result == VK_SUCCESS);
    t_assert(fd >= 0);

    VkDeviceMemory mem2 =
        qoAllocMemoryFromRequirements(ctx2.device, &buffer_reqs,
            .memoryTypeIndex = t_mem_type_index_for_mmap,
            .pNext = &(VkImportMemoryFdInfoKHX) {
                .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHX,
                .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
                .fd = fd,
            });

    qoBindBufferMemory(ctx1.device, ctx1.buffer, mem1, 0);
    qoBindBufferMemory(ctx2.device, ctx2.buffer, mem2, 0);

    uint32_t cpu_data[TEST_BUFFER_SIZE / 4];
    init_memory_contents(&ctx1, cpu_data, mem1);

    VkCommandBuffer cmd_buffer1 = create_command_buffer(&ctx1, 0);
    VkCommandBuffer cmd_buffer2 = create_command_buffer(&ctx2, 1);

    VkSemaphore *semaphores =
        malloc(sizeof(VkSemaphore) * (NUM_HASH_ITERATIONS - 1) * 2);
    for (unsigned i = 0; i < NUM_HASH_ITERATIONS - 1; i++) {
        VkSemaphore sem1;
        result = vkCreateSemaphore(ctx1.device,
            &(VkSemaphoreCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &(VkExportSemaphoreCreateInfoKHX) {
                .sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHX,
                .handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
            }}, NULL, &sem1);
        t_assert(result == VK_SUCCESS);
        t_cleanup_push_vk_semaphore(ctx1.device, sem1);

        int fd;
        result = GetSemaphoreFdKHX(ctx1.device, sem1,
            VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHX, &fd);
        t_assert(result == VK_SUCCESS);

        VkSemaphore sem2;
        result = vkCreateSemaphore(ctx2.device,
            &(VkSemaphoreCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            }, NULL, &sem2);
        t_assert(result == VK_SUCCESS);
        t_cleanup_push_vk_semaphore(ctx2.device, sem2);

        result = ImportSemaphoreFdKHX(ctx2.device,
            &(VkImportSemaphoreFdInfoKHX) {
                .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHX,
                .semaphore = sem2,
                .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
                .fd = fd,
            });
        t_assert(result == VK_SUCCESS);

        semaphores[i * 2 + 0] = sem1;
        semaphores[i * 2 + 1] = sem2;
    }

    logi("Begin queuing batches\n");

    /* NUM_HASH_ITERATIONS is odd, so we use ctx1 for both the first and
     * last submissions.  This makes keeping track of where the memory is a
     * bit easier.
     */
    for (unsigned i = 0; i < NUM_HASH_ITERATIONS; i++) {
        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
        };

        if ((i & 1) == 0) {
            if (i != 0) {
                submit.waitSemaphoreCount = 1;
                submit.pWaitSemaphores = &semaphores[(i - 1) * 2 + 0];
            }

            submit.pCommandBuffers = &cmd_buffer1;

            if (i != NUM_HASH_ITERATIONS - 1) {
                submit.signalSemaphoreCount = 1;
                submit.pSignalSemaphores = &semaphores[i * 2 + 0];
            }

            result = vkQueueSubmit(ctx1.queue, 1, &submit, VK_NULL_HANDLE);
            t_assert(result == VK_SUCCESS);
        } else {
            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &semaphores[(i - 1) * 2 + 1];

            submit.pCommandBuffers = &cmd_buffer2;

            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &semaphores[i * 2 + 1];

            result = vkQueueSubmit(ctx2.queue, 1, &submit, VK_NULL_HANDLE);
            t_assert(result == VK_SUCCESS);
        }
    }

    logi("All compute batches queued\n");

    free(semaphores);

    check_memory_contents(&ctx1, cpu_data, mem1);
}

test_define {
    .name = "func.sync.semaphore-fd.opaque-fd",
    .start = test_opaque_fd,
    .no_image = true,
};

static void
test_sync_fd(void)
{
    t_require_ext("VK_KHX_external_memory");
    t_require_ext("VK_KHX_external_memory_fd");
    t_require_ext("VK_KHX_external_semaphore");
    t_require_ext("VK_KHX_external_semaphore_fd");

    struct test_context ctx1, ctx2;
    init_context(&ctx1, 1.0);
    init_context(&ctx2, 0.0);

#define GET_FUNCTION_PTR(name, device) \
    PFN_vk##name name = (PFN_vk##name)vkGetDeviceProcAddr(device, "vk"#name)

    GET_FUNCTION_PTR(GetMemoryFdKHX, ctx1.device);
    GET_FUNCTION_PTR(GetSemaphoreFdKHX, ctx1.device);
    GET_FUNCTION_PTR(ImportSemaphoreFdKHX, ctx2.device);

#undef GET_FUNCTION_PTR

    VkMemoryRequirements buffer_reqs =
        qoGetBufferMemoryRequirements(ctx1.device, ctx1.buffer);

    VkDeviceMemory mem1 =
        qoAllocMemoryFromRequirements(ctx1.device, &buffer_reqs,
            .memoryTypeIndex = t_mem_type_index_for_mmap,
            .pNext = &(VkExportMemoryAllocateInfoKHX) {
                .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHX,
                .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
            });

    int fd;
    VkResult result =
        GetMemoryFdKHX(ctx1.device, mem1,
                       VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
                       &fd);
    t_assert(result == VK_SUCCESS);
    t_assert(fd >= 0);

    VkDeviceMemory mem2 =
        qoAllocMemoryFromRequirements(ctx2.device, &buffer_reqs,
            .memoryTypeIndex = t_mem_type_index_for_mmap,
            .pNext = &(VkImportMemoryFdInfoKHX) {
                .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHX,
                .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
                .fd = fd,
            });

    qoBindBufferMemory(ctx1.device, ctx1.buffer, mem1, 0);
    qoBindBufferMemory(ctx2.device, ctx2.buffer, mem2, 0);

    uint32_t cpu_data[TEST_BUFFER_SIZE / 4];
    init_memory_contents(&ctx1, cpu_data, mem1);

    VkCommandBuffer cmd_buffer1 = create_command_buffer(&ctx1, 0);
    VkCommandBuffer cmd_buffer2 = create_command_buffer(&ctx2, 1);

    logi("Begin queuing batches\n");

    /* NUM_HASH_ITERATIONS is odd, so we use ctx1 for both the first and
     * last submissions.  This makes keeping track of where the memory is a
     * bit easier.
     */
    int last_fence_fd = -1;
    for (unsigned i = 0; i < NUM_HASH_ITERATIONS; i++) {
        struct test_context *ctx;

        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
        };

        if ((i & 1) == 0) {
            ctx = &ctx1;
            submit.pCommandBuffers = &cmd_buffer1;
        } else {
            ctx = &ctx2;
            submit.pCommandBuffers = &cmd_buffer2;
        }

        VkSemaphore signal_sem, wait_sem;
        if (i != 0) {
            result = vkCreateSemaphore(ctx->device,
                &(VkSemaphoreCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                }, NULL, &wait_sem);
            t_assert(result == VK_SUCCESS);
            t_cleanup_push_vk_semaphore(ctx->device, wait_sem);

            result = ImportSemaphoreFdKHX(ctx->device,
                &(VkImportSemaphoreFdInfoKHX) {
                    .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHX,
                    .semaphore = wait_sem,
                    .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_FENCE_FD_BIT_KHX,
                    .fd = last_fence_fd,
                });
            t_assert(result == VK_SUCCESS);

            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &wait_sem;
        }

        if (i != NUM_HASH_ITERATIONS - 1) {
            result = vkCreateSemaphore(ctx->device,
                &(VkSemaphoreCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = &(VkExportSemaphoreCreateInfoKHX) {
                    .sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHX,
                    .handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_FENCE_FD_BIT_KHX,
                }}, NULL, &signal_sem);
            t_assert(result == VK_SUCCESS);
            t_cleanup_push_vk_semaphore(ctx->device, signal_sem);

            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &signal_sem;
        }

        result = vkQueueSubmit(ctx->queue, 1, &submit, VK_NULL_HANDLE);
        t_assert(result == VK_SUCCESS);

        if (i != NUM_HASH_ITERATIONS - 1) {
            result = GetSemaphoreFdKHX(ctx->device, signal_sem,
                VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHX,
                &last_fence_fd);
            t_assert(result == VK_SUCCESS);
            t_assert(last_fence_fd >= 0);
        }
    }

    logi("All compute batches queued\n");

    check_memory_contents(&ctx1, cpu_data, mem1);
}

test_define {
    .name = "func.sync.semaphore-fd.sync-fd",
    .start = test_sync_fd,
    .no_image = true,
};
