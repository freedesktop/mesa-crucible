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

#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/string.h"
#include "util/misc.h"

#include "cmd.h"

enum {
    OPT_HELP = 1,
};

static int opt_flag;
static const char *opt_command_name;

static const char *shortopts = "";

static const struct option longopts[] = {
    {"help", no_argument, &opt_flag, OPT_HELP},
    {0},
};

static void
print_main_help(void)
{
    const cru_command_t *cmd;

    printf(
        "NAME\n"
        "  crucible - a testsuite for Vulkan\n"
        "\n"
        "SYNOPSIS\n"
        "  crucible [--version] [--help] <command> [<args>]\n"
        "\n"
        "COMMANDS\n");

    cru_foreach_command(cmd) {
        printf("  %s\n", cmd->name);
    }
}

static void
print_command_help(const char *cmd_name)
{
    const cru_command_t *cmd;

    cmd = cru_find_command(cmd_name);
    if (!cmd) {
        cru_loge("failed to find command '%s'", cmd_name);
        exit(1);
    }

    cru_command_page_help(cmd);
}

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
                break;
            default:
                cru_unreachable;
                break;
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
    if (optind == argc)
        return;

    opt_command_name = argv[optind++];

    if (optind < argc)
        cru_usage_error(cmd, "trailing arguments");
}

static int
start(const cru_command_t *cmd, int argc, char **argv)
{
    parse_args(cmd, argc, argv);

    if (opt_command_name)
        print_command_help(opt_command_name);
    else
        print_main_help();

    return 0;
}

cru_define_command {
    .name = "help",
    .start = start,
};
