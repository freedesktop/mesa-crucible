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

#include "runner.h"
#include "slave.h"

static cru_pipe_t *dispatch_pipe;
static cru_pipe_t *result_pipe;

/// Return NULL if the pipe is empty or has errors.
static const test_def_t *
slave_recv_test(void)
{
    dispatch_packet_t pk;

    if (!cru_pipe_atomic_read(dispatch_pipe, &pk))
        return false;

    return pk.test_def;
}

/// Return false on failure.
static bool
slave_send_result(const test_def_t *def, test_result_t result)
{
    const result_packet_t pk = {
        .test_def = def,
        .result = result,
    };

    return cru_pipe_atomic_write(result_pipe, &pk);
}

static void
slave_loop(void)
{
    const test_def_t *def;

    for (;;) {
        test_result_t result;

        def = slave_recv_test();
        if (!def)
            return;

        result = run_test_def(def);
        slave_send_result(def, result);
    }
}

void
slave_run(cru_pipe_t *_dispatch_pipe, cru_pipe_t *_result_pipe)
{
    assert(!dispatch_pipe);
    assert(!result_pipe);

    dispatch_pipe = _dispatch_pipe;
    result_pipe = _result_pipe;

    slave_loop();
}
