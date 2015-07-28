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

#include <stdint.h>

#include <crucible/cru_log.h>

#include "cru_runner.h"
#include "cru_test.h"

bool cru_runner_do_cleanup_phase = true;
bool cru_runner_do_image_dumps = false;
bool cru_runner_use_spir_v = false;

static uint32_t num_tests = 0;
static uint32_t num_pass = 0;
static uint32_t num_fail = 0;
static uint32_t num_skip = 0;

static inline void
mark_test_def(cru_test_def_t *def)
{
    if (!def->priv.run)
        ++num_tests;

    def->priv.run = true;
}

void
cru_runner_mark_all_nonexample_tests(void)
{
    cru_test_def_t *def;

    cru_foreach_test_def(def) {
        if (!cru_test_def_match(def, "example.*")) {
            mark_test_def(def);
        }
    }
}

void
cru_runner_mark_matching_tests(const cru_cstr_vec_t *testname_globs)
{
    cru_test_def_t *def;
    char **glob;

    cru_foreach_test_def(def) {
        cru_vec_foreach(glob, testname_globs) {
            if (cru_test_def_match(def, *glob)) {
                mark_test_def(def);
            }
        }
    }
}

static void
report_result(const const char *name, cru_test_result_t result)
{
    switch (result) {
        case CRU_TEST_RESULT_PASS: num_pass++; break;
        case CRU_TEST_RESULT_FAIL: num_fail++; break;
        case CRU_TEST_RESULT_SKIP: num_skip++; break;
    }

    cru_log_tag(cru_test_result_to_string(result), "%s", name);
}

static void
run_one_test(const cru_test_def_t *def)
{
    cru_test_t *test;

    if (!def->priv.run)
        return;

    cru_log_tag("start", "%s", def->name);

    test = cru_test_create(def);
    if (!test) {
        report_result(def->name, CRU_TEST_RESULT_FAIL);
        return;
    }

    if (cru_runner_do_image_dumps)
        cru_test_enable_dump(test);

    if (!cru_runner_do_cleanup_phase)
        cru_test_disable_cleanup(test);

    if (cru_runner_use_spir_v)
        cru_test_enable_spir_v(test);

    cru_test_start(test);
    cru_test_wait(test);

    report_result(def->name, cru_test_get_result(test));

    cru_test_destroy(test);
}

/// Return true if and only all tests pass or skip.
bool
cru_runner_run_tests(void)
{
    const cru_test_def_t *def;

    cru_log_align_tags(true);
    cru_logi("running %u tests", num_tests);
    cru_logi("================================");

    cru_foreach_test_def(def) {
        run_one_test(def);
    }

    const uint32_t num_ran = num_pass + num_fail + num_skip;
    const uint32_t num_missing = num_tests - num_ran;

    cru_logi("================================");
    cru_logi("ran %u tests", num_ran);
    cru_logi("pass %u", num_pass);
    cru_logi("fail %u", num_fail);
    cru_logi("skip %u", num_skip);

    if (num_missing > 0)
        cru_logi("missing %u", num_missing);

    return num_pass + num_skip == num_tests;
}
