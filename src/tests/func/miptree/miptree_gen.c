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
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-with-draw.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-with-draw.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array01"
            ".upload-copy-with-draw.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-with-draw.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-with-draw.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels01.array02"
            ".upload-copy-with-draw.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-with-draw.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-with-draw.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array01"
            ".upload-copy-with-draw.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-with-draw.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-with-draw.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".r8g8b8a8-unorm"
            ".aspect-color"
            ".extent-512x512"
            ".view-2d.levels02.array02"
            ".upload-copy-with-draw.download-copy-with-draw",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .aspect = VK_IMAGE_ASPECT_COLOR,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 512,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_WITH_DRAW,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_WITH_DRAW,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".d32-sfloat"
            ".aspect-depth"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_D32_SFLOAT,
        .aspect = VK_IMAGE_ASPECT_DEPTH,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array01"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels01.array02"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 1,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array01"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 1,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-buffer.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_BUFFER,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-to-buffer",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_BUFFER,
    },
};

test_define {
    .name = "func.miptree"
            ".s8-uint"
            ".aspect-stencil"
            ".extent-1024x512"
            ".view-2d.levels02.array02"
            ".upload-copy-from-linear-image.download-copy-to-linear-image",
    .start = test,
    .skip = false,
    .no_image = true,
    .user_data = &(test_params_t) {
        .format = VK_FORMAT_S8_UINT,
        .aspect = VK_IMAGE_ASPECT_STENCIL,
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .levels = 2,
        .width = 1024,
        .height = 512,
        .array_length = 2,
        .upload_method = MIPTREE_UPLOAD_METHOD_COPY_FROM_LINEAR_IMAGE,
        .download_method = MIPTREE_DOWNLOAD_METHOD_COPY_TO_LINEAR_IMAGE,
    },
};
