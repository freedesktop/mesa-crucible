/*
 * Copyright © 2020 Robin Heinemann <robin.ole.heinemann@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "tapi/t.h"
#include <unistd.h>
#include <sys/mman.h>

#define GET_DEVICE_FUNCTION_PTR(name) \
    PFN_vk##name name = (PFN_vk##name)vkGetDeviceProcAddr(t_device, "vk"#name)

static void
test_funcs(void)
{
    t_require_ext("VK_KHR_external_memory_fd");

    GET_DEVICE_FUNCTION_PTR(GetMemoryFdKHR);

    t_assert(GetMemoryFdKHR != NULL);
}

test_define {
    .name = "func.memory-fd.funcs",
    .start = test_funcs,
    .no_image = true,
};

static void
test_read(int handle_type)
{
    t_require_ext("VK_KHR_external_memory_fd");

    GET_DEVICE_FUNCTION_PTR(GetMemoryFdKHR);

    t_assert(GetMemoryFdKHR != NULL);

    size_t size = 1024;

    VkMemoryRequirements reqs = {
        .memoryTypeBits = (1 << t_physical_dev_mem_props->memoryTypeCount) - 1,
        .size = size
    };

    VkDeviceMemory mem = qoAllocMemoryFromRequirements(t_device,
        &reqs,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .pNext = &(VkExportMemoryAllocateInfo) {
            .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
            .handleTypes = handle_type
        });

    void *map;
    vkMapMemory(t_device, mem, 0, size, 0, &map);

    uint32_t *map32 = map;
    for (unsigned i = 0; i < size / sizeof(*map32); i++)
        map32[i] = i;

    vkUnmapMemory(t_device, mem);

    int fd;
    VkResult result = GetMemoryFdKHR(t_device, &(VkMemoryGetFdInfoKHR) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
            .memory = mem,
            .handleType = handle_type,
        }, &fd);

    t_assert(result == VK_SUCCESS);

    void *fd_map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    t_assert(fd_map != MAP_FAILED);

    uint32_t *fd_map32 = fd_map;
    for (unsigned i = 0; i < size / sizeof(*fd_map32); i++)
        t_assertf(fd_map32[i] == i,
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, fd_map32[i], i);

    munmap(fd_map, size);
}

static void
test_read_opaque(void)
{
    test_read(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
}

test_define {
    .name = "func.memory-fd.opaque.read",
    .start = test_read_opaque,
    .no_image = true,
};

static void
test_read_dma_buf(void)
{
    t_require_ext("VK_EXT_external_memory_dma_buf");
    test_read(VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
}

test_define {
    .name = "func.memory-fd.dma-buf.read",
    .start = test_read_dma_buf,
    .no_image = true,
};

static void
test_write(int handle_type)
{
    t_require_ext("VK_KHR_external_memory_fd");

    GET_DEVICE_FUNCTION_PTR(GetMemoryFdKHR);

    t_assert(GetMemoryFdKHR != NULL);

    size_t size = 1024;

    VkMemoryRequirements reqs = {
        .memoryTypeBits = (1 << t_physical_dev_mem_props->memoryTypeCount) - 1,
        .size = size
    };

    VkDeviceMemory mem = qoAllocMemoryFromRequirements(t_device,
        &reqs,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .pNext = &(VkExportMemoryAllocateInfo) {
            .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
            .handleTypes = handle_type
        });

    int fd;
    VkResult result = GetMemoryFdKHR(t_device, &(VkMemoryGetFdInfoKHR) {
            .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
            .memory = mem,
            .handleType = handle_type,
        }, &fd);
    t_assert(result == VK_SUCCESS);

    void *fd_map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    t_assert(fd_map != MAP_FAILED);

    uint32_t *fd_map32 = fd_map;
    for (unsigned i = 0; i < size / sizeof(*fd_map32); i++)
        fd_map32[i] = i;

    munmap(fd_map, size);
    close(fd);

    void *map;
    vkMapMemory(t_device, mem, 0, size, 0, &map);

    uint32_t *map32 = map;
    for (unsigned i = 0; i < size / sizeof(*map32); i++)
        t_assertf(map32[i] == i,
                  "buffer mismatch at dword %d: found 0x%x, "
                  "expected 0x%x", i, map32[i], i);

    vkUnmapMemory(t_device, mem);
}

static void
test_write_opaque(void)
{
    test_write(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
}

test_define {
    .name = "func.memory-fd.opaque.write",
    .start = test_write_opaque,
    .no_image = true,
};

static void
test_write_dma_buf(void)
{
    t_require_ext("VK_EXT_external_memory_dma_buf");
    test_write(VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
}

test_define {
    .name = "func.memory-fd.dma-buf.write",
    .start = test_write_dma_buf,
    .no_image = true,
};
