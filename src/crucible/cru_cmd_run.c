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

#include <crucible/cru_array.h>

#include "cru_cmd.h"
#include "cru_test.h"

enum {
    // Must be 1 because we rely on '-' in the getopt optstring to detect
    // testnames.
    OPT_TEST = 1,

    OPT_HELP = 2,
};

static int opt_flag;
static int opt_no_cleanup = 0;
static int opt_dump = 0;
static int opt_use_spir_v = 0;
static int opt_all_tests = 1;

// From man:getopt(3) :
//
//    If the first character  of  optstring  is '-', then each nonoption
//    argv-element is handled as if it were the argument of an option with
//    character code 1.  (This is used by programs  that were  written  to
//    expect options  and other argv-elements in any order and that care about
//    the ordering of the two.) The special argument "--" forces an  end  of
//    option- scanning regardless of the scanning mode.
static const char *shortopts = "-:";

static const struct option longopts[] = {
    {"help",          no_argument,       &opt_flag,       OPT_HELP},
    {"no-cleanup",    no_argument,       &opt_no_cleanup, true},
    {"dump",          no_argument,       &opt_dump,       true},
    {"no-dump",       no_argument,       &opt_dump,       false},
    {"use-spir-v",    no_argument,       &opt_use_spir_v, true},
    {0},
};

// Array of `const char*`.
static cru_array_t test_patterns = CRU_ARRAY_INIT;
static cru_array_t test_matches = CRU_ARRAY_INIT;

static _Atomic uint32_t num_pass;
static _Atomic uint32_t num_skip;
static _Atomic uint32_t num_fail;

static void
parse_args(const cru_command_t *cmd, int argc, char **argv)
{
    // Suppress getopt from printing error messages.
    opterr = 0;

    // Reset getopt.
    optind = 0;

    while (true) {
        int optchar;

        opt_flag = 0;
        optchar = getopt_long(argc, argv, shortopts, longopts, NULL);

        switch (optchar) {
        case -1:
            goto done_getopt;
        case 0:
            switch (opt_flag) {
                case OPT_HELP:
                    cru_command_page_help(cmd);
                    exit(0);
                default:
                    break;
            }
            break;
        case OPT_TEST:
            ; // Shut up compiler. Declaration cannot follow label.
            const char *pattern = optarg;
            if (!cru_array_memcpy(&test_patterns, &pattern, sizeof(pattern))) {
                cru_loge("out of memory");
                abort();
            }
            opt_all_tests = false;
            break;
        case ':':
            cru_usage_error(cmd, "%s requires an argument", argv[optind-1]);
            break;
        case '?':
        default:
            cru_usage_error(cmd, "unknown option: %s", argv[optind-1]);
            break;
        }
    }

done_getopt:
    if (optind < argc)
        cru_usage_error(cmd, "trailing arguments");
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
run_single_test(const cru_test_def_t *def)
{
    cru_test_t *test;

    cru_log_tag("start", "%s", def->name);

    test = cru_test_create(def);
    if (!test) {
        report_result(def->name, CRU_TEST_RESULT_FAIL);
        return;
    }

    cru_test_enable_dump(test, opt_dump);

    if (opt_no_cleanup)
        cru_test_disable_cleanup(test);

    if (opt_use_spir_v)
        cru_test_enable_spir_v(test);

    cru_test_start(test);
    cru_test_wait(test);
    report_result(def->name, cru_test_get_result(test));
    cru_test_destroy(test);
}

static void
run_all_tests(void)
{
    const cru_test_def_t *def;

    cru_foreach_test_def(def) {
        if (!cru_test_def_match(def, "example.*")) {
          run_single_test(def);
        }
    }
}

/// Return number of tests.
static uint32_t
collect_tests(void)
{
    uint32_t num_tests = 0;
    const cru_test_def_t *def;

    if (opt_all_tests) {
        cru_foreach_test_def(def) {
            if (!cru_test_def_match(def, "example.*")) {
                ++num_tests;
            }
        }
    } else {
        cru_foreach_test_def(def) {
            const char **pattern;
            cru_array_foreach(pattern, &test_patterns) {
                if (cru_test_def_match(def, *pattern)) {
                    if (!cru_array_memcpy(&test_matches, &def, sizeof(def))) {
                        cru_loge("out of memory");
                        abort();
                    }

                    ++num_tests;
                    break;
                }
            }
        }
    }

    return num_tests;
}

static void
run_matching_tests(void)
{
    const cru_test_def_t **def;

    cru_array_foreach(def, &test_matches)
        run_single_test(*def);
}

static int
cmd_start(const cru_command_t *cmd, int argc, char **argv)
{
    uint32_t num_tests;
    uint32_t num_missing;
    const char *tests_str;

    parse_args(cmd, argc, argv);
    num_tests = collect_tests();

    if (num_tests == 1)
        tests_str = "test";
    else
        tests_str = "tests";

    cru_log_align_tags(true);
    cru_logi("running %u %s", num_tests, tests_str);

    if (opt_all_tests)
        run_all_tests();
    else
        run_matching_tests();

    cru_logi("ran %u %s", num_tests, tests_str);
    cru_logi("pass %u", num_pass);
    cru_logi("fail %u", num_fail);
    cru_logi("skip %u", num_skip);

    num_missing = num_tests - (num_pass + num_fail + num_skip);
    if (num_missing > 0)
        cru_logi("missing %u", num_missing);

    if (num_pass == 0 || num_fail > 0 || num_missing > 0) {
        exit(1);
    } else {
        exit(0);
    }
}

cru_define_command {
    .name = "run",
    .start = cmd_start,
};
