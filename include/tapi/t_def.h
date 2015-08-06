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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util/macros.h"
#include "util/vk_wrapper.h"

typedef struct test_def test_def_t;

/// \brief A test definition.
///
/// All public members are const. This prevents a test from modifying its
/// definition while running.
struct test_def {
    /// The test name must be a valid filename with no path separator.
    const char *const name;

    /// Reserved for the test author. The test framework never touches this
    /// data.
    const void *const user_data;

    /// \brief Filename of the test's reference image.
    ///
    /// The filename is relative to Crucible's "img" directory. If unset, then
    /// the default filename "{test_name}.ref.png" is used.
    const char *const image_filename;

    void (*const start)(void);
    const uint32_t samples;
    const bool no_image;

    /// \brief Create a default depthstencil attachment.
    ///
    /// If and only if depthstencil_format is set, then the test's default
    /// framebuffer (t_framebuffer) will have a depthstencil attachment; and
    /// t_ds_image, t_ds_attachment_view, and t_depth_view will be non-null.
    const VkFormat depthstencil_format;

    /// \brief Skip this test.
    ///
    /// This is useful for work-in-progress tests.
    const bool skip;

    /// \brief Private data for the test framework.
    ///
    /// Test authors shouldn't touch this struct.
    ///
    /// The test runner walks twice over the global list of test definitions.
    /// In the first pass, it enables each test that it plans to run.  In the
    /// second pass, it runs the enabled tests.
    struct test_def_priv {
        bool enable;
    } priv;
} __attribute__((aligned(32)));

/// Example usage:
///
///    static void
///    draw_a_triangle(void)
///    {
///       ...
///    }
///
///    test_define {
///       .name = "draw-a-triangle",
///       .start = draw_a_triangle,
///    }
///
#define test_define                                                         \
                                                                            \
    static test_def_t                                                       \
    CRU_CAT(__test_def_, __COUNTER__)                                       \
        __attribute__((__section__("cru_test_defs")))                       \
        __attribute__((__used__)) =
