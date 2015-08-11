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

#include "framework/runner/runner.h"
#include "framework/test/test.h"
#include "framework/test/test_def.h"

typedef union cru_pipe cru_pipe_t;
typedef struct dispatch_packet dispatch_packet_t;
typedef struct result_packet result_packet_t;

union cru_pipe {
    int fd[2];

    struct {
        int read_fd;
        int write_fd;
    };
};

struct dispatch_packet {
    const test_def_t *test_def;
};

struct result_packet {
    const test_def_t *test_def;
    test_result_t result;
};

bool cru_pipe_init(cru_pipe_t *p);
void cru_pipe_finish(cru_pipe_t *p);
bool cru_pipe_become_reader(cru_pipe_t *p);
bool cru_pipe_become_writer(cru_pipe_t *p);
bool cru_pipe_atomic_write_n(const cru_pipe_t *p, const void *data, size_t n);
bool cru_pipe_atomic_read_n(const cru_pipe_t *p, void *data, size_t n);

#define cru_pipe_atomic_write(p, data) \
    cru_pipe_atomic_write_n((p), (data), sizeof(*(data)))

#define cru_pipe_atomic_read(p, data) \
    cru_pipe_atomic_read_n((p), (data), sizeof(*(data)))

test_result_t run_test_def(const test_def_t *def);
