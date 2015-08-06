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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <pthread.h>

#include "util/log.h"
#include "util/misc.h"
#include "util/string.h"

static string_t cru_prefix_path_ = STRING_INIT;

void cru_noreturn
cru_oom(void)
{
    fprintf(stderr, "crucible: out of memory\n");
    abort();
}


/// Set \a result and return 0 if and only if the environment variable is
/// unset, the empty string, "0", or "1". Otherwise return -EINVAL and leave
/// \a result unchanged.
cru_err_t
cru_getenv_bool(const char *name, bool default_, bool *result)
{
    const char *env;

    env = getenv(name);
    if (!env || (env[0] == '\0')) {
        *result = default_;
        return 0;
    }

    if (env[0] && env[1]) {
        // string is too long
        return -EINVAL;
    }

    if (!env) {
        *result = default_;
    } else if (env[0] == '\0') {
        *result = default_;
    } else if (env[0] == '0') {
        *result = false;
    } else if (env[0] == '1') {
        *result = true;
    } else {
        return -EINVAL;
    }

    return 0;
}

static void
cru_prefix_setup(void)
{
    static char buf[2048];
    const char *tail = "/bin/crucible";

    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf));
    if (len <= 0) {
        loge("readlink(\"/proc/self/exe\") failed");
        abort();
    } else if (len >= sizeof(buf)) {
        loge("real path of /proc/self/exe is too long");
        abort();
    }

    string_attach(&cru_prefix_path_, buf, len, sizeof(buf));
    if (!string_endswith_cstr(&cru_prefix_path_, tail)) {
        loge("real path of /proc/self/exe doesn't end in %s", tail);
        abort();
    }

    // Transform PREFIX/bin/crucible to PREFIX.
    path_dirname(&cru_prefix_path_);
    path_dirname(&cru_prefix_path_);
}

const string_t *
cru_prefix_path(void)
{
    static pthread_once_t cru_prefix_once = PTHREAD_ONCE_INIT;
    int err;

    err = pthread_once(&cru_prefix_once, cru_prefix_setup);
    if (err)
        abort();

    return &cru_prefix_path_;
}
