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

#include "util/misc.h"
#include "util/cru_vec.h"

#include "cmd.h"
#include "framework/runner/runner.h"

static runner_isolation_mode_t opt_isolation = RUNNER_ISOLATION_MODE_PROCESS;
static int opt_fork = 1;
static int opt_no_cleanup = 0;
static int opt_dump = 0;
static int opt_use_spir_v = 0;
static int opt_separate_cleanup_thread = 1;

// From man:getopt(3) :
//
//    By default, getopt() permutes the contents of argv as it scans, so that
//    eventually all the nonoptions are at the end. [...] If the first
//    character of optstring is '+' or the environ‐ ment variable
//    POSIXLY_CORRECT is set, then option processing stops as soon as
//    a nonoption argument is encountered.
//
//    If the first character (following any optional '+' or '-' described
//    above) of optstring is a colon (':'),  then getopt() returns ':' instead
//    of '?' to indicate a missing option argument.
//
static const char *shortopts = "+:hI:";

static const struct option longopts[] = {
    {"help",          no_argument,       NULL,           'h'},
    {"isolation",     required_argument, NULL,           'I'},
    {"fork",          no_argument,       &opt_fork,       true},
    {"no-fork",       no_argument,       &opt_fork,       false},
    {"no-cleanup",    no_argument,       &opt_no_cleanup, true},
    {"dump",          no_argument,       &opt_dump,       true},
    {"no-dump",       no_argument,       &opt_dump,       false},
    {"use-spir-v",    no_argument,       &opt_use_spir_v, true},

    {"separate-cleanup-threads",    no_argument, &opt_separate_cleanup_thread, true},
    {"no-separate-cleanup-threads", no_argument, &opt_separate_cleanup_thread, false},

    {0},
};

static cru_cstr_vec_t test_patterns = CRU_VEC_INIT;

static void
parse_args(const cru_command_t *cmd, int argc, char **argv)
{
    // Suppress getopt from printing error messages.
    opterr = 0;

    // Reset getopt.
    optind = 1;

    while (true) {
        int optchar;

        optchar = getopt_long(argc, argv, shortopts, longopts, NULL);

        switch (optchar) {
        case -1:
            goto done_getopt;
	case 0:
	    break;
        case 'h':
            cru_command_page_help(cmd);
            exit(0);
            break;
        case 'I':
            if (cru_streq(optarg, "p") || cru_streq(optarg, "process")) {
                opt_isolation = RUNNER_ISOLATION_MODE_PROCESS;
            } else if (cru_streq(optarg, "t") || cru_streq(optarg, "thread")) {
                opt_isolation = RUNNER_ISOLATION_MODE_THREAD;
            } else {
                cru_usage_error(cmd, "invalid value '%s' for --isolation",
                                argv[optind-1]);
            }
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
    while (optind < argc) {
        char *arg = argv[optind++];

        if (arg[0] == '-') {
            cru_usage_error(cmd, "option %s follows a non-option",
                            argv[optind-1]);
        }

        *cru_vec_push(&test_patterns, 1) = arg;
    }
}

static int
cmd_start(const cru_command_t *cmd, int argc, char **argv)
{
    parse_args(cmd, argc, argv);

    runner_isolation_mode = opt_isolation;
    runner_do_forking = opt_fork;
    runner_do_cleanup_phase = !opt_no_cleanup;
    runner_use_separate_cleanup_threads = opt_separate_cleanup_thread;
    runner_do_image_dumps = opt_dump;
    runner_use_spir_v = opt_use_spir_v;

    if (test_patterns.len == 0) {
        runner_enable_all_nonexample_tests();
    } else {
        runner_enable_matching_tests(&test_patterns);
    }

    if (runner_run_tests()) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}

cru_define_command {
    .name = "run",
    .start = cmd_start,
};
