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

/// \file
/// \brief Test API for setting the result and possibly ending the test.
///
///   - The framework automatically calls t_compare_image() when the
///     test's start function returns, unless it's a no-image test.

#pragma once

#include <stdarg.h>
#include <stdbool.h>

#include "util/macros.h"

typedef enum test_result test_result_t;

enum test_result {
    TEST_RESULT_PASS,
    TEST_RESULT_SKIP,
    TEST_RESULT_FAIL,
};

void
test_result_merge(test_result_t *accum,
                      test_result_t new_result);

void t_end(test_result_t result) cru_noreturn;

void t_pass(void) cru_noreturn;

#define t_skip() __t_skip(__FILE__, __LINE__)
#define t_skipf(format, ...) __t_skipf(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define t_skipvf(format, va) __t_skipfv(__FILE__, __LINE__, format, va)
#define t_skip_silent() __t_skip_silent()

#define t_fail() __t_fail(__FILE__, __LINE__)
#define t_failf(format, ...) __t_failf(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define t_failvf(format, va) __t_failfv(__FILE__, __LINE__, format, va)
#define t_fail_silent() __t_fail_silent()

#define t_assert(cond) __t_assert(__FILE__, __LINE__, (cond), #cond)
#define t_assertf(cond, format, ...) __t_assertf(__FILE__, __LINE__, (cond), #cond, format, ##__VA_ARGS__)
#define t_assertfv(cond, format, va) __t_assertfv(__FILE__, __LINE__, (cond), #cond, format, va)

/// Compare the test's rendered image against its reference image, ending the
/// test.
///
/// \see t_color_image
/// \see t_ref_image
void t_compare_image(void) cru_noreturn;

void __t_skip(const char *file, int line) cru_noreturn;
void __t_skipf(const char *file, int line, const char *format, ...) cru_noreturn cru_printflike(3, 4);
void __t_skipfv(const char *file, int line, const char *format, va_list va) cru_noreturn;
void __t_skip_silent(void) cru_noreturn;

void __t_fail(const char *file, int line) cru_noreturn;
void __t_failf(const char *file, int line, const char *format, ...) cru_noreturn cru_printflike(3, 4);
void __t_failfv(const char *file, int line, const char *format, va_list va) cru_noreturn;
void __t_fail_silent(void) cru_noreturn;

void __t_assert(const char *file, int line, bool cond, const char *cond_string);
void __t_assertf(const char *file, int line, bool cond, const char *cond_string, const char *format, ...) cru_printflike(5, 6);
void __t_assertfv(const char *file, int line, bool cond, const char *cond_string, const char *format, va_list va);
