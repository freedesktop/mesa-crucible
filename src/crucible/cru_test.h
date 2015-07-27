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

#include <crucible/cru_test.h>
#include <crucible/cru_vec.h>

typedef struct cru_test cru_test_t;
typedef struct cru_test_def_vec cru_test_def_vec_t;

CRU_VEC_DEFINE(struct cru_test_def_vec, const struct cru_test_def *);

const char *cru_test_result_to_string(cru_test_result_t result);

bool cru_test_is_current(void);
cru_test_t *cru_test_create(const cru_test_def_t *def);
void cru_test_destroy(cru_test_t *test);
void cru_test_enable_dump(cru_test_t *t);
bool cru_test_enable_bootstrap(cru_test_t *test, uint32_t image_width, uint32_t image_height);
bool cru_test_disable_cleanup(cru_test_t *test);
void cru_test_enable_spir_v(cru_test_t *t);
void cru_test_start(cru_test_t *test);
void cru_test_wait(cru_test_t *test);
cru_test_result_t cru_test_get_result(cru_test_t *test);

extern const cru_test_def_t __start_cru_test_defs, __stop_cru_test_defs;
uint32_t cru_num_defs(void);
const cru_test_def_t *cru_find_def(const char *name);
bool cru_test_def_match(const cru_test_def_t *def, const char *glob);

#define cru_foreach_test_def(def) \
   \
   cru_static_assert( \
      __builtin_types_compatible_p(__typeof__(def), \
                                   const cru_test_def_t *)); \
   \
   for (def = &__start_cru_test_defs; \
        def < &__stop_cru_test_defs; ++def)
