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
#include <endian.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct string string_t;

/// The string has a "short" and "long" layout. In the short layout, the
/// string's character array is embedded in the struct. In the long layout, the
/// character array is allocated separately.
///
/// The short layout exists mainly not to increase performance but:
///   * To ensure that statically initialized strings are always valid, and
///   * To allow static initialization of non-empty strings.
///
struct string {
#if BYTE_ORDER == LITTLE_ENDIAN
    /// Length of string. Agrees with strlen().
    size_t len:(CHAR_BIT * sizeof(size_t) - 1);

    bool is_short:1;
#else
#   error untested endianness
#endif

    union {
        /// If the string is "short", then short_buf::data holds
        /// a null-terminated string.
        struct {
            char data[3 * sizeof(void*)];
        } short_buf;

        /// If the string is "long", then long_buf::data holds
        /// a null-terminated string whose allocated size is long_buf::cap;
        struct {
            char *data;
            size_t cap;
        } long_buf;
    };
};

/// Statically initialize as a "short" string.
#define STRING_INIT_SHORT(__str_literal) \
    (string_t) { \
        .len = sizeof(__str_literal) - 1, \
        .is_short = true, \
        .short_buf.data = {(__str_literal)}, \
    }

/// Statically initialize as a "short" empty string.
#define STRING_INIT STRING_INIT_SHORT("")

/// Initialize to an empty string.
static inline void
string_init(string_t *s)
{
    *s = STRING_INIT;
}

static inline size_t
string_capacity(const string_t *s)
{
    if (s->is_short) {
        return sizeof(s->short_buf.data);
    } else {
        return s->long_buf.cap;
    }
}

static inline char *
__string_data(string_t *s)
{
    if (s->is_short) {
        return s->short_buf.data;
    } else {
        return s->long_buf.data;
    }
}

static inline const char *
__string_data_const(const string_t *s)
{
    if (s->is_short) {
        return s->short_buf.data;
    } else {
        return s->long_buf.data;
    }
}

#define string_data(s) \
    _Generic((s), \
        const string_t *: __string_data_const, \
        string_t *: __string_data \
    )(s)

static inline void
string_set_len(string_t *s, size_t len)
{
    assert(len < string_capacity(s));

    s->len = len;
    string_data(s)[s->len] = 0;
}

void string_finish(string_t *s);
malloclike string_t *string_new(void);
void string_free(string_t *s);
void string_make_long(string_t *s);
void string_truncate(string_t *s, size_t len);
void string_attach(string_t *s, char *cstr, size_t len, size_t cap);
void string_attach_ro(string_t *s, char *cstr, size_t len);
char *string_detach(string_t *s);
void string_grow(string_t *s, size_t len);
void string_grow_to_len(string_t *s, size_t len);
void string_append(string_t *s, const string_t *tail);
void string_append_cstr(string_t *s, const char *tail);
void string_append_raw(string_t *s, const void *restrict src, size_t len);
void string_append_char(string_t *s, char c);
void string_copy(string_t *dest, const string_t *src);
void string_copy_cstr(string_t *dest, const char *src);
void string_copy_raw(string_t *s, const void *restrict src, size_t len);
int string_cmp(const string_t *a, const string_t *b);
bool string_startswith(const string_t *s, const string_t *head);
bool string_startswith_cstr(const string_t *s, const char *head);
bool string_endswith(const string_t *s, const string_t *tail);
bool string_endswith_cstr(const string_t *s, const char *tail);
int64_t string_rfind_char(const string_t *s, char c);
void string_rstrip_char(string_t *s, char c);
void string_printf(string_t *s, const char *format, ...) printflike(2, 3);
void string_vprintf(string_t *s, const char *format, va_list va);
void string_appendf(string_t *s, const char *format, ...) printflike(2, 3);
void string_vappendf(string_t *s, const char *format, va_list va);

bool path_is_abs(const string_t *path);
void path_to_abs(string_t *restrict dest, const string_t *restrict path);
void path_append(string_t *dest, const string_t *tail);
void path_append_cstr(string_t *dest, const char *tail);
void path_append_raw(string_t *dest, const char *tail, size_t len);
void path_appendf(string_t *dest, const char *format, ...) printflike(2, 3);
void path_vappendf(string_t *dest, const char *format, va_list va);
void path_dirname(string_t *path);

#ifdef __cplusplus
}
#endif
