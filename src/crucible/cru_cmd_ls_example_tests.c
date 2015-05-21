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

#include "cru_test.h"
#include "cru_cmd.h"

enum {
    OPT_HELP = 1,
};

static int opt_flag;

static const char *shortopts = "";

static const struct option longopts[] = {
    {"help", no_argument, &opt_flag, OPT_HELP},
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
        case 0:
            switch (opt_flag) {
            case OPT_HELP:
                cru_command_page_help(cmd);
                exit(0);
            default:
                cru_unreachable;
            }
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

static int
start(const cru_command_t *cmd, int argc, char **argv)
{
    const cru_test_def_t *def;

    parse_args(cmd, argc, argv);

    cru_foreach_test_def(def) {
        if (cru_test_def_match(def, "example.*")) {
            printf("%s\n", def->name);
        }
    }

    return 0;
}

cru_define_command {
    .name = "ls-example-tests",
    .start = start,
};
