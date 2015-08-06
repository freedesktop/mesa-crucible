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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/test/test.h"
#include "qonos/qonos.h"
#include "tapi/t.h"
#include "util/cru_format.h"
#include "util/cru_image.h"
#include "util/log.h"
#include "util/cru_refcount.h"
#include "util/cru_slist.h"
#include "util/misc.h"
#include "util/string.h"
#include "util/xalloc.h"

typedef struct cru_current_test cru_current_test_t;
typedef struct user_thread_arg user_thread_arg_t;

/// Tests proceed through the stages in the order listed.
enum test_phase {
    CRU_TEST_PHASE_PRESTART,
    CRU_TEST_PHASE_SETUP,
    CRU_TEST_PHASE_MAIN,
    CRU_TEST_PHASE_PENDING_CLEANUP,
    CRU_TEST_PHASE_CLEANUP,
    CRU_TEST_PHASE_STOPPED,
};

struct cru_current_test {
    test_t *test;
    cru_cleanup_stack_t *cleanup;
};

struct user_thread_arg {
    test_t *test;
    void (*start_func)(void *start_arg);
    void *start_arg;
};

struct test {
    const test_def_t *def;
    cru_slist_t *threads; ///< List of `pthread_t *`.

    /// \brief List of cleanup stacks, one for each test thread.
    ///
    /// The list's element type is `cru_cleanup_stack_t *`.
    ///
    /// When each test thread is created, a new thread-local cleanup stack,
    /// \ref cru_current_test::cleanup, is assigned to it. During
    /// CRU_TEST_PHASE_CLEANUP, all cleanup stacks are unwound.
    ///
    /// CAUTION: During CRU_TEST_PHASE_CLEANUP the the test is, by intentional
    /// design, current in no thread.  As a consequence, during
    /// CRU_TEST_PHASE_CLEANUP it is illegal to call functions whose names
    /// begins with "t_".
    cru_slist_t *cleanup_stacks;

    /// This remains zero until a thread promotes itself to become the result
    /// thread. It remains set until test_destroy().
    pthread_t result_thread;

    /// This remains zero until the cleanup thread is created. It remains set
    /// until test_destroy().
    pthread_t cleanup_thread;

    /// Threads coordinate activity with the phase.
    _Atomic enum test_phase phase;

    enum test_result result;

    /// The test broadcasts this condition when it enters
    /// CRU_TEST_PHASE_STOPPED.
    pthread_cond_t stop_cond;

    /// Protects test::stop_cond.
    pthread_mutex_t stop_mutex;

    /// \brief Options that control the test's behavior.
    ///
    /// These must be set, if at all, before the test starts.
    struct test_options {
        /// Run the test in bootstrap mode.
        bool bootstrap;

        /// Disable image dumps.
        ///
        /// \see t_dump_image()
        bool no_dump;

        /// Don't run the cleanup commands in test::cleanup_stacks.
        bool no_cleanup;

        /// Try and use SPIR-V shaders when available
        bool use_spir_v;

        /// If set, the test's cleanup stacks will unwind in the result
        /// thread. If unset, the result thread will create a separate cleanup
        /// thread.
        bool no_separate_cleanup_thread;
    } opt;

    /// Atomic counter for t_dump_seq_image().
    cru_refcount_t dump_seq;

    /// Reference image
    struct {
        string_t filename;
        cru_image_t *image;
        uint32_t width;
        uint32_t height;
    } ref;

    VkInstance instance;
    VkPhysicalDevice physical_dev;
    VkPhysicalDeviceMemoryProperties physical_dev_mem_props;
    VkDevice device;
    VkQueue queue;
    VkCmdPool cmd_pool;
    VkCmdBuffer cmd_buffer;
    VkDynamicViewportState dynamic_vp_state;
    VkDynamicRasterState dynamic_rs_state;
    VkDynamicColorBlendState dynamic_cb_state;
    VkDynamicDepthStencilState dynamic_ds_state;
    VkImage rt_image;
    VkAttachmentView color_attachment_view;
    VkImageView color_texture_view;
    VkImage ds_image;
    VkAttachmentView ds_attachment_view;
    VkImageView depth_image_view;
    VkFramebuffer framebuffer;
    VkPipelineCache pipeline_cache;

    uint32_t mem_type_index_for_mmap;
    uint32_t mem_type_index_for_device_access;
};

extern __thread cru_current_test_t current
    __attribute__((tls_model("local-exec")));

#define GET_CURRENT_TEST(__var) \
    ASSERT_IN_TEST_THREAD; \
    test_t *__var = current.test

#define ASSERT_IN_TEST_THREAD \
    do { \
        assert(current.test != NULL); \
        assert(current.cleanup != NULL); \
    } while (0)

#define ASSERT_NOT_IN_TEST_THREAD \
    do { \
        assert(current.test == NULL); \
        assert(current.cleanup == NULL); \
    } while (0)

#define ASSERT_TEST_IN_PRESTART_PHASE(t) \
    do { \
        ASSERT_NOT_IN_TEST_THREAD; \
        assert(t->phase == CRU_TEST_PHASE_PRESTART); \
    } while (0)

#define ASSERT_TEST_IN_SETUP_PHASE \
    do { \
        GET_CURRENT_TEST(t); \
        assert(t->phase == CRU_TEST_PHASE_SETUP); \
    } while (0)

#define ASSERT_TEST_IN_MAJOR_PHASE \
    do { \
        GET_CURRENT_TEST(t); \
        assert(t->phase >= CRU_TEST_PHASE_SETUP); \
        assert(t->phase <= CRU_TEST_PHASE_PENDING_CLEANUP); \
    } while (0)

#define ASSERT_TEST_IN_CLEANUP_PHASE(t) \
    do { \
        assert(t->phase == CRU_TEST_PHASE_CLEANUP); \
    } while(0)

#define ASSERT_TEST_IN_STOPPED_PHASE(t) \
    do { \
        assert(t->phase == CRU_TEST_PHASE_STOPPED); \
    } while(0)

#define ASSERT_IN_RESULT_THREAD(t) \
    do { \
        ASSERT_NOT_IN_TEST_THREAD; \
        assert((t)->phase == CRU_TEST_PHASE_PENDING_CLEANUP); \
        assert(pthread_equal(pthread_self(), (t)->result_thread)); \
        assert(!pthread_equal(pthread_self(), (t)->cleanup_thread)); \
    } while (0)

#define ASSERT_IN_CLEANUP_THREAD(t) \
    do { \
        ASSERT_NOT_IN_TEST_THREAD; \
        assert((t)->phase == CRU_TEST_PHASE_CLEANUP); \
        assert(pthread_equal(pthread_self(), (t)->cleanup_thread)); \
        assert(pthread_equal(pthread_self(), (t)->result_thread) == \
               t->opt.no_separate_cleanup_thread); \
    } while (0)
