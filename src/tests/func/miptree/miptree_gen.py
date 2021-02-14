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

Extent3D = namedtuple('Extent3D', ('width', 'height', 'depth'))
Format = namedtuple('Format', ('short_name', 'vk_name'))
Params = namedtuple('Params', ('format', 'aspect', 'view', 'extent',
                    'levels', 'array_length', 'upload', 'download', 'intermediate'))

color_1d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('r8g8b8a8-unorm', 'VK_FORMAT_R8G8B8A8_UNORM'),)
    for aspect in ('color',)
    for view in ('1d',)
    for extent in (Extent3D(512, 1, 1), Extent3D(16384, 1, 1),)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',

        # Illegal to attach 1D image to a framebuffer.
        #'copy-with-draw',
    )
    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',

        # FINISHME: The test cannot yet texture from 1D images.
        #'copy-with-draw',
    )

    for intermediate_method in (
        'none',
        'copy-image'
    )
)

color_2d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('r8g8b8a8-unorm', 'VK_FORMAT_R8G8B8A8_UNORM'),)
    for aspect in ('color',)
    for view in ('2d',)
    for extent in (Extent3D(512, 512, 1), Extent3D(16384, 32, 1), Extent3D(32, 16384, 1),)
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
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

color_2d_bc3_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('bc3-unorm', 'VK_FORMAT_BC3_UNORM_BLOCK'),)
    for aspect in ('color',)
    for view in ('2d',)
    for extent in (Extent3D(512, 512, 1),)
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
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

color_3d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('r8g8b8a8-unorm', 'VK_FORMAT_R8G8B8A8_UNORM'),)
    for aspect in ('color',)
    for view in ('3d',)
    for extent in (Extent3D(32, 32, 32),)
    for levels in (1, 2)
    for array_length in (1,)

    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',

        # Illegal to attach 3D image to a framebuffer.
        #'copy-with-draw',
    )

    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',

        # FINISHME: The test cannot yet texture from 3D images.
        #'copy-with-draw',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

depth_1d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('d32-sfloat', 'VK_FORMAT_D32_SFLOAT'),)
    for aspect in ('depth',)
    for view in ('1d',)
    for extent in (Extent3D(1024, 1, 1),)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',

        # Illegal to attach 1D image to a framebuffer.
        #'copy-with-draw',
    )
    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',

        # FINISHME: The test cannot yet texture from 1D images.
        #'copy-with-draw',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

depth_2d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('d32-sfloat', 'VK_FORMAT_D32_SFLOAT'),)
    for aspect in ('depth',)
    for view in ('2d',)
    for extent in (Extent3D(1024, 512, 1),)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',
    )
    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',
        'copy-with-draw',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

depth_3d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('d32-sfloat', 'VK_FORMAT_D32_SFLOAT'),)
    for aspect in ('depth',)
    for view in ('3d',)
    for extent in (Extent3D(1024, 512, 32),)
    for levels in (1, 2)
    for array_length in (1,)

    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',

        # Illegal to attach 3D image to a framebuffer.
        #'copy-with-draw',
    )

    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',

        # FINISHME: The test cannot yet texture from 3D images.
        #'copy-with-draw',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

stencil_1d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('s8-uint', 'VK_FORMAT_S8_UINT'),)
    for aspect in ('stencil',)
    for view in ('1d',)
    for extent in (Extent3D(1024, 1, 1),)
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

        # FINISHME: The test cannot yet texture from 1D images.
        #'copy-with-draw',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

stencil_2d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('s8-uint', 'VK_FORMAT_S8_UINT'),)
    for aspect in ('stencil',)
    for view in ('2d',)
    for extent in (Extent3D(1024, 512, 1),)
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
        'copy-with-draw',

        # Intel doesn't support linear stencil images. See above comment.
        #'copy-to-linear-image',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

stencil_3d_params_iter = (
    Params(format, aspect, view, extent, levels, array_length, upload_method, download_method, intermediate_method)
    for format in (Format('s8-uint', 'VK_FORMAT_S8_UINT'),)
    for aspect in ('stencil',)
    for view in ('3d',)
    for extent in (Extent3D(1024, 512, 32),)
    for levels in (1, 2)
    for array_length in (1,)
    for upload_method in (
        'copy-from-buffer',

        # Intel doesn't support linear stencil images. See above comment.
        #'copy-from-linear-image',
    )
    for download_method in (
        'copy-to-buffer',

        # Intel doesn't support linear stencil images. See above comment.
        #'copy-to-linear-image',

        # FINISHME: The test cannot yet texture from 3D stencil images.
        #'copy-with-draw',
    )
    for intermediate_method in (
        'none',
        'copy-image'
    )
)

def all_params_iter():
    for p in color_1d_params_iter:
        yield p
    for p in color_2d_params_iter:
        yield p
    for p in color_2d_bc3_params_iter:
        yield p
    for p in color_3d_params_iter:
        yield p
    for p in depth_1d_params_iter:
        yield p
    for p in depth_2d_params_iter:
        yield p
    for p in depth_3d_params_iter:
        yield p
    for p in stencil_1d_params_iter:
        yield p
    for p in stencil_2d_params_iter:
        yield p
    for p in stencil_3d_params_iter:
        yield p

template = dedent("""
    test_define {{
        .name = "func.miptree"
                ".{format[0]}"
                ".aspect-{aspect}"
                ".view-{view}"
                ".levels{levels:02}"
                "{array_length_str}"
                ".extent-{extent_str}"
                ".upload-{upload}.download-{download}.intermediate-{intermediate}",
        .start = test,
        .skip = {skip},
        .no_image = true,
        .user_data = &(test_params_t) {{
            .format = {format[1]},
            .aspect = VK_IMAGE_ASPECT_{aspect_caps}_BIT,
            .view_type = VK_IMAGE_VIEW_TYPE_{view_caps},
            .levels = {levels},
            .width = {extent.width},
            .height = {extent.height},
            .depth = {extent.depth},
            .array_length = {array_length},
            .upload_method = MIPTREE_UPLOAD_METHOD_{upload_caps},
            .download_method = MIPTREE_DOWNLOAD_METHOD_{download_caps},
            .intermediate_method = MIPTREE_INTERMEDIATE_METHOD_{intermediate_caps},
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

def get_extent_str(params):
    e = params.extent

    if params.view == '1d':
        return '{}'.format(e.width)
    elif params.view == '2d':
        return '{}x{}'.format(e.width, e.height)
    elif params.view == '3d':
        return '{}x{}x{}'.format(e.width, e.height, e.depth)
    else:
        raise Exception('unhandled view in get_extent_str')

def get_array_length_str(params):
    if params.view in ('1d', '2d'):
        return '.array{:02}'.format(params.array_length)
    elif params.view == '3d':
        return ''
    else:
        raise Exception('unhandled view in get_array_length_str')

def main():
    import sys
    out_filename = sys.argv[1]

    with open(out_filename, 'w') as out_file:
        out_file.write(copyright)

        for p in all_params_iter():
            test_def = template.format(
                format = p.format,
                aspect = p.aspect,
                view = p.view,
                extent = p.extent,
                extent_str = get_extent_str(p),
                levels = p.levels,
                array_length = p.array_length,
                array_length_str = get_array_length_str(p),
                upload = p.upload,
                download = p.download,
                intermediate = p.intermediate,
                aspect_caps = to_caps(p.aspect),
                view_caps = to_caps(p.view),
                upload_caps = to_caps(p.upload),
                download_caps = to_caps(p.download),
                intermediate_caps = to_caps(p.intermediate),
                skip = 'false')
            out_file.write(test_def)

main()
