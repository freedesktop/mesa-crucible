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

#include <stdio.h>

#include "tapi/t.h"

// Use VK_EXT_memory_budget to check the memory usage before and after
// an allocation

#define MB (1024 * 1024)
#define BUFFER_SIZE (64 * MB)

static void
get_memory_budget(unsigned i, VkDeviceSize *usage, VkDeviceSize *budget)
{
    VkPhysicalDeviceMemoryBudgetPropertiesEXT budget_prop = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT,
    };
    VkPhysicalDeviceMemoryProperties2 props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
        .pNext = &budget_prop,
    };
    vkGetPhysicalDeviceMemoryProperties2(t_physical_dev, &props);

    *usage = budget_prop.heapUsage[i];
    *budget = budget_prop.heapBudget[i];
}

static void
print_memory_budget(unsigned heap_index, const char *message,
                    VkDeviceSize usage, VkDeviceSize budget)
{
    VkDeviceSize size = t_physical_dev_mem_props->memoryHeaps[heap_index].size;
    logi("[heap %d] size = %5lu MB, usage = %3lu MB, budget = %5lu MB (%s)",
         heap_index, size / MB, usage / MB, budget / MB, message);
}

static void
test_memory_budget(void)
{
    t_require_ext("VK_EXT_memory_budget");

    for (uint32_t type_index = 0;
         type_index < t_physical_dev_mem_props->memoryTypeCount;
         type_index++)
    {
        uint32_t heap_index = t_physical_dev_mem_props->memoryTypes[type_index].heapIndex;

        VkDeviceSize usage_before, budget_before;
        get_memory_budget(heap_index, &usage_before, &budget_before);
        print_memory_budget(heap_index, "at the start", usage_before, budget_before);

        VkBuffer buffer = qoCreateBuffer(t_device, .size = BUFFER_SIZE);
        VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
                                                 .memoryTypeIndex = type_index);

        uint8_t *map = qoMapMemory(t_device, mem, /*offset*/ 0,
                                   BUFFER_SIZE, /*flags*/ 0);

        /* Write something so that the memory gets actually allocated */
        memset(map, 0xff, BUFFER_SIZE);

        VkDeviceSize usage_after, budget_after;
        get_memory_budget(heap_index, &usage_after, &budget_after);
        print_memory_budget(heap_index, "after allocating 64MB", usage_after, budget_after);

        if (usage_after < usage_before + BUFFER_SIZE)
            t_failf("application's heap usage must have grown by at least the buffer size");

        if (usage_before > budget_before)
            t_failf("heap usage before is larger than the heap budget before");

        if (usage_after > budget_after)
            t_failf("heap usage after is larger than the heap budget after");

        if (budget_before > t_physical_dev_mem_props->memoryHeaps[heap_index].size)
            t_failf("heap budget before is larger than the heap size");

        if (budget_after > t_physical_dev_mem_props->memoryHeaps[heap_index].size)
            t_failf("heap budget after is larger than the heap size");
    }

    t_end(TEST_RESULT_PASS);
}

test_define {
    .name = "func.memory_budget",
    .start = test_memory_budget,
    .no_image = true,
};
