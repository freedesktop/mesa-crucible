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
#include <stdatomic.h>
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

typedef enum test_phase test_phase_t;
typedef struct cru_current_test cru_current_test_t;
typedef struct test test_t;
typedef struct test_thread_arg test_thread_arg_t;

/// Tests proceed through the stages in the order listed.
enum test_phase {
    TEST_PHASE_PRESTART,
    TEST_PHASE_SETUP,
    TEST_PHASE_MAIN,
    TEST_PHASE_PRECLEANUP,
    TEST_PHASE_CLEANUP,
    TEST_PHASE_STOPPED,
};

struct cru_current_test {
    test_t *test;
    cru_cleanup_stack_t *cleanup;
};

struct test_thread_arg {
    test_t *test;
    void (*start_func)(void *start_arg);
    void *start_arg;
};

struct test {
    const test_def_t *def;

    atomic_uint num_threads;

    /// \brief List of cleanup stacks, one for each test thread.
    ///
    /// The list's element type is `cru_cleanup_stack_t *`.
    ///
    /// When each test thread is created, a new thread-local cleanup stack,
    /// \ref cru_current_test::cleanup, is assigned to it. During
    /// TEST_PHASE_CLEANUP, all cleanup stacks are unwound.
    ///
    /// CAUTION: During TEST_PHASE_CLEANUP the the test is, by intentional
    /// design, current in no thread.  As a consequence, during
    /// TEST_PHASE_CLEANUP it is illegal to call functions whose names
    /// begins with "t_".
    cru_slist_t *cleanup_stacks;

    /// Threads coordinate activity with the phase.
    _Atomic test_phase_t phase;

    test_result_t result;
    atomic_bool result_is_final;

    /// The test broadcasts this condition when it enters
    /// TEST_PHASE_STOPPED.
    pthread_cond_t stop_cond;

    /// Protects cru_test::stop_cond.
    pthread_mutex_t stop_mutex;

    /// \brief Options that control the test's behavior.
    ///
    /// These must be set, if at all, before the test starts.
    struct cru_test_options {
        /// Run the test in bootstrap mode.
        bool bootstrap;

        /// Disable image dumps.
        ///
        /// \see t_dump_image()
        bool no_dump;

        /// Don't run the cleanup commands in cru_test::cleanup_stacks.
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
        uint32_t width;
        uint32_t height;

        string_t filename;
        cru_image_t *image;

        string_t stencil_filename;
        cru_image_t *stencil_image;
    } ref;

    /// Vulkan data
    struct {
        VkInstance instance;
        uint32_t instance_extension_count;
        VkExtensionProperties *instance_extension_props;
        VkPhysicalDevice physical_dev;
        VkPhysicalDeviceProperties physical_dev_props;
        VkPhysicalDeviceMemoryProperties physical_dev_mem_props;
        VkDevice device;
        uint32_t device_extension_count;
        VkExtensionProperties *device_extension_props;
        uint32_t queue_family_count;
        VkQueueFamilyProperties *queue_family_props;
        VkQueue *queue;
        VkDescriptorPool descriptor_pool;
        VkPipelineCache pipeline_cache;
        VkCommandPool *cmd_pool;
        VkCommandBuffer cmd_buffer;
        VkRenderPass render_pass;
        VkFramebuffer framebuffer;

        VkImage color_image;
        VkImageView color_image_view;

        VkImage ds_image;
        VkImageView depthstencil_image_view;

        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
        VkDebugReportCallbackEXT debug_callback;
    } vk;
};

void test_broadcast_stop(test_t *t);
void t_compare_image(void);

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
        assert(t->phase == TEST_PHASE_PRESTART); \
    } while (0)

#define ASSERT_TEST_IN_SETUP_PHASE \
    do { \
        GET_CURRENT_TEST(t); \
        assert(t->phase == TEST_PHASE_SETUP); \
    } while (0)

#define ASSERT_TEST_IN_CLEANUP_PHASE(t) \
    do { \
        assert(t->phase == TEST_PHASE_CLEANUP); \
    } while(0)

#define ASSERT_TEST_IN_STOPPED_PHASE(t) \
    do { \
        assert(t->phase == TEST_PHASE_STOPPED); \
    } while(0)

#define ASSERT_TEST_IN_MAJOR_PHASE \
    do { \
        GET_CURRENT_TEST(t); \
        assert(t->phase >= TEST_PHASE_SETUP); \
        assert(t->phase <= TEST_PHASE_CLEANUP); \
    } while (0)
