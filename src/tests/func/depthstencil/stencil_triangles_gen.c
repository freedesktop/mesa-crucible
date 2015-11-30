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

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-keep"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-zero"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-replace"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-inc-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-dec-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-inc-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-always"
            ".pass-op-dec-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_ALWAYS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-never"
            ".pass-op-invert"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_NEVER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-keep"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-zero"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-replace"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-invert"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-inc-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less"
            ".pass-op-dec-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-keep"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-zero"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-replace"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-invert"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-inc-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-less-equal"
            ".pass-op-dec-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-keep"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-zero"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-replace"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-invert"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-inc-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-equal"
            ".pass-op-dec-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-keep"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-zero"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-replace"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-invert"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-inc-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater-equal"
            ".pass-op-dec-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER_OR_EQUAL,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-keep"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_KEEP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-zero"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_ZERO,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-replace"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_REPLACE,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-clamp"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-invert"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INVERT,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-inc-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-keep",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_KEEP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-zero",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_ZERO,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-replace",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_REPLACE,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-inc-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-dec-clamp",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-invert",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INVERT,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-inc-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_INCREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};

test_define {
    .start = test,
    .name = "func.depthstencil.stencil-triangles"
            ".clear-0x17"
            ".ref-0x17"
            ".compare-op-greater"
            ".pass-op-dec-wrap"
            ".fail-op-dec-wrap",
    .user_data = &(test_params_t) {
        .clear_value = { .stencil = 0x17 },
        .stencil_ref = 0x17,
        .stencil_compare_op = VK_COMPARE_OP_GREATER,
        .stencil_pass_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
        .stencil_fail_op = VK_STENCIL_OP_DECREMENT_AND_WRAP,
    },
    .ref_stencil_filename = "DEFAULT",
    .depthstencil_format = VK_FORMAT_S8_UINT,
};
