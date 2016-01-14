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

#include <poll.h>
#include <stdio.h>
#include "tapi/t.h"

static VkEvent
create_event(void)
{
    VkResult result;
    VkEvent event;

    static const VkEventCreateInfo create_info = {
	    .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
	    .pNext = NULL,
	    .flags = 0
    };

    result = vkCreateEvent(t_device,
			   &create_info,
			   NULL,
			   &event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_RESET);

    return event;
}

static void
test_event_basic(void)
{
    VkResult result;
    VkEvent event = create_event();

    result = vkSetEvent(t_device, event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_SET);

    // Set again to make sure nothing funny happens when we do that.
    result = vkSetEvent(t_device, event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_SET);

    result = vkResetEvent(t_device, event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_RESET);

    // Reset again to make sure nothing funny happens when we do that.
    result = vkResetEvent(t_device, event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_RESET);

    // Set again after reset to make sure that works
    result = vkSetEvent(t_device, event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_SET);

    vkDestroyEvent(t_device, event, NULL);
}

test_define {
    .name = "func.event.basic",
    .start = test_event_basic,
    .no_image = true,
};

static inline VkCommandBuffer
__alloc_and_begin(void)
{
	VkCommandBuffer cmdBuffer =
		qoAllocateCommandBuffer(t_device, t_cmd_pool);

	qoBeginCommandBuffer(cmdBuffer);

	return cmdBuffer;
}

static inline bool
__create_and_wait(VkCommandBuffer *cmdBuffer)
{
	if (*cmdBuffer == NULL) {
		*cmdBuffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
		qoBeginCommandBuffer(*cmdBuffer);

		return true;
	} else {
		qoEndCommandBuffer(*cmdBuffer);

		qoQueueSubmit(t_queue, 1, cmdBuffer, VK_NULL_HANDLE);

		vkQueueWaitIdle(t_queue);

		return false;
	}
}

#define cmd_buffer_do(__cmd_buffer)					\
	for (VkCommandBuffer __cmd_buffer = NULL;			\
	     __create_and_wait(&__cmd_buffer); )

static void
test_event_cmd_buffer(void)
{
    VkEvent event = create_event();
    VkResult result;

    cmd_buffer_do(cmd_buffer) {
	    vkCmdSetEvent(cmd_buffer, event,
			  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    }

    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_SET);

    cmd_buffer_do(cmd_buffer) {
	    vkCmdResetEvent(cmd_buffer, event,
			    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    }
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_RESET);

    result = vkSetEvent(t_device, event);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_SET);

    cmd_buffer_do(cmd_buffer) {
	    vkCmdResetEvent(cmd_buffer, event,
			    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    }
    t_assert(vkGetEventStatus(t_device, event) == VK_EVENT_RESET);

    vkDestroyEvent(t_device, event, NULL);
}

test_define {
    .name = "func.event.cmd_buffer",
    .start = test_event_cmd_buffer,
    .no_image = true,
};

static void
test_event_wait_preset(void)
{
    VkEvent event_a = create_event();
    VkEvent event_b = create_event();
    VkResult result;

    result = vkSetEvent(t_device, event_a);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event_a) == VK_EVENT_SET);
    t_assert(vkGetEventStatus(t_device, event_b) == VK_EVENT_RESET);

    cmd_buffer_do(cmd_buffer) {
	    vkCmdWaitEvents(cmd_buffer, 1, &event_a,
			    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			    0, NULL, 0, NULL, 0, NULL);
	    vkCmdSetEvent(cmd_buffer, event_b,
			  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    }

    t_assert(vkGetEventStatus(t_device, event_a) == VK_EVENT_SET);
    t_assert(vkGetEventStatus(t_device, event_b) == VK_EVENT_SET);
}

test_define {
    .name = "func.event.wait_preset",
    .start = test_event_wait_preset,
    .no_image = true,
};

static void
test_event_wait_set_later(void)
{
    VkEvent event_a = create_event();
    VkEvent event_b = create_event();
    VkResult result;

    t_assert(vkGetEventStatus(t_device, event_a) == VK_EVENT_RESET);
    t_assert(vkGetEventStatus(t_device, event_b) == VK_EVENT_RESET);

    VkCommandBuffer cmd_buffer = qoAllocateCommandBuffer(t_device, t_cmd_pool);
    qoBeginCommandBuffer(cmd_buffer);
    vkCmdWaitEvents(cmd_buffer, 1, &event_a,
		    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		    VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		    0, NULL, 0, NULL, 0, NULL);
    vkCmdSetEvent(cmd_buffer, event_b,
		  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    qoEndCommandBuffer(cmd_buffer);

    qoQueueSubmit(t_queue, 1, &cmd_buffer, VK_NULL_HANDLE);

    poll(NULL, 0, 100);
    t_assert(vkGetEventStatus(t_device, event_a) == VK_EVENT_RESET);
    t_assert(vkGetEventStatus(t_device, event_b) == VK_EVENT_RESET);

    result = vkSetEvent(t_device, event_a);
    t_assert(result == VK_SUCCESS);
    t_assert(vkGetEventStatus(t_device, event_a) == VK_EVENT_SET);

    vkQueueWaitIdle(t_queue);

    t_assert(vkGetEventStatus(t_device, event_a) == VK_EVENT_SET);
    t_assert(vkGetEventStatus(t_device, event_b) == VK_EVENT_SET);
}

test_define {
    .name = "func.event.wait_set_later",
    .start = test_event_wait_set_later,
    .no_image = true,
};
