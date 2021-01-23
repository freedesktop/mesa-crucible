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

#include <limits.h>

#include <fcntl.h>
#include <unistd.h>

#include "runner.h"
#include "slave.h"

static int dispatch_fd;
static int result_fd;

/// Return NULL if the pipe is empty or has errors.
static void
slave_recv_test(const test_def_t **test_def, uint32_t *queue_num)
{
    dispatch_packet_t pk;

    static_assert(sizeof(pk) <= PIPE_BUF, "dispatch packets will not be read "
                  "and written atomically");

    if (read(dispatch_fd, &pk, sizeof(pk)) != sizeof(pk)) {
        *test_def = NULL;
        return;
    }

    *queue_num = pk.queue_num;
    *test_def = pk.test_def;
}

static bool
slave_send_result(const test_def_t *def, uint32_t queue_num,
                  test_result_t result)
{
    const result_packet_t pk = {
        .test_def = def,
        .queue_num = queue_num,
        .result = result,
    };

    static_assert(sizeof(pk) <= PIPE_BUF, "result packets will not be read "
                  "and written atomically");

    return write(result_fd, &pk, sizeof(pk)) == sizeof(pk);
}

static void
slave_loop(void)
{
    const test_def_t *def;

    for (;;) {
        test_result_t result;
        uint32_t queue_num;

        slave_recv_test(&def, &queue_num);
        if (!def)
            return;

        result = run_test_def(def, queue_num);
        slave_send_result(def, queue_num, result);
    }
}

void
slave_run(int _dispatch_fd, int _result_fd)
{
    assert(_dispatch_fd >= 0);
    assert(_result_fd >= 0);

    dispatch_fd = _dispatch_fd;
    result_fd = _result_fd;

    slave_loop();
}
