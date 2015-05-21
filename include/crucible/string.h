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
#include <stdbool.h>
#include <stddef.h>

#include <crucible/cru_macros.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct string string_t;

struct string {
    char *buf;

    /// Length of string. Agrees with strlen().
    size_t len;

    /// Total allocated size of \ref data.
    size_t cap;
};

extern char string_dummy[1];

/// Statically initialize to the dummy string.
#define STRING_INIT ((string_t) {.buf = string_dummy})

/// Is the string read-only?
static inline bool
string_is_ro(const string_t *s)
{
    return s->cap == 0;
}

/// Initialize to the dummy string.
static inline void
string_init(string_t *s)
{
    *s = STRING_INIT;
    assert(string_is_ro(s));
}

static inline void
string_set_len(string_t *s, size_t len)
{
    assert(len < s->cap);
    s->len = len;
    s->buf[s->len] = 0;
}

void string_finish(string_t *s);
string_t *string_new(void) cru_malloclike;
void string_free(string_t *s);
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
bool path_is_abs(const string_t *path);
void path_get_abs(string_t *dest, const string_t *path);
void path_append(string_t *dest, const string_t *tail);
void path_append_cstr(string_t *dest, const char *tail);
void path_append_raw(string_t *dest, const char *tail, size_t len);
void path_dirname(string_t *path);

#ifdef __cplusplus
}
#endif
