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

#include <crucible/xalloc.h>

#include "cru_cmd.h"

enum {
    OPT_VERSION = 1,
    OPT_HELP = 'h',
};

static int opt_flag;
static const char *opt_command;

// From man:getopt(3) :
//    By default, getopt() permutes the contents of argv as it scans, so that
//    eventually all the nonoptions are at the end. Two other modes are also
//    implemented. If the first character of optstring is '+' or the environ‐
//    ment  variable  POSIXLY_CORRECT is set, then option processing stops as
//    soon as a nonoption argument is encountered.
//
//    If the first character (following any optional '+' or '-' described
//    above) of optstring is a colon (':'),  then getopt() returns ':' instead
//    of '?' to indicate a missing option argument.
//
static const char *shortopts = "+:h";

static const struct option longopts[] = {
    {"help",    no_argument, &opt_flag, OPT_HELP},
    {"version", no_argument, &opt_flag, OPT_VERSION},
    {0},
};

static void
parse_args(int *argc, char **argv)
{
    // Suppress getopt from printing error messages.
    opterr = 0;

    while (true) {
        int optchar = getopt_long(*argc, argv, shortopts, longopts, NULL);

        switch (optchar) {
        case -1:
            goto getopt_done;
        case 0:
            switch (opt_flag) {
            case OPT_HELP:
                opt_command = "help";
                goto getopt_done;
            case OPT_VERSION:
                opt_command = "version";
                goto getopt_done;
            default:
                break;
            }
            break;
        case ':':
            cru_usage_error(NULL, "%s lacks required argument", argv[optind-1]);
            break;
        case '?':
        default:
            cru_usage_error(NULL, "unknown option %s", argv[optind-1]);
            break;
        }
    }

getopt_done:
    cru_pop_argv(1, optind - 1, argc, argv);

    if (!opt_command) {
        if (*argc > 1) {
            opt_command = argv[1];
            cru_pop_argv(1, 1, argc, argv);
        } else {
            opt_command = "help";
        }
    }
}

int
main(int argc, char **argv)
{
    const cru_command_t *cmd = NULL;

    parse_args(&argc, argv);
    assert(opt_command);

    cmd = cru_find_command(opt_command);
    if (!cmd)
        cru_usage_error(NULL, "unknown command: %s", opt_command);

    // Replace argv[0] with "crucible-${cmd_name}".
    argv[0] = xmalloc(strlen("crucible-") + strlen(cmd->name) + 1);
    sprintf(argv[0], "crucible-%s", cmd->name);

    return cmd->start(cmd, argc, argv);
}
