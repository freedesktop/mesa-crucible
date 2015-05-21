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

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <crucible/cru_array.h>
#include <crucible/cru_macros.h>
#include <crucible/xalloc.h>

bool
cru_array_init(cru_array_t *a)
{
    *a = (cru_array_t) CRU_ARRAY_INIT;
    return true;
}

void
cru_array_finish(cru_array_t *a)
{
    free(a->data);
}

void
cru_array_clear(cru_array_t *a)
{
    free(a->data);

    a->data = NULL;
    a->len = 0;
    a->cap = 0;
}

/// \brief Push bytes onto the array's tail.
///
/// Increase the array's length by \a size bytes and return the starting
/// address of the added memory. This automatically increases the array's
/// capacity if necessary.
void*
cru_array_push(cru_array_t *a, size_t size)
{
    static const size_t initial_capacity = 16;
    size_t cap = a->cap;

    if (cru_unlikely(cap == 0))
        cap = initial_capacity;

    while (cap < a->len + size) {
        if (cru_unlikely(cap > SIZE_MAX / 2)) {
            errno = -ENOMEM;
            return NULL;
        }

        cap *= 2;
    }

    if (a->cap < cap) {
        a->data = xrealloc(a->data, cap);
        a->cap = cap;
    }

    void *elem = a->data + a->len;
    a->len += size;
    return elem;
}

/// Copy bytes onto the array's tail, like cru_array_push().
bool
cru_array_memcpy(cru_array_t *a, void *data, size_t size)
{
    void *dest;

    dest = cru_array_push(a, size);
    if (!dest)
        return false;

    memcpy(dest, data, size);
    return true;
}

/// \brief Pop bytes off the array's tail.
///
/// Reduce the array's length by \a size bytes and return the starting address
/// of the removed memory.  If \a size exceeds the array's length, then empty
/// the array and return null.
void*
cru_array_pop(cru_array_t *a, size_t size)
{
    if (cru_unlikely(a->len == 0)) {
        return NULL;
    } else if (cru_unlikely(a->len < size)) {
        a->len = 0;
        return NULL;
    } else {
        a->len -= size;
        return a->data + a->len;
    }
}

/// This increases the destination array's capacity if necessary.
bool
cru_array_copy(cru_array_t *dest, const cru_array_t *src)
{
    if (src->len > dest->len) {
        if (!cru_array_push(dest, src->len - dest->len)) {
            return false;
        }
    } else {
        dest->len = src->len;
    }

    memcpy(dest->data, src->data, src->len);
    return true;
}
