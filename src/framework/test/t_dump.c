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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "test.h"

bool
t_is_dump_enabled(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return !t->opt.no_dump;
}

void
t_dump_seq_image(cru_image_t *image)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (t->opt.no_dump)
        return;

    uint64_t seq = cru_refcount_get(&t->dump_seq);
    t_assertf(seq <= 9999, "image sequence %" PRIu64 " exceeds 9999", seq);

    string_t filename = STRING_INIT;
    string_printf(&filename, "%s.seq%04" PRIu64 ".png", t_name, seq);
    cru_image_write_file(image, string_data(&filename));
}

void printflike(2, 3)
t_dump_image_f(cru_image_t *image, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    t_dump_image_fv(image, format, va);
    va_end(va);
}

void
t_dump_image_fv(cru_image_t *image, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (t->opt.no_dump)
        return;

    string_t filename = STRING_INIT;
    string_append_cstr(&filename, t_name);
    string_append_char(&filename, '.');
    string_vappendf(&filename, format, va);

    cru_image_write_file(image, string_data(&filename));
}
