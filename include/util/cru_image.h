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

/// \file
/// \brief cru_image
///
/// NOTES:
///    - Filenames are relative the 'img' directory.
///
///    - Tests can load miptrees by a sequence of:
///
///         file_img = cru_image_load_file(filename
///         cru_image_copy(tex_image, file_img);
///
///    - Tests can probe render targets with:
///
///         file_img = cru_image_load_file(filename
///         cru_image_compare(tex_image, file_img);
///
///    - Test authors can create new images with:
///
///         cru_image_write_file(tex_image, filename);
///
///    - Images created by cru_image_from_pixels() are read-write.
///
///    - Images created by cru_image_load_file() are read-only.
///
///    - Images hold no reference on their storage. For example, pixel images
///      do not own the backing pixel array.

#include <stdbool.h>

#include "util/macros.h"
#include "util/vk_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cru_image cru_image_t;

enum {
   CRU_IMAGE_MAP_ACCESS_READ = 0x1,
   CRU_IMAGE_MAP_ACCESS_WRITE = 0x2,
};

void cru_image_reference(cru_image_t *image);
void cru_image_release(cru_image_t *image);
uint32_t cru_image_get_width(cru_image_t *image);
uint32_t cru_image_get_height(cru_image_t *image);
VkFormat cru_image_get_format(cru_image_t *image);
malloclike cru_image_t *cru_image_from_pixels(void *restrict pixels, VkFormat format, uint32_t width, uint32_t height);
malloclike cru_image_t *cru_image_load_file(const char *filename);
bool cru_image_write_file(cru_image_t *image, const char *filename);
bool cru_image_copy(cru_image_t *dest, cru_image_t *src);
bool cru_image_compare(cru_image_t *a, cru_image_t *b);
bool cru_image_compare_rect(cru_image_t *a, uint32_t a_x, uint32_t a_y,
                            cru_image_t *b, uint32_t b_x, uint32_t b_y,
                            uint32_t width, uint32_t height);

/// \brief Map the image to an array of pixels.
///
/// The pixel format is cru_image::format. The array is tightly packed (that
/// is, the mapping has minimal pixel stride and row stride).  The \a
/// access_mask must be a bitmask of CRU_IMAGE_MAP_ACCESS_*.
///
/// Return NULL on failure.  Attempting to map an already mapped image will
/// fail. Attempting to map a read-only image (such as a PNG image) with
/// CRU_IMAGE_MAP_ACCESS_WRITE will fail.
void *cru_image_map(cru_image_t *image, uint32_t access_mask);

/// \brief Unmap an image.
///
/// Attempting to unmap an unmapped image will fail. If the image was mapped
/// CRU_IMAGE_MAP_ACCESS_WRITE, then writeback may occur (and may fail) during
/// unmapping.
bool cru_image_unmap(cru_image_t *image);

#ifdef __cplusplus
}
#endif
