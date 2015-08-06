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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <crucible/cru_misc.h>
#include <crucible/string.h>

#include "cmd.h"

const cru_command_t *
cru_find_command(const char *name)
{
    const cru_command_t *cmd;

    cru_foreach_command(cmd) {
        if (cru_streq(cmd->name, name)) {
            return cmd;
        }
    }

    return NULL;
}

void cru_noreturn cru_printflike(2, 3)
cru_usage_error(const cru_command_t *cmd, const char *format, ...)
{
    va_list va;

    const char *dash = "";
    const char *cmd_name = "";

    if (cmd) {
        dash = "-";
        cmd_name = cmd->name;
    }

    va_start(va, format);
    fprintf(stderr, "crucible%s%s: usage error: ", dash, cmd_name);
    vfprintf(stderr, format, va);
    fprintf(stderr, "\n");
    va_end(va);

    // Follow git's precedent. It exits with 129 on usage error.
    exit(129);
}


void
cru_pop_argv(int start, int count, int *argc, char **argv)
{
    for (int i = start; i < *argc; ++i) {
        argv[i] = argv[i + count];
    }

   *argc -= count;
}

void cru_noreturn
cru_command_page_help(const cru_command_t *cmd)
{
    int err;

    // Build path to "{prefix}/doc/crucible-{command}.1".
    string_t help_path = STRING_INIT;
    path_append(&help_path, cru_prefix_path());
    path_append_cstr(&help_path, "doc");
    path_append_cstr(&help_path, "crucible-");
    string_append_cstr(&help_path, cmd->name);
    string_append_cstr(&help_path, ".1");

    struct stat help_stat;
    err = stat(string_data(&help_path), &help_stat);
    if (err) {
        int stat_errno = errno;
        switch (stat_errno) {
        case ENOENT:
            cru_loge("no help text for crucible-%s", cmd->name);
            exit(1);
        default:
            cru_loge("failed to read help text at %s",
                     string_data(&help_path));
            cru_loge("%s", strerror(stat_errno));
            exit(1);
        }
    }

    char *man_args[] = {"man", "--local-file", string_data(&help_path), NULL};

    err = execvp(man_args[0], man_args);
    if (err) {
        cru_loge("exec failed: %s %s %s",
                 man_args[0], man_args[1], man_args[2]);
        exit(1);
    }

    cru_unreachable;
}
