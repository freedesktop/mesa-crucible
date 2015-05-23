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

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <crucible/cru_log.h>
#include <crucible/cru_misc.h>
#include <crucible/string.h>
#include <crucible/xalloc.h>

char string_dummy[1] = {0};

void
string_finish(string_t *s)
{
    if (!string_is_ro(s))
        free(s->buf);
}

string_t * cru_malloclike
string_new(void)
{
    string_t *s = xmalloc(sizeof(*s));
    string_init(s);
    return s;
}

void
string_free(string_t *s)
{
    if (!s)
        return;

    string_finish(s);
    free(s);
}

void
string_truncate(string_t *s, size_t len)
{
    if (string_is_ro(s))
        string_copy_raw(s, s->buf, s->len);

    if (s->len < len)
        return;

    string_set_len(s, len);
}

void
string_attach(string_t *s, char *cstr, size_t len, size_t cap)
{
    if (!string_is_ro(s)) {
        free(s->buf);
    }

    s->buf = cstr;
    s->len = len;
    s->cap = cap;
}

void
string_attach_ro(string_t *s, char *cstr, size_t len)
{
    string_attach(s, cstr, len, 0);
}

char *
string_detach(string_t *s)
{
    char *cstr = s->buf;
    string_init(s);
    return cstr;
}

void
string_grow_to_len(string_t *s, size_t len)
{
    static const size_t initial_capacity = 64;
    const size_t old_cap = s->cap;
    size_t new_cap = len + 1;

    if (new_cap == 0) {
        new_cap = initial_capacity;
    }

    // Round up new_cap to a power of 2.
    uint8_t high_bit = CHAR_BIT * sizeof(new_cap) - __builtin_clzl(new_cap);
    if (new_cap != 1 << (high_bit - 1)) {
        if (high_bit == CHAR_BIT * sizeof(new_cap)) {
            xdie_oom();
        }
        new_cap = 1 << high_bit;
    }

    while (new_cap < len + 1) {
        if (cru_unlikely(new_cap > SIZE_MAX / 2)) {
            xdie_oom();
        }
        new_cap *= 2;
    }

    if (old_cap < new_cap) {
        if (string_is_ro(s)) {
            const char *const old_buf = s->buf;
            char *new_buf = xzalloc(new_cap);
            memcpy(new_buf, old_buf, s->len);
            new_buf[s->len] = 0;
            s->buf = new_buf;
        } else {
            s->buf = xrealloc(s->buf, new_cap);
            // Be safe! Zero-fill the newly allocated bytes.
            memset(s->buf + old_cap, 0, new_cap - old_cap);
        }

        s->cap = new_cap;
    }
}

void
string_grow(string_t *s, size_t len)
{
    string_grow_to_len(s, s->len + len);
}

void
string_append(string_t *s, const string_t *tail)
{
    string_append_raw(s, tail->buf, tail->len);
}

void
string_append_cstr(string_t *s, const char *tail)
{
    string_append_raw(s, tail, strlen(tail));
}

void
string_append_raw(string_t *s, const void *restrict src, size_t len)
{
    string_grow(s, len);
    memcpy(s->buf + s->len, src, len);
    string_set_len(s, s->len + len);
}

void
string_append_char(string_t *s, char c)
{
    string_grow(s, 1);
    s->buf[s->len] = c;
    string_set_len(s, s->len + 1);
}

void
string_copy(string_t *dest, const string_t *src)
{
    string_copy_raw(dest, src->buf, src->len);
}

void
string_copy_cstr(string_t *dest, const char *src)
{
    string_copy_raw(dest, src, strlen(src));
}

void
string_copy_raw(string_t *dest, const void *restrict src, size_t len)
{
    string_grow_to_len(dest, len);
    memcpy(dest->buf, src, len);
    string_set_len(dest, len);
}

int
string_cmp(const string_t *a, const string_t *b)
{
    int cmp = memcmp(a->buf, b->buf, MIN(a->len, b->len));

    if (cmp != 0) {
        return cmp;
    } else if (a->len > b->len) {
        return 1;
    } else {
        return -1;
    }
}

static bool
string_startswith_raw(const string_t *s, const char *head, size_t head_len)
{
    if (s->len < head_len)
        return false;
    else
        return !memcmp(s->buf, head, head_len);
}

bool
string_startswith(const string_t *s, const string_t *head)
{
    return string_startswith_raw(s, head->buf, head->len);
}

bool
string_startswith_cstr(const string_t *s, const char *head)
{
    return string_startswith_raw(s, head, strlen(head));
}

static bool
string_endswith_raw(const string_t *s, const char *tail, size_t tail_len)
{
    if (s->len < tail_len)
        return false;
    else
        return !memcmp(s->buf + (s->len - tail_len), tail, tail_len);
}

bool
string_endswith(const string_t *s, const string_t *tail)
{
    return string_endswith_raw(s, tail->buf, tail->len);
}

bool
string_endswith_cstr(const string_t *s, const char *tail)
{
    return string_endswith_raw(s, tail, strlen(tail));
}

int64_t
string_rfind_char(const string_t *s, char c)
{
    for (int64_t i = s->len - 1; i >= 0; --i) {
        if (s->buf[i] == c) {
            return i;
        }
    }

    return -1;
}

/// Strip all occurences of a character from the right of the string.
///
/// \section Examples
///
///     c    before     after
///     -------------------------
///     'a'  "aabcbaa"  "aabcb"
///     'a'  "aabcb"    "aabcb"
///     'b'  "aabcbaa"  "aabcbaa"
///     'x'  "xxx"      ""
///     'x'  ""         ""
void
string_rstrip_char(string_t *s, char c)
{
    while (s->len > 0 && s->buf[s->len - 1] == c) {
        string_set_len(s, s->len - 1);
    }
}

/// Like sprintf, but resized the string as needed.
void cru_printflike(2, 3)
string_printf(string_t *s, const char *format, ...)
{
    va_list va;

    va_start(va, format);
    string_vprintf(s, format, va);
    va_end(va);
}

void
string_vprintf(string_t *s, const char *format, va_list va)
{
    string_set_len(s, 0);
    string_vappendf(s, format, va);
}

/// Like string_printf(), but appends to rather than replaces the string.
void cru_printflike(2, 3)
string_appendf(string_t *s, const char *format, ...)
{
    va_list va;

    va_start(va, format);
    string_appendf(s, format, va);
    va_end(va);
}

void
string_vappendf(string_t *s, const char *format, va_list va)
{
    va_list va_probe;
    va_copy(va_probe, va);

    int len = vsnprintf(NULL, 0, format, va_probe);
    if (len < 0) {
        cru_loge("vsnprintf failed");
        abort();
    }

    va_end(va_probe);

    string_grow(s, len);

    if (vsnprintf(s->buf + s->len, s->cap, format, va) != len) {
        cru_loge("vsnprintf failed");
        abort();
    }

    string_set_len(s, s->len + len);
}

bool
path_is_abs(const string_t *path)
{
    return path->buf[0] == '/';
}

void
path_get_abs(string_t *dest, const string_t *path)
{
    if (path_is_abs(path)) {
        string_copy(dest, path);
        return;
    }
}

void
path_append(string_t *dest, const string_t *tail)
{
    path_append_raw(dest, tail->buf, tail->len);
}

void
path_append_cstr(string_t *dest, const char *tail)
{
    path_append_raw(dest, tail, strlen(tail));
}

void
path_append_raw(string_t *dest, const char *tail, size_t len)
{
    if (len == 0) {
        string_truncate(dest, 0);
    } else if (tail[0] == '/' || dest->len == 0) {
        string_copy_raw(dest, tail, len);
    } else {
        if (!string_endswith_cstr(dest, "/")) {
            string_append_char(dest, '/');
        }
        string_append_raw(dest, tail, len);
    }
}

/// Parse the dirname of path, in-place.
///
///
/// This function is an easier-to-use replacement for dirname(3) from libgen.h.
///
/// \section Examples
///
///    path       dirname  notes
///    ------------------------------
///    /          /
///    /usr       /
///    /usr/      /
///    /usr/bin   /usr
///    /usr/bin/  /usr
///    foo        .
///    foo/       .
///    foo/bar    foo
///    foo/bar/   foo
///               .        empty string
///    .          .
///    ./         .
///    ..         ..
///    ../        ..
///
void
path_dirname(string_t *path)
{
    if (path->len == 0) {
        // Always return a valid path, even when given an empty path.
        string_append_char(path, '.');
        return;
    }

    // Normalize path by stripping trailiing separators.
    string_rstrip_char(path, '/');

    if (path->len == 0) {
        // Oops, we stripped down the root path to the empty path.
        string_append_char(path, '/');
        return;
    }

    int64_t new_len = string_rfind_char(path, '/');
    if (new_len < 0) {
        new_len = 0;
    }

    string_set_len(path, new_len);
    if (new_len == 0) {
        string_append_char(path, '.');
    }
}
