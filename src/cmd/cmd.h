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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

#include <getopt.h>

#include "util/log.h"
#include "util/macros.h"

typedef struct cru_command cru_command_t;

/// This structure must be aligned to allow cru_foreach_command() to iterate
/// over the elements in the ELF section.
struct cru_command {
   const char *name;
   int (*start)(const cru_command_t *cmd, int argc, char **argv);
} __attribute__((aligned(16)));

const cru_command_t *cru_find_command(const char *name);
void cru_pop_argv(int start, int count, int *argc, char **argv);
noreturn void cru_usage_error(const cru_command_t *cmd, const char *format, ...) printflike(2, 3);
noreturn void cru_command_page_help(const cru_command_t *cmd);
noreturn void cru_open_crucible_manpage(int volume, const char *suffix);

extern const cru_command_t __start_cru_commands, __stop_cru_commands;

#define cru_define_command \
   static const cru_command_t  CRU_CAT(__cru_local_command, __COUNTER__) \
      __attribute__((section("cru_commands"))) \
      __attribute__((used)) =

#define cru_foreach_command(cmd)                                             \
                                                                              \
   cru_static_assert(                                                        \
      __builtin_types_compatible_p(__typeof__(cmd),                           \
                                   const cru_command_t *));                  \
                                                                              \
   for (cmd = &__start_cru_commands;                                         \
        cmd < &__stop_cru_commands;  ++cmd)
