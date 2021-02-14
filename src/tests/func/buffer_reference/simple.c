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

#include "util/simple_pipeline.h"
#include "tapi/t.h"

#include "src/tests/func/buffer_reference/simple-spirv.h"

#define GET_DEVICE_FUNCTION_PTR(name) \
    PFN_##name name = (PFN_##name)vkGetDeviceProcAddr(t_device, #name)


static void
simple(void)
{
    t_require_ext("VK_KHR_buffer_device_address");

    GET_DEVICE_FUNCTION_PTR(vkGetBufferDeviceAddressKHR);

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_EXT_buffer_reference : require

        layout(local_size_x = 1) in;

        layout(buffer_reference) buffer ValueRef {
            uint val;
        };

        layout(push_constant, std430) uniform Push {
            ValueRef value_ref;
        };

        void main() {
            value_ref.val = 42;
        }
    );

    unsigned buffer_size = sizeof(unsigned);
    VkBuffer buffer = qoCreateBuffer(t_device,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
        .size = buffer_size);
    VkDeviceMemory memory = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *map = qoMapMemory(t_device, memory, 0, buffer_size, 0);
    memset(map, 0, buffer_size);

    qoBindBufferMemory(t_device, buffer, memory, 0);

    VkBufferDeviceAddressInfoKHR addr_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
        .buffer = buffer,
    };

    VkDeviceAddress addr = vkGetBufferDeviceAddressKHR(t_device, &addr_info);

    simple_compute_pipeline_options_t opts = {
        .push_constants = &addr,
        .push_constants_size = sizeof(addr),
    };
    run_simple_compute_pipeline(cs, &opts);

    unsigned val = *((unsigned *)map);
    t_assert(val == 42);

    t_pass();
}

test_define {
    .name = "func.buffer_reference.simple",
    .start = simple,
    .no_image = true,
};


static void
simple_ext(void)
{
    t_require_ext("VK_EXT_buffer_device_address");

    GET_DEVICE_FUNCTION_PTR(vkGetBufferDeviceAddressEXT);

    VkShaderModule cs = qoCreateShaderModuleGLSL(t_device, COMPUTE,
        QO_EXTENSION GL_EXT_buffer_reference : require

        layout(local_size_x = 1) in;

        layout(buffer_reference) buffer ValueRef {
            uint val;
        };

        layout(push_constant, std430) uniform Push {
            ValueRef value_ref;
        };

        void main() {
            value_ref.val = 42;
        }
    );

    unsigned buffer_size = sizeof(unsigned);
    VkBuffer buffer = qoCreateBuffer(t_device,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT,
        .size = buffer_size);
    VkDeviceMemory memory = qoAllocBufferMemory(t_device, buffer,
        .properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *map = qoMapMemory(t_device, memory, 0, buffer_size, 0);
    memset(map, 0, buffer_size);

    qoBindBufferMemory(t_device, buffer, memory, 0);

    VkBufferDeviceAddressInfoEXT addr_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT,
        .buffer = buffer,
    };

    VkDeviceAddress addr = vkGetBufferDeviceAddressEXT(t_device, &addr_info);

    simple_compute_pipeline_options_t opts = {
        .push_constants = &addr,
        .push_constants_size = sizeof(addr),
    };
    run_simple_compute_pipeline(cs, &opts);

    unsigned val = *((unsigned *)map);
    t_assert(val == 42);

    t_pass();
}

test_define {
    .name = "func.buffer_reference.simple_ext",
    .start = simple_ext,
    .no_image = true,
};
