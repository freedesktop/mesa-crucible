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

/// \file
/// \brief The runner's vulkan helper routines

#include "runner_vk.h"

#include <stdlib.h>

#include "util/vk_wrapper.h"

bool
runner_get_vulkan_queue_count(uint32_t *count)
{
    if (count == NULL)
        return false;

    VkResult res;
    uint32_t instance_extension_count;
    res = vkEnumerateInstanceExtensionProperties(NULL,
        &instance_extension_count, NULL);
    if (res != VK_SUCCESS)
        return false;

    VkExtensionProperties *instance_extension_props;
    instance_extension_props =
        malloc(instance_extension_count * sizeof(*instance_extension_props));
    if (instance_extension_props == NULL)
        return false;

    res = vkEnumerateInstanceExtensionProperties(NULL,
        &instance_extension_count, instance_extension_props);
    if (res != VK_SUCCESS) {
        free(instance_extension_props);
        return false;
    }

    const char **ext_names;
    ext_names = malloc(instance_extension_count * sizeof(*ext_names));
    if (ext_names == NULL) {
        free(instance_extension_props);
        return false;
    }

    for (uint32_t i = 0; i < instance_extension_count; i++) {
        ext_names[i] = instance_extension_props[i].extensionName;
    }

    VkInstance instance;
    res = vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = NULL,
            .pApplicationInfo = &(VkApplicationInfo) {
                .pApplicationName = "crucible",
                .apiVersion = VK_MAKE_VERSION(1, 0, 0),
            },
            .enabledExtensionCount = instance_extension_count,
            .ppEnabledExtensionNames = ext_names,
        }, NULL, &instance);
    free(instance_extension_props);
    free(ext_names);
    if (res != VK_SUCCESS)
        return false;

    uint32_t phy_dev_count = 0;
    res = vkEnumeratePhysicalDevices(instance, &phy_dev_count, NULL);
    if (res != VK_SUCCESS || phy_dev_count == 0)
        return false;

    VkPhysicalDevice phy_dev;
    phy_dev_count = 1;
    res = vkEnumeratePhysicalDevices(instance, &phy_dev_count, &phy_dev);
    if ((res != VK_SUCCESS && res != VK_INCOMPLETE) || phy_dev_count != 1)
        return false;

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(phy_dev,
                                             &queue_family_count, NULL);
    *count = queue_family_count;

    return true;
}
