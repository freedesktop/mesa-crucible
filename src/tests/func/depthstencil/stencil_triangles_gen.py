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

Enum = namedtuple('Enum', ('vk', 'short'))

VK_COMPARE_OP_ALWAYS        = Enum('VK_COMPARE_OP_ALWAYS',        'always')
VK_COMPARE_OP_NEVER         = Enum('VK_COMPARE_OP_NEVER',         'never')
VK_COMPARE_OP_LESS          = Enum('VK_COMPARE_OP_LESS',          'less')
VK_COMPARE_OP_LESS_EQUAL    = Enum('VK_COMPARE_OP_LESS_EQUAL',    'less-equal')
VK_COMPARE_OP_EQUAL         = Enum('VK_COMPARE_OP_EQUAL',         'equal')
VK_COMPARE_OP_GREATER_EQUAL = Enum('VK_COMPARE_OP_GREATER_EQUAL', 'greater-equal')
VK_COMPARE_OP_GREATER       = Enum('VK_COMPARE_OP_GREATER',       'greater')

VK_STENCIL_OP_KEEP      = Enum('VK_STENCIL_OP_KEEP',      'keep')
VK_STENCIL_OP_ZERO      = Enum('VK_STENCIL_OP_ZERO',      'zero')
VK_STENCIL_OP_REPLACE   = Enum('VK_STENCIL_OP_REPLACE',   'replace')
VK_STENCIL_OP_INC_CLAMP = Enum('VK_STENCIL_OP_INC_CLAMP', 'inc-clamp')
VK_STENCIL_OP_DEC_CLAMP = Enum('VK_STENCIL_OP_DEC_CLAMP', 'dec-clamp')
VK_STENCIL_OP_INVERT    = Enum('VK_STENCIL_OP_INVERT',    'invert')
VK_STENCIL_OP_INC_WRAP  = Enum('VK_STENCIL_OP_INC_WRAP',  'inc-wrap')
VK_STENCIL_OP_DEC_WRAP  = Enum('VK_STENCIL_OP_DEC_WRAP',  'dec-wrap')

VK_COMPARE_OPS = [
    VK_COMPARE_OP_ALWAYS,
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_LESS_EQUAL,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_GREATER_EQUAL,
    VK_COMPARE_OP_GREATER,
]

VK_STENCIL_OPS = [
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INC_CLAMP,
    VK_STENCIL_OP_DEC_CLAMP,
    VK_STENCIL_OP_INVERT,
    VK_STENCIL_OP_INC_WRAP,
    VK_STENCIL_OP_DEC_WRAP,
]

template = dedent("""
    test_define {{
        .start = test,
        .name = "func.depthstencil.stencil-triangles"
                ".clear-0x{clear_value:x}"
                ".ref-0x{ref:x}"
                ".compare-op-{short_compare_op}"
                ".pass-op-{short_pass_op}"
                ".fail-op-{short_fail_op}",
        .user_data = &(test_params_t) {{
            .clear_value = {{ .stencil = 0x{clear_value:x} }},
            .stencil_ref = 0x{ref:x},
            .stencil_compare_op = {vk_compare_op},
            .stencil_pass_op = {vk_pass_op},
            .stencil_fail_op = {vk_fail_op},
        }},
        .ref_stencil_filename = "DEFAULT",
        .depthstencil_format = VK_FORMAT_S8_UINT,
    }};
    """)

def iter_params():
    # When compare_op == VK_COMPARE_OP_ALWAYS, fail_op is always a no-op, and
    # so there's no reason to iterate over it. Just set the fail_op to
    # anything other than the identity operation (VK_STENCIL_OP_KEEP).
    for p in (Params(0x17, 0x17, compare_op, pass_op, fail_op)
              for compare_op in (VK_COMPARE_OP_ALWAYS,)
              for pass_op in VK_STENCIL_OPS
              for fail_op in (VK_STENCIL_OP_INVERT,)):
        yield p

    # When compare_op == VK_COMPARE_OP_NEVER, pass_op is always a no-op, and
    # so there's no reason to iterate over it. Just set the pass_op to
    # anything other than the identity operation (VK_STENCIL_OP_KEEP).
    for p in (Params(0x17, 0x17, compare_op, pass_op, fail_op)
              for compare_op in (VK_COMPARE_OP_NEVER,)
              for pass_op in (VK_STENCIL_OP_INVERT,)
              for fail_op in VK_STENCIL_OPS):
        yield p

    for p in (Params(0x17, 0x17, compare_op, pass_op, fail_op)
              for compare_op in (VK_COMPARE_OP_LESS,
                                 VK_COMPARE_OP_LESS_EQUAL,
                                 VK_COMPARE_OP_EQUAL,
                                 VK_COMPARE_OP_GREATER_EQUAL,
                                 VK_COMPARE_OP_GREATER)
              for pass_op in VK_STENCIL_OPS
              for fail_op in VK_STENCIL_OPS):
        yield p

class Params:

    def __init__(self, clear_value, ref, compare_op, pass_op, fail_op):
        self.clear_value = clear_value
        self.ref = ref
        self.compare_op = compare_op
        self.pass_op = pass_op
        self.fail_op = fail_op

    def write(self, buf):
        buf.write(template.format(
            clear_value = self.clear_value,
            ref = self.ref,

            vk_compare_op = self.compare_op.vk,
            vk_pass_op = self.pass_op.vk,
            vk_fail_op = self.fail_op.vk,

            short_compare_op = self.compare_op.short,
            short_pass_op = self.pass_op.short,
            short_fail_op = self.fail_op.short))

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

def main():
    out_filename = __file__.replace('.py', '.c')

    with open(out_filename, 'w') as out_file:
        out_file.write(copyright)
        for p in iter_params():
            p.write(out_file)

main()
