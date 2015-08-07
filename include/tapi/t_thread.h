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
/// \brief Test API for managing threads

#pragma once

#include <stdnoreturn.h>

/// \brief Start a new test thread.
///
/// The new thread may be a newly created thread or may be an existing thread
/// resused from the framework's thread pool.  When the thread returns from its
/// start function, it automatically calls t_thread_release().
void t_thread_start(void (*start)(void *arg), void *arg);

/// \brief Release a test thread to the framework.
///
/// This call does not return.  On release, the calling thread may immediately
/// exit or may return to the framework's thread pool.
///
/// This call is useful if a thread needs to "exit" early without selecting the
/// test's result, possibly because it defers the result selection to
/// a different thread.
noreturn void t_thread_release(void);

/// \brief Yield control to the framework.
///
/// Before returning, this call may perform a small action on behalf of the
/// framework.  This call does not return if the framework has decided that the
/// test has already completed (for example, because another thread called
/// t_fail()), in which case this call is equivalent to t_thread_release().
void t_thread_yield(void);

/// \brief Reduce the test's thread count to 1.
///
/// This call returns only if the calling thread is the test's sole thread.
void t_thread_sieve(void);
