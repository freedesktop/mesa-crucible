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

// TODO(chadv): Drop this tiny header.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cru_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct string string_t;
typedef int cru_err_t;

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN3(a, b, c) MIN(a, MIN(b, c))
#define MAX3(a, b, c) MAX(a, MAX(b, c))

#define CLAMP(x, a, b) MIN(MAX(x, a), b)

/// \brief Out-of-memory handler.
///
/// Print an error message and exit.
void cru_oom(void) cru_noreturn;

cru_err_t cru_getenv_bool(const char *name, bool default_, bool *result);

static inline bool
cru_streq(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

static inline uint32_t
cru_minify(uint32_t x, uint32_t levels)
{
    if (x == 0)
        return 0;
    else
        return MAX(1, x >> levels);
}

static inline size_t
cru_align_size(size_t n, size_t a)
{
    return (n + a - 1) & ~(a - 1);
}

static inline bool
cru_add_size_checked(size_t *result, size_t a, size_t b)
{
    *result = a + b;
    return a <= SIZE_MAX - b;
}

static inline bool
cru_mul_size_checked(size_t *result, size_t a, size_t b)
{
    *result = a * b;
    return a <= SIZE_MAX / b;
}

/// The path that contains all of Crucible's files. This is normally the top of
/// the git repository.
const string_t * cru_prefix_path(void);

#ifdef __cplusplus
}
#endif
