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

Params = namedtuple('Params', ('format', 'view', 'levels', 'array_length',
                    'upload', 'download'))

params_iter = (
    Params(format, view, levels, array_length, upload_method, download_method)
    for format in (('r8g8b8a8-unorm', 'VK_FORMAT_R8G8B8A8_UNORM'),)
    for view in ('2d',)
    for levels in (1, 2)
    for array_length in (1, 2)
    for upload_method in (
        'copy-from-buffer',
        'copy-from-linear-image',
        'render')
    for download_method in (
        'copy-to-buffer',
        'copy-to-linear-image',
        'render')
)

template = dedent("""
    cru_define_test {{
        .name = "func.miptree"
                ".{format[0]}"
                ".view-{view}.levels{levels:02}.array{array_length:02}"
                ".upload-{upload}.download-{download}",
        .start = test,
        .no_image = true,
        .user_data = &(test_params_t) {{
            .format = {format[1]},
            .view_type = VK_IMAGE_VIEW_TYPE_{view_caps},
            .levels = {levels},
            .width = 512,
            .height = 512,
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

        for p in params_iter:
            test_def = template.format(
                format = p.format,
                view = p.view,
                levels = p.levels,
                array_length = p.array_length,
                upload = p.upload,
                download = p.download,
                view_caps = to_caps(p.view),
                upload_caps = to_caps(p.upload),
                download_caps = to_caps(p.download))
            out_file.write(test_def)

main()
