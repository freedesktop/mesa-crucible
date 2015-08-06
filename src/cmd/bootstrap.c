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

#include "cmd.h"
#include "framework/test/test.h"
#include "framework/test/test_def.h"

enum {
    OPT_HELP = 'h',
    OPT_WIDTH,
    OPT_HEIGHT,
};

static int opt_flag;
static const char *opt_test_name;
static uint32_t opt_image_width;
static uint32_t opt_image_height;

// From man:getopt(3) :
//
//    If the first character (following any optional '+' or '-' described
//    above) of optstring is a colon (':'),  then getopt() returns ':' instead
//    of '?' to indicate a missing option argument.
//
static const char *shortopts = ":h";

static const struct option longopts[] = {
    {"help",       no_argument,       NULL,      OPT_HELP},
    {"width",      required_argument, &opt_flag, OPT_WIDTH},
    {"height",     required_argument, &opt_flag, OPT_HEIGHT},
    {0},
};

static void
parse_args(const cru_command_t *cmd, int argc, char **argv)
{
    // Suppress getopt from printing error messages.
    opterr = 0;

    // Reset getopt.
    optind = 0;

    while (true) {
        int optchar = getopt_long(argc, argv, shortopts, longopts, NULL);

        switch (optchar) {
        case -1:
            goto done_getopt;
        case OPT_HELP:
            cru_command_page_help(cmd);
            exit(0);
            break;
        case 0:
            switch (opt_flag) {
            case OPT_WIDTH:
                if (sscanf(optarg, "%u", &opt_image_width) < 1) {
                    cru_usage_error(cmd, "--width requires an uint argument");
                }
                break;
            case OPT_HEIGHT:
                if (sscanf(optarg, "%u", &opt_image_height) < 1) {
                    cru_usage_error(cmd, "--height requires an uint argument");
                }
                break;
            default:
                cru_unreachable;
            }
            break;
        case ':':
            cru_usage_error(cmd, "%s requires an argument", argv[optind-1]);
            break;
        case '?':
        default:
            cru_usage_error(cmd, "uknown option %s", argv[optind-1]);
            break;
        }
    }

done_getopt:
    if (optind == argc)
        cru_usage_error(cmd, "missing test name");

    opt_test_name = argv[optind++];

    if (optind < argc)
        cru_usage_error(cmd, "trailing arguments");
}

static int
start(const cru_command_t *cmd, int argc, char **argv)
{
    const test_def_t *def = NULL;
    test_t *test = NULL;

    parse_args(cmd, argc, argv);
    assert(opt_test_name);

    def = cru_find_def(opt_test_name);
    if (!def) {
        cru_loge("failed to find test: %s", opt_test_name);
        exit(1);
    }

    test = test_create(def);
    if (!test) {
        cru_loge("%s: failed to initialize", def->name);
        exit(1);
    }

    if (!def->no_image) {
        if (opt_image_width == 0) {
            cru_loge("%s: test has image, --width must be non-zero",
                     def->name);
            exit(1);
        }
        if (opt_image_height == 0) {
            cru_loge("%s: test has image, --height must be non-zero",
                     def->name);
            exit(1);
        }
    }

    if (!test_enable_bootstrap(test, opt_image_width, opt_image_height))
        exit(1);

    test_start(test);
    test_wait(test);
    test_destroy(test);

    return 0;
}

cru_define_command {
    .name = "bootstrap",
    .start = start,
};
