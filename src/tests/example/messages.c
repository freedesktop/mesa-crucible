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

#include <crucible/cru.h>

static void
test_assert(void)
{
    t_assert(2 + 2 == 5);
}

cru_define_test {
    .name = "example.messages.assert",
    .start = test_assert,
    .no_image = true,
};

static void
test_assertf(void)
{
    for (int i = 0; i < 8; ++i) {
        t_assertf(i == -i, "%d is not its additive inverse today (%s)",
                  i, __DATE__);
    }
}

cru_define_test {
    .name = "example.messages.assertf",
    .start = test_assertf,
    .no_image = true,
};

static void
test_skipf(void)
{
    t_skipf("i skipped on %s", __DATE__);
}

cru_define_test {
    .name = "example.messages.skipf",
    .start = test_skipf,
    .no_image = true,
};

static void
test_failf(void)
{
    t_failf("i failed on %s", __DATE__);
}

cru_define_test {
    .name = "example.messages.failf",
    .start = test_failf,
    .no_image = true,
};
