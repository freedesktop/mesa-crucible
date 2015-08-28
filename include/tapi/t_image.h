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

/// \file
/// \brief Test API: Helpers for Crucible images

#pragma once

#include "util/macros.h"
#include "util/vk_wrapper.h"

typedef struct cru_image cru_image_t;

/// \brief Create a Crucible image from a file.
///
/// This is a wrapper around cru_image_from_filename(). On success, the new
/// image is pushed onto the test thread's cleanup stack.  On failure, the test
/// fails.
///
/// \see cru_image_from_filename()
///
malloclike cru_image_t *
t_new_cru_image_from_filename(const char *filename);

/// \brief Create a Crucible image from a Vulkan image.
///
/// This is a wrapper around cru_image_from_vk_image(). On success, the new
/// image is pushed onto the test thread's cleanup stack.  On failure, the test
/// fails.
///
/// \see cru_image_from_vk_image()
///
malloclike cru_image_t *
t_new_cru_image_from_vk_image(VkDevice dev, VkQueue queue, VkImage image,
                              VkFormat format, VkImageAspect aspect,
                              uint32_t level0_width, uint32_t level0_height,
                              uint32_t miplevel, uint32_t array_slice);
