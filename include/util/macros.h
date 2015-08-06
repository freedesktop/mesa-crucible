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

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRU_VERSION_MAJOR 0
#define CRU_VERSION_MINOR 1

#define cru_noreturn __attribute__((__noreturn__))
#define cru_constructor __attribute__((__constructor__))
#define cru_printflike(a, b) __attribute__((__format__(__printf__, a, b)))
#define cru_pure __attribute__((__pure__))
#define cru_malloclike __attribute__((__malloc__))

#define cru_static_assert(expr) _Static_assert((expr), "")

#define cru_likely(expr) __builtin_expect(!!(expr), 1)
#define cru_unlikely(expr) __builtin_expect(!!(expr), 0)

#define cru_unreachable \
    do { \
        assert(0); \
        __builtin_unreachable(); \
    } while (0)

#define __CRU_CAT(x, y) x##y
#define CRU_CAT(x, y) __CRU_CAT(x, y)

#define GLSL(version, src) ("#version " #version "\n" #src)

#ifdef __cplusplus
}
#endif
