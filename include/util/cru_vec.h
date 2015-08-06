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

#include "util/macros.h"

typedef struct __cru_vec __cru_vec_t;
typedef struct cru_void_vec cru_void_vec_t;
typedef struct cru_cstr_vec cru_cstr_vec_t;
typedef struct cru_vec cru_vec_t;

#ifdef DOXYGEN
/// \brief A flexible, type-safe vector.
///
/// The vector automatically reallocates its storage when needed.
///
/// \see CRU_VEC_DEFINE()
///
struct cru_vec {
    elem_t *data;

    /// Number of elements in vector.  If the vector's element type is
    /// `void*`, however, length is in bytes.
    size_t len;

    /// Capacity of allocated data, in units of element size. If the vector's
    /// type is `void*`, however, capacity is in bytes.
    ///
    /// For example, if the vector's element type is `struct theropod` and the
    /// vector's capacity is 100, then the array can hold 100 theropods.
    size_t cap;
};
#else
struct __cru_vec {
    void *data;
    size_t len;
    size_t cap;
};
#endif

#define CRU_VEC_DEFINE(__vec_t, __elem_t)                                   \
                                                                            \
    __vec_t {                                                               \
        union {                                                             \
            struct __cru_vec priv;                                          \
                                                                            \
            /* Must be compatible with struct __cru_vec. */                 \
            struct {                                                        \
                __elem_t *data;                                             \
                size_t len;                                                 \
                size_t cap;                                                 \
            };                                                              \
        };                                                                  \
    };

/// Static initializer for cru_vec_t.
#define CRU_VEC_INIT \
    \
    /* To enforce type-safety, initialize __vec_t::priv with a typed
     * initializer. */ \
    { .priv = (__cru_vec_t) {0} }

#ifdef DOXYGEN
/// Get size of the vector's element type.
size_t cru_vec_elem_size(const cru_vec_t *v);
#else
#define cru_vec_elem_size(v) \
    _Generic((v)->data, \
        void *: (size_t) 1, \
        default: sizeof((v)->data[0]))
#endif

#ifdef DOXYGEN
void cru_vec_init(cru_vec_t *v);
#else
#define cru_vec_init(v) \
    __cru_vec_init(&(v)->priv)
#endif

#ifdef DOXYGEN
void cru_vec_finish(cru_vec_t *v);
#else
#define cru_vec_finish(v) \
    __cru_vec_finish(&(v)->priv)
#endif

#ifdef DOXYGEN
/// Grow the vector's capacity to hold at least elem_count additional
/// elements.
///
/// Aborts on failure.
void cru_vec_grow_capacity(cru_vec_t *v, size_t elem_count);
#else
#define cru_vec_grow_capacity(v, elem_count) \
    __cru_vec_grow_capcity(&(v)->priv, (elem_count), cru_vec_elem_size(v))
#endif

#ifdef DOXYGEN
/// Grow the vector's capacity to hold at least elem_count elements.
///
/// Aborts on failure.
void cru_vec_grow_capacity_to(cru_vec_t *v, size_t elem_count);
#else
#define cru_vec_grow_capacity_to(v, elem_count) \
    __cru_vec_grow_to(&(v)->priv, (elem_count), cru_vec_elem_size(v))
#endif

#ifdef DOXYGEN
/// Push new elements onto vector's tail.
///
/// Increase the vector's length by \a elem_count and return the starting
/// address of the new elements. This automatically increases the vector's
/// capacity if needed.
///
/// Aborts on failure.
elem_t *
cru_vec_push(cru_vec_t *v, size_t elem_count);
#else
#define cru_vec_push(v, elem_count) \
    ((__typeof__((v)->data)) \
        __cru_vec_push(&(v)->priv, (elem_count), cru_vec_elem_size(v)))
#endif

#ifdef DOXYGEN
/// Copy elements onto vector's tail.
///
/// Behaves like cru_vec_push() with a follow-up memcpy.
///
/// Aborts on failure.
elem_t *
cru_vec_push_memcpy(cru_vec_t *v, const elem_t *restrict elems,
                    size_t elem_count);
#else
#define cru_vec_push_memcpy(v, elems, elem_count) \
({ \
    /* Statically assert that elems has the correct type. */ \
    (void) sizeof(((v)->data = elems)); \
    (__typeof__((v)->data)) __cru_vec_push_memcpy(&(v)->priv, \
                                                  (elems), (elem_count), \
                                                  cru_vec_elem_size(v)); \
})
#endif

#ifdef DOXYGEN
/// Pop elements off the vector's tail.
///
/// Reduce the vector's length by \a elem_count and return the starting
/// address of the removed elements.  If \a elem_count is 0, then return NULL.
/// Fails if \a elem_count exceeds the vector's length.
///
/// Aborts on failure.
elem_t *
cru_vec_pop(cru_vec_t *v, size_t elem_count);
#else
#define cru_vec_pop(v, elem_count) \
    __cru_vec_pop(&(v)->priv, (elem_count), cru_vec_elem_size(v))
#endif

#ifdef DOXYGEN
/// Clear all elements from the vector, resetting its length to 0.
///
/// Aborts on failure.
void cru_vec_clear(cru_vec_t *v);
#else
#define cru_vec_clear(v) \
    __cru_vec_clear(&(v)->priv)
#endif

#ifdef DOXYGEN
void cru_vec_copy(cru_vec_t *restrict dest, const vec_t *restrict src);
#else
#define cru_vec_copy(dest, src) \
    do { \
        /* Statically assert that types are compatible. */ \
        (void) sizeof((dest)->data = (src)->data); \
        __cru_vec_copy(&(a)->priv, &(b)->priv); \
    } while (0)
#endif

/// \brief Loop over all vector elements.
///
/// \par Example Code
///
///     void
///     print_vec(const cru_cstr_vec_t *v)
///     {
///         const char **str;
///
///         cru_vec_foreach(str, v) {
///             printf("%s\n", *str);
///         }
///
#define cru_vec_foreach(elem_ptr, v) \
    for (elem_ptr = (v)->data; elem_ptr < (v)->data + (v)->len; ++elem_ptr)

CRU_VEC_DEFINE(struct cru_void_vec, void)
CRU_VEC_DEFINE(struct cru_cstr_vec, char *)

void __cru_vec_init(__cru_vec_t *v);
void __cru_vec_finish(__cru_vec_t *v);
void __cru_vec_clear(__cru_vec_t *v);
void __cru_vec_grow_capacity(__cru_vec_t *v, size_t num_elems, size_t elem_size);
void __cru_vec_grow_capacity_to(__cru_vec_t *v, size_t num_elems, size_t elem_size);
void *__cru_vec_push(__cru_vec_t *v, size_t num_elems, size_t elem_size);
void *__cru_vec_push_memcpy(__cru_vec_t *v, void *restrict elems, size_t n, size_t elem_size);
void *__cru_vec_pop(__cru_vec_t *v, size_t num_elems, size_t elem_size);
void __cru_vec_copy(__cru_vec_t *restrict dest, const __cru_vec_t *restrict src, size_t elem_size);
