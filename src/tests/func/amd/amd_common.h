// Copyright 2018 Google LLC
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

#ifndef AMD_COMMON_H
#define AMD_COMMON_H

#include <math.h>

VkDeviceMemory
common_init(VkShaderModule cs, const uint32_t ssbo_size);

void
dispatch_and_wait(uint32_t x, uint32_t y, uint32_t z);

#define RUN_CASES_CMP(type, printf_identifier, cmp) {                                     \
    const unsigned case_count = sizeof(cases) / sizeof(cases[0]);                         \
    const uint32_t ssbo_size = 16 * case_count;                                           \
    VkDeviceMemory mem = common_init(cs, ssbo_size);                                      \
    type *map = qoMapMemory(t_device, mem, 0, ssbo_size, 0);                              \
                                                                                          \
    for (unsigned i = 0; i < case_count; ++i) {                                           \
        for (unsigned j = 0; j < 3; ++j)                                                  \
            map[4 * i+ j] = cases[i].params[j];                                           \
        map[4 * i + 3] = 4356; /* Something to prevent the initial value form matching */ \
    }                                                                                     \
    dispatch_and_wait(case_count, 1, 1);                                                  \
                                                                                          \
    for (unsigned i = 0; i < case_count; i++) {                                           \
        t_assertf((cmp(map[4 * i + 3], cases[i].result)),                                 \
                  "buffer mismatch at case %d: found " printf_identifier ", "             \
                  "expected " printf_identifier, i, map[4 * i + 3], cases[i].result);     \
    }                                                                                     \
    t_pass();                                                                             \
}

#define CMP_EXACT(a, b) a == b
#define RUN_CASES(type, printf_identifier)                                                \
    RUN_CASES_CMP(type, printf_identifier, CMP_EXACT)

#define CMP_CLOSE(a, b) (isnan(a) && isnan(b)) || fabs(a - b) < 0.001
#define RUN_CASES_CLOSE(type, printf_identifier)                                          \
    RUN_CASES_CMP(type, printf_identifier, CMP_CLOSE)


#endif	// AMD_COMMON_H
