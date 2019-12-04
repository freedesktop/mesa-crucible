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
/// \brief Crucible Test Runner
///
/// The runner consists of two processes: master and slave. The master forks
/// the slave. The tests execute in the slave process. The master collects the
/// test results and prints their summary. The separation ensures that test
/// results and summary are printed even when a test crashes its process.

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <regex.h>

#include "util/log.h"

#include "framework/runner/runner.h"
#include "framework/test/test.h"
#include "framework/test/test_def.h"

#include "master.h"
#include "runner.h"

static uint32_t runner_num_tests;
static bool runner_is_init = false;
runner_opts_t runner_opts = {0};

#define ASSERT_RUNNER_IS_INIT \
    do { \
        if (!runner_is_init) { \
            log_internal_error("%s: runner is not initialized", __func__); \
        } \
    } while (0)

bool
runner_init(runner_opts_t *opts)
{
    if (runner_is_init) {
        log_internal_error("cannot initialize runner twice");
        return false;
    }

    if (opts->no_fork
        && opts->isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        log_finishme("support no_fork with RUNNER_ISOLATION_MODE_THREAD");
        return false;
    }

    if (opts->jobs > 1
        && opts->isolation_mode == RUNNER_ISOLATION_MODE_THREAD) {
        log_finishme("support jobs > 1 with RUNNER_ISOLATION_MODE_THREAD");
        return false;
    }

    if (opts->jobs > 1 && opts->no_fork) {
        log_finishme("support jobs > 1 with no_fork");
        return false;
    }

    runner_opts = *opts;
    runner_is_init = true;

    return true;
}

test_result_t
run_test_def(const test_def_t *def, uint32_t queue_family_index)
{
    ASSERT_RUNNER_IS_INIT;

    test_t *test;
    test_result_t result;

    assert(def->priv.enable);

    test = test_create(.def = def,
                       .enable_dump = !runner_opts.no_image_dumps,
                       .enable_cleanup_phase = !runner_opts.no_cleanup_phase,
                       .enable_separate_cleanup_thread =
                            runner_opts.use_separate_cleanup_threads,
                       .device_id = runner_opts.device_id,
                       .queue_family_index = queue_family_index,
                       .verbose = runner_opts.verbose);
    if (!test)
        return TEST_RESULT_FAIL;

    test_start(test);
    test_wait(test);
    result = test_get_result(test);
    test_destroy(test);

    return result;
}

/// Return true if and only all tests pass or skip.
bool
runner_run_tests(void)
{
    ASSERT_RUNNER_IS_INIT;

    return master_run(runner_num_tests);
}

static bool
glob_is_negative(const char *glob)
{
    bool neg = false;

    while (glob && glob[0] == '!') {
        ++glob;
        neg = !neg;
    }

    return neg;
}

typedef struct {
    char *glob;
    bool free_string;
    uint64_t queue_family_index;
} split_glob_t;

typedef struct split_glob_vec split_glob_vec_t;
CRU_VEC_DEFINE(struct split_glob_vec, split_glob_t)

static bool
parse_u32(const char *str, uint32_t *u32)
{
    char *endptr;
    unsigned long l;

    if (str[0] == 0)
        return false;

    l = strtoul(str, &endptr, 10);
    if (endptr[0] != 0) {
        // Entire string was not parsed.
        return false;
    } else if (l < 0 || l > UINT32_MAX) {
        return false;
    }

    *u32 = l;
    return true;
}

void
runner_enable_matching_tests(const cru_cstr_vec_t *testname_globs)
{
    ASSERT_RUNNER_IS_INIT;

    regex_t device_suffix_re;
    if (regcomp(&device_suffix_re, "\\.q[0-9]+$", REG_EXTENDED) != 0)
        abort();

    test_def_t *def;
    char **glob;

    cru_foreach_test_def(def) {
        def->priv.queue_family_index = NO_QUEUE_FAMILY_INDEX_PREF;
    }

    split_glob_vec_t split_globs = CRU_VEC_INIT;
    cru_vec_foreach(glob, testname_globs) {
        split_glob_t *split_glob = cru_vec_push(&split_globs, 1);
        regmatch_t match;
        if (regexec(&device_suffix_re, *glob, 1, &match, 0) == 0) {
            split_glob->glob = strdup(*glob);
            split_glob->glob[match.rm_so] = '\0';
            split_glob->free_string = true;
            char *queue_family_index_str = &split_glob->glob[match.rm_so + 2];
            uint32_t queue_family_index;
            if (parse_u32(queue_family_index_str, &queue_family_index)) {
                split_glob->queue_family_index = queue_family_index;
            } else {
                split_glob->queue_family_index = INVALID_QUEUE_FAMILY_INDEX_PREF;
            }
        } else {
            split_glob->glob = *glob;
            split_glob->free_string = false;
            split_glob->queue_family_index = NO_QUEUE_FAMILY_INDEX_PREF;
        }
    }

    const bool first_glob_is_neg = testname_globs->len > 0 &&
                                   glob_is_negative(testname_globs->data[0]);

    const bool implicit_all = testname_globs->len == 0 || first_glob_is_neg;

    split_glob_t *split_glob;
    cru_foreach_test_def(def) {
        bool enable = false;

        if (implicit_all) {
            enable = test_def_match(def, "*");
        }

        // Last matching glob wins.
        cru_vec_foreach(split_glob, &split_globs) {
            if (test_def_match(def, split_glob->glob)) {
                enable =
                    (split_glob->queue_family_index !=
                     INVALID_QUEUE_FAMILY_INDEX_PREF) &&
                    !glob_is_negative(split_glob->glob);
                def->priv.queue_family_index = split_glob->queue_family_index;
            }
        }

        if (enable) {
            def->priv.enable = true;
            ++runner_num_tests;
        }
    }

    cru_vec_foreach(split_glob, &split_globs) {
        if (split_glob->free_string)
            free(split_glob->glob);
    }
    cru_vec_finish(&split_globs);
    regfree(&device_suffix_re);
}
