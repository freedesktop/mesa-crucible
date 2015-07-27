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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <crucible/cru_vec.h>
#include <crucible/cru_macros.h>
#include <crucible/cru_misc.h>
#include <crucible/xalloc.h>

#define CRU_VEC_INIT_CAPACITY 16

void
__cru_vec_init(__cru_vec_t *v)
{
    *v = (__cru_vec_t) {0};
}

void
__cru_vec_finish(__cru_vec_t *v)
{
    free(v->data);
}

void
__cru_vec_clear(__cru_vec_t *v)
{
    free(v->data);
    __cru_vec_init(v);
}

void
__cru_vec_grow_capacity(__cru_vec_t *v, size_t num_elems, size_t elem_size)
{
    size_t min_cap;
    size_t new_cap;

    if (!cru_add_size_checked(&min_cap, v->cap, num_elems))
        cru_oom();

    min_cap = MAX(min_cap, CRU_VEC_INIT_CAPACITY);
    new_cap = MAX(v->cap, CRU_VEC_INIT_CAPACITY);

    // Increase the capacity exponentially, not linearly.
    while (new_cap < min_cap) {
        if (!cru_mul_size_checked(&new_cap, 2, new_cap)) {
            cru_oom();
        }
    }

    __cru_vec_grow_capacity_to(v, new_cap, elem_size);
}

void
__cru_vec_grow_capacity_to(__cru_vec_t *v, size_t num_elems, size_t elem_size)
{
    if (num_elems <= v->cap)
        return;

    v->data = xreallocn(v->data, num_elems, elem_size);
    v->cap = num_elems;
}

void *
__cru_vec_push(__cru_vec_t *v, size_t num_elems, size_t elem_size)
{
    size_t new_len;
    void *new_elems;

    if (!cru_add_size_checked(&new_len, v->len, num_elems))
        cru_oom();

    if (v->cap < new_len)
        __cru_vec_grow_capacity(v, new_len, elem_size);

    new_elems = v->data + v->len * elem_size;
    v->len = new_len;

    return new_elems;
}

void *
__cru_vec_push_memcpy(__cru_vec_t *v, void *restrict elems,
                      size_t num_elems, size_t elem_size)
{
    void *new_elems;

    new_elems = __cru_vec_push(v, num_elems, elem_size);
    memcpy(new_elems, elems, num_elems * elem_size);

    return new_elems;
}

void *
__cru_vec_pop(__cru_vec_t *v, size_t num_elems, size_t elem_size)
{
    if (num_elems > v->len) {
        fprintf(stderr, "crucible: %s() overflow\n", __func__);
        abort();
    } else if (num_elems == 0) {
        return NULL;
    } else {
        v->len -= num_elems;
        return v->data + v->len * elem_size;
    }
}

void
__cru_vec_copy(__cru_vec_t *restrict dest,
               const __cru_vec_t *restrict src,
               size_t elem_size)
{
    __cru_vec_grow_capacity_to(dest, src->len, elem_size);
    dest->len = src->len;
    memcpy(dest->data, src->data, src->len * elem_size);
}
