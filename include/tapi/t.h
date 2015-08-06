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

/// \file
/// \brief Test API
///
/// FINISHME(chadv): Document how tests work, the different test phases, and how
/// some of the more mysterious APIs should be used.
///
/// NOTES:
///   - Functions prefixed with 't_' operate on the thread-local test context
///     and are reentrant.

#pragma once

#include <stdlib.h>
#include <string.h>

#include "qonos/qonos.h"
#include "util/cru_log.h"
#include "util/macros.h"
#include "util/xalloc.h"

#include "t_cleanup.h"
#include "t_data.h"
#include "t_def.h"
#include "t_dump.h"
#include "t_misc.h"
#include "t_result.h"
#include "t_thread.h"
