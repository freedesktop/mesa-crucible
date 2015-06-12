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

#include <crucible/cru_image.h>
#include <crucible/cru_refcount.h>

enum cru_image_type {
   CRU_IMAGE_TYPE_PIXELS,
   CRU_IMAGE_TYPE_PNG,
   CRU_IMAGE_TYPE_TEXTURE,
};

struct cru_image {
   uint32_t format;
   uint32_t cpp;
   uint32_t width;
   uint32_t height;
   bool read_only;

   cru_refcount_t refcount;
   enum cru_image_type type;

   void (*destroy)(cru_image_t *image);

   /// \see cru_image_map()
   uint8_t *(*map_pixels)(cru_image_t *image, uint32_t access_mask);

   /// \see cru_image_unmap()
   bool (*unmap_pixels)(cru_image_t *image);
};

/// All image code assumes VK_FORMAT_R8G8B8A8_UNORM.
static const uint8_t cru_image_pixel_size = 4;

// file: cru_image.c
bool cru_image_init(cru_image_t *image, enum cru_image_type type, VkFormat format, uint32_t width, uint32_t height, bool read_only);
char *cru_image_get_abspath(const char *filename);

// file: cru_png_image.c
cru_image_t *cru_png_image_load_file(const char *filename);
bool cru_png_image_copy_to_pixels(cru_image_t *png_image, cru_image_t *dest);
