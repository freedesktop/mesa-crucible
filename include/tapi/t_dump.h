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
/// \brief Test API for dumping images to disk

#pragma once

#include <stdbool.h>

#include "util/cru_image.h"
#include "util/macros.h"

/// Is image dumping enabled on the cmdline?
bool t_is_dump_enabled(void);

/// Dump images to sequentially named files.
///
/// Each call dumps the image to a sequentially named file. The filename is
/// prefixed by the testname. and the sequence resets to 0 each time the test
/// is run. In other words, differently named tests will not clobber each
/// other's files. But if you run a test twice, the second run will clobber any
/// the files dumped by the first run.
///
/// This is useful for tests that produce multiple images that all need
/// inspection.
///
/// This function is a no-op if image dumping is disabled on the cmdline.
void t_dump_seq_image(cru_image_t *image);

/// Dump image to file whose filename is suffixed by a formatted string.
///
/// The image's full filename is "{testname}.{format}".
/// This function is a no-op if image dumping is disabled on the cmdline.
void t_dump_image_f(cru_image_t *image, const char *format, ...)
    cru_printflike(2, 3);

/// \copydoc t_image_dump_f()
void t_dump_image_fv(cru_image_t *image, const char *format, va_list va);
