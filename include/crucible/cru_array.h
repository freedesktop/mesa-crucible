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
#include <stddef.h>

#include <crucible/cru_macros.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cru_array cru_array_t;

/// \brief A flexible array
///
/// This array automatically reallocates its storage when needed.
struct cru_array {
    void *data;
    size_t len; ///< Length of in-use data, in bytes
    size_t cap; ///< Capacity of array data, in bytes
};

/// \brief Static initializer for cru_array_t
#define CRU_ARRAY_INIT {0}

bool cru_array_init(cru_array_t *a);
void cru_array_finish(cru_array_t *a);
void cru_array_clear(cru_array_t *a);
void* cru_array_push(cru_array_t *a, size_t size) cru_malloclike;
bool cru_array_memcpy(cru_array_t *a, void *data, size_t size);
void* cru_array_pop(cru_array_t *a, size_t size);
bool cru_array_copy(cru_array_t *dest, const cru_array_t *src);

#define cru_array_foreach(elem, array) \
    cru_static_assert(__builtin_types_compatible_p(__typeof__(array), \
                                                   cru_array_t*)); \
    for (elem = (array)->data; \
        (void *) elem < (array)->data + (array)->len; \
        ++elem)

static inline bool
cru_array_is_empty(cru_array_t *a)
{
    return a->len == 0;
}

#ifdef __cplusplus
}
#endif
