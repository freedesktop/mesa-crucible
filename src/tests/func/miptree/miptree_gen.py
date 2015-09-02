#!/usr/bin/env python3

# Copyright 2015 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

from textwrap import dedent
from collections import namedtuple

Extent2D = namedtuple('Extent2D', ('width', 'height'))
Format = namedtuple('Format', ('short_name', 'vk_name'))
Params = namedtuple('Params', ('format', 'aspect', 'view', 'extent',
                    'levels', 'array_length', 'upload', 'download'))

color_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method)
    for format in (Format('r8g8b8a8-unorm', 'VK_FORMAT_R8G8B8A8_UNORM'),)
    for aspect in ('color',)
    for view in ('2d',)
    for extent in (Extent2D(512, 512),)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',
        'copy-with-draw',
    )
    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',
        'copy-with-draw',
    )
)

depth_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method)
    for format in (Format('d32-sfloat', 'VK_FORMAT_D32_SFLOAT'),)
    for aspect in ('depth',)
    for view in ('2d',)
    for extent in (Extent2D(1024, 512),)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',
    )
    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',
    )
)

stencil_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method)
    for format in (Format('s8-uint', 'VK_FORMAT_S8_UINT'),)
    for aspect in ('stencil',)
    for view in ('2d',)
    for extent in (Extent2D(1024, 512),)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',

        # Intel doesn't support linear stencil images. If you want to enable
        # testing linear stencil images, then you must first teach the test
        # how to skip when the driver does not support the particular
        # format/image/usage combination.
        #
        #'copy-from-linear-image',
    )
    for download_method in (
        'copy-to-buffer',

        # Intel doesn't support linear stencil images. See above comment.
        #'copy-to-linear-image',
    )
)

def all_params_iter():
    for p in color_params_iter:
        yield p
    for p in depth_params_iter:
        yield p
    for p in stencil_params_iter:
        yield p

template = dedent("""
    test_define {{
        .name = "func.miptree"
                ".{format[0]}"
                ".aspect-{aspect}"
                ".extent-{extent.width}x{extent.height}"
                ".view-{view}.levels{levels:02}.array{array_length:02}"
                ".upload-{upload}.download-{download}",
        .start = test,
        .skip = {skip},
        .no_image = true,
        .user_data = &(test_params_t) {{
            .format = {format[1]},
            .aspect = VK_IMAGE_ASPECT_{aspect_caps},
            .view_type = VK_IMAGE_VIEW_TYPE_{view_caps},
            .levels = {levels},
            .width = {extent.width},
            .height = {extent.height},
            .array_length = {array_length},
            .upload_method = MIPTREE_UPLOAD_METHOD_{upload_caps},
            .download_method = MIPTREE_DOWNLOAD_METHOD_{download_caps},
        }},
    }};
    """)

copyright = dedent("""\
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
    """)

def to_caps(s):
    return s.upper().replace('-', '_')

def main():
    out_filename = __file__.replace('.py', '.c')

    with open(out_filename, 'w') as out_file:
        out_file.write(copyright)

        for p in all_params_iter():
            test_def = template.format(
                format = p.format,
                aspect = p.aspect,
                view = p.view,
                extent = p.extent,
                levels = p.levels,
                array_length = p.array_length,
                upload = p.upload,
                download = p.download,
                aspect_caps = to_caps(p.aspect),
                view_caps = to_caps(p.view),
                upload_caps = to_caps(p.upload),
                download_caps = to_caps(p.download),
                skip = 'false')
            out_file.write(test_def)

main()
