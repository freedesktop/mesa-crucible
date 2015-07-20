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

#include <crucible/cru_format.h>

#define FMT(__vk_token) .format = __vk_token, .name = #__vk_token

static const struct cru_format_info
cru_format_info_table[] = {
    {
        FMT(VK_FORMAT_R8G8B8A8_UNORM),
        .num_type = CRU_NUM_TYPE_UNORM,
        .num_channels = 4,
        .cpp = 4,
        .is_color = true,
    },
    {
        FMT(VK_FORMAT_D32_SFLOAT),
        .num_type = CRU_NUM_TYPE_SFLOAT,
        .num_channels = 1,
        .cpp = 4,
        .depth_format = VK_FORMAT_D32_SFLOAT,
    },
    {
        FMT(VK_FORMAT_UNDEFINED),
    },
};

#undef FMT

const struct cru_format_info *
cru_format_get_info(VkFormat format)
{
    const struct cru_format_info *info;

    for (info = cru_format_info_table;
         info->format != VK_FORMAT_UNDEFINED; ++info) {
        if (info->format == format) {
            return info;
        }
    }

    return NULL;
}
