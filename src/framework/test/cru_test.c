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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/test/cru_test.h"
#include "qonos/qonos.h"
#include "util/cru_format.h"
#include "util/cru_image.h"
#include "util/cru_log.h"
#include "util/cru_refcount.h"
#include "util/cru_slist.h"
#include "util/misc.h"
#include "util/string.h"
#include "util/xalloc.h"

#define GET_CURRENT_TEST(__var) \
    ASSERT_IN_TEST_THREAD; \
    cru_test_t *__var = current.test

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

typedef struct cru_current_test cru_current_test_t;
typedef struct user_thread_arg user_thread_arg_t;

struct cru_current_test {
    cru_test_t *test;
    cru_cleanup_stack_t *cleanup;
};

static __thread cru_current_test_t current
    __attribute__((tls_model("local-exec"))) = {0};

/// Tests proceed through the stages in the order listed.
enum cru_test_phase {
    CRU_TEST_PHASE_PRESTART,
    CRU_TEST_PHASE_SETUP,
    CRU_TEST_PHASE_MAIN,
    CRU_TEST_PHASE_PENDING_CLEANUP,
    CRU_TEST_PHASE_CLEANUP,
    CRU_TEST_PHASE_STOPPED,
};

struct cru_test {
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
    /// thread. It remains set until cru_test_destroy().
    pthread_t result_thread;

    /// This remains zero until the cleanup thread is created. It remains set
    /// until cru_test_destroy().
    pthread_t cleanup_thread;

    /// Threads coordinate activity with the phase.
    _Atomic enum cru_test_phase phase;

    enum test_result result;

    /// The test broadcasts this condition when it enters
    /// CRU_TEST_PHASE_STOPPED.
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

    string_t ref_image_filename;
    cru_image_t *image;

    /// Atomic counter for t_dump_seq_image().
    cru_refcount_t dump_seq;

    uint32_t width;
    uint32_t height;
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

struct user_thread_arg {
    cru_test_t *test;
    void (*start_func)(void *start_arg);
    void *start_arg;
};

static bool
cru_test_create_thread(cru_test_t *t, void *(*start)(void *arg), void *arg,
                       pthread_t *out_thread);

static void *cleanup_thread_start(void *arg);
static void cru_noreturn t_thread_exit(void);

static void
join_thread(pthread_t thread)
{
    int err;

    err = pthread_join(thread, NULL);
    if (err) {
        // Abort because there is no safe way to recover.
        cru_loge("failed to join thread... abort!");
        abort();
    }
}

const char *
test_result_to_string(test_result_t result)
{
    switch (result) {
    case TEST_RESULT_PASS:
        return "pass";
    case TEST_RESULT_SKIP:
        return "skip";
    case TEST_RESULT_FAIL:
        return "fail";
    }

    cru_unreachable;
}

bool
cru_test_is_current(void)
{
    return current.test != NULL;
}

static void
cru_test_set_image_filename(cru_test_t *t)
{
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    // Always define the reference image's filename, even when
    // test_def_t::no_image is set. This will be useful for tests that
    // generate their reference images at runtime and wish to dump them to
    // disk.
    assert(t->ref_image_filename.len == 0);

    if (t->def->image_filename) {
        // Test uses a custom filename.
        string_copy_cstr(&t->ref_image_filename, t->def->image_filename);
    } else {
        // Test uses the default filename.
        string_copy_cstr(&t->ref_image_filename, t->def->name);
        string_append_cstr(&t->ref_image_filename, ".ref.png");
    }
}

static void
t_load_image_file(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    if (t->image)
        return;

    assert(!t->def->no_image);
    assert(t->ref_image_filename.len > 0);

    t->image = cru_image_load_file(string_data(&t->ref_image_filename));
    t_assert(t->image);

    t_cleanup_push(t->image);
    t->width = cru_image_get_width(t->image);
    t->height = cru_image_get_height(t->image);

    t_assert(t->width > 0);
    t_assert(t->height > 0);
}

void
cru_test_destroy(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    if (!t)
        return;

    assert(t->phase == CRU_TEST_PHASE_PRESTART ||
           t->phase == CRU_TEST_PHASE_STOPPED);

    // This test must own no running threads:
    //   - In the "prestart" phase, no test threads have been created yet.
    //   - In the "stopped" phase, all test threads have been joined.
    assert(t->threads == NULL);

    pthread_mutex_destroy(&t->stop_mutex);
    pthread_cond_destroy(&t->stop_cond);
    string_finish(&t->ref_image_filename);

    free(t);
}

cru_test_t *
cru_test_create(const test_def_t *def)
{
    ASSERT_NOT_IN_TEST_THREAD;

    cru_test_t *t = NULL;
    int err;

    t = xzalloc(sizeof(*t));
    t->def = def;
    t->phase = CRU_TEST_PHASE_PRESTART;
    t->result = TEST_RESULT_PASS;
    t->ref_image_filename = STRING_INIT;
    t->opt.no_dump = true;
    t->opt.no_cleanup = false;
    t->opt.use_spir_v = false;
    t->opt.no_separate_cleanup_thread = false;

    if (t->def->samples > 0) {
        cru_loge("%s: multisample tests not yet supported", t->def->name);
        goto fail;
    }

    err = pthread_mutex_init(&t->stop_mutex, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized mutex later.
        cru_loge("%s: failed to init mutex during test creation",
                 t->def->name);
        abort();
    }

    err = pthread_cond_init(&t->stop_cond, NULL);
    if (err) {
        // Abort to avoid destroying an uninitialized cond later.
        cru_loge("%s: failed to init thread condition during test creation",
                 t->def->name);
        abort();
    }

    cru_test_set_image_filename(t);

    return t;

fail:
    t->result = CRU_TEST_PHASE_STOPPED;
    t->result = TEST_RESULT_FAIL;
    return t;
}

bool
cru_test_enable_bootstrap(cru_test_t *t,
                          uint32_t image_width, uint32_t image_height)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    if (!t->def->no_image && (image_width == 0 || image_height == 0)) {
        cru_loge("%s: bootstrap image must have non-zero size", t->def->name);
        return false;
    }

    t->opt.bootstrap = true;
    t->opt.no_cleanup = true;
    t->width = image_width;
    t->height = image_height;

    return true;
}

void
cru_test_enable_dump(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.no_dump = false;
}

bool
cru_test_disable_cleanup(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.no_cleanup = true;

    return true;
}

void
cru_test_enable_spir_v(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.use_spir_v = true;
}

void
cru_test_disable_separate_cleanup_thread(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->opt.no_separate_cleanup_thread = true;
}

const VkInstance *
__t_instance(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->instance;
}

const VkDevice *
__t_device(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->device;
}

const VkPhysicalDevice *
__t_physical_dev(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->physical_dev;
}

const VkPhysicalDeviceMemoryProperties *
__t_physical_dev_mem_props(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->physical_dev_mem_props;
}

const uint32_t
__t_mem_type_index_for_mmap(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->mem_type_index_for_mmap;
}

const uint32_t
__t_mem_type_index_for_device_access(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->mem_type_index_for_device_access;
}

const VkQueue *
__t_queue(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->queue;
}

const VkCmdPool *
__t_cmd_pool(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->cmd_pool;
}

const VkCmdBuffer *
__t_cmd_buffer(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->cmd_buffer;
}

const VkDynamicViewportState *
__t_dynamic_vp_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->dynamic_vp_state;
}

const VkDynamicRasterState *
__t_dynamic_rs_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->dynamic_rs_state;
}

const VkDynamicColorBlendState *
__t_dynamic_cb_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->dynamic_cb_state;
}

const VkDynamicDepthStencilState *
__t_dynamic_ds_state(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->dynamic_ds_state;
}

const VkImage *
__t_color_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->rt_image;
}

const VkAttachmentView *
__t_color_attachment_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->color_attachment_view;
}

const VkImageView *
__t_color_image_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->color_texture_view;
}

const VkImage *
__t_ds_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->ds_image.handle);

    return &t->ds_image;
}

const VkAttachmentView *
__t_ds_attachment_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->ds_attachment_view.handle);

    return &t->ds_attachment_view;
}

const VkImageView *
__t_depth_image_view(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);
    t_assert(t->depth_image_view.handle);

    return &t->depth_image_view;
}

const VkFramebuffer *
__t_framebuffer(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->framebuffer;
}

const VkPipelineCache *
__t_pipeline_cache(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->pipeline_cache;
}

const uint32_t *
__t_height(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->height;
}

const uint32_t *
__t_width(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return &t->width;
}

const bool *
__t_use_spir_v(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return &t->opt.use_spir_v;
}

const char *
__t_name(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->def->name;
}

const void *
__t_user_data(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return t->def->user_data;
}

cru_image_t *
t_ref_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_assert(!t->def->no_image);

    return t->image;
}

/// Illegal to call before cru_test_wait().
test_result_t
cru_test_get_result(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_STOPPED_PHASE(t);

    return t->result;
}

void
t_check_cancelled(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (t->phase >= CRU_TEST_PHASE_PENDING_CLEANUP)
        t_thread_exit();
}

static void
result_thread_join_others(cru_test_t *t)
{
    ASSERT_IN_RESULT_THREAD(t);

    pthread_t *thread = NULL;

    while ((thread = cru_slist_pop(&t->threads))) {
        if (!pthread_equal(*thread, t->result_thread)) {
            join_thread(*thread);
        }

        free(thread);
    }
}

static void
cru_test_stop(cru_test_t *t)
{
    assert(t->phase < CRU_TEST_PHASE_STOPPED);
    t->phase = CRU_TEST_PHASE_STOPPED;
    pthread_cond_broadcast(&t->stop_cond);
}

static void cru_noreturn
result_thread_enter_cleanup_phase(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_IN_RESULT_THREAD(t);

    int err;

    t->phase = CRU_TEST_PHASE_CLEANUP;

    if (t->opt.no_separate_cleanup_thread) {
        t->cleanup_thread = pthread_self();
        cleanup_thread_start(t);
    } else {
        err = pthread_create(&t->cleanup_thread, NULL, cleanup_thread_start, t);
        if (err) {
            cru_loge("%s: failed to start cleanup thread", t->def->name);

            t->result = TEST_RESULT_FAIL;
            cru_test_stop(t);

            t_thread_exit();
        }

        // The cleanup thread is now responsible for broadcasting the test result.
        // Broadcasting in the cleanup thread instead of here, in the result
        // thread, avoids the overhead of an extra context switch and
        // pthread_join().
    }

    t_thread_exit();
}

void cru_noreturn
t_end(enum test_result result)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (atomic_exchange(&t->phase, CRU_TEST_PHASE_PENDING_CLEANUP)
        >= CRU_TEST_PHASE_PENDING_CLEANUP) {
        // A previous call to cru_test_end already cancelled the test and set
        // the test result.
        t_thread_exit();
    }

    // This thread wins! It now unbinds itself from the test and becomes the
    // "result" thread.
    ASSERT_IN_TEST_THREAD;
    current = (cru_current_test_t) {0};
    t->result = result;
    t->result_thread = pthread_self();
    t->phase = CRU_TEST_PHASE_PENDING_CLEANUP;
    ASSERT_NOT_IN_TEST_THREAD;

    result_thread_join_others(t);

    // This thread, the "result" thread, is the test's sole remaining thread.
    assert(cru_slist_length(t->threads) == 0);

    result_thread_enter_cleanup_phase(t);
}

bool
t_is_dump_enabled(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    return !t->opt.no_dump;
}

void
t_dump_seq_image(cru_image_t *image)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (t->opt.no_dump)
        return;

    uint64_t seq = cru_refcount_get(&t->dump_seq);
    t_assertf(seq <= 9999, "image sequence %lu exceeds 9999", seq);

    string_t filename = STRING_INIT;
    string_printf(&filename, "%s.seq%04lu.png", t_name, seq);
    cru_image_write_file(image, string_data(&filename));
}

void cru_printflike(2, 3)
t_dump_image_f(cru_image_t *image, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    t_dump_image_fv(image, format, va);
    va_end(va);
}

void
t_dump_image_fv(cru_image_t *image, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    if (t->opt.no_dump)
        return;

    string_t filename = STRING_INIT;
    string_appendf(&filename, t_name);
    string_append_char(&filename, '.');
    string_vappendf(&filename, format, va);

    cru_image_write_file(image, string_data(&filename));
}

const cru_format_info_t *
t_format_info(VkFormat format)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    const cru_format_info_t *info;

    info = cru_format_get_info(format);
    t_assertf(info, "failed to find cru_format_info for VkFormat %d", format);

    return info;
}

void
t_compare_image(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    t_check_cancelled();

    // Fail if the user accidentially tries to check the image in a non-image
    // test.
    t_assert(!t->def->no_image);

    assert(t->width > 0);
    assert(t->height > 0);

    size_t buffer_size = 4 * t_width * t_height;

    VkBuffer buffer = qoCreateBuffer(t->device,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DESTINATION_BIT);

    VkDeviceMemory mem = qoAllocBufferMemory(t_device, buffer,
        .memoryTypeIndex = t_mem_type_index_for_mmap);

    void *map = qoMapMemory(t_device, mem, /*offset*/ 0,
                            buffer_size, /*flags*/ 0);

    qoBindBufferMemory(t_device, buffer, mem, /*offset*/ 0);

    VkBufferImageCopy copy = {
        .bufferOffset = 0,
        .imageSubresource = {
            .aspect = VK_IMAGE_ASPECT_COLOR,
            .mipLevel = 0,
            .arraySlice = 0,
        },
        .imageOffset = { .x = 0, .y = 0, .z = 0 },
        .imageExtent = {
            .width = t_width,
            .height = t_height,
            .depth = 1,
        },
    };

    VkCmdBuffer cmd = qoCreateCommandBuffer(t->device, t->cmd_pool);
    qoBeginCommandBuffer(cmd);
    vkCmdCopyImageToBuffer(cmd, t_color_image,
                           VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &copy);
    qoEndCommandBuffer(cmd);
    qoQueueSubmit(t_queue, 1, &cmd, QO_NULL_FENCE);
    vkQueueWaitIdle(t_queue);

    cru_image_t *actual_image =
        cru_image_from_pixels(map, VK_FORMAT_R8G8B8A8_UNORM,
                              t_width, t_height);
    t_assert(actual_image);
    t_cleanup_push(actual_image);

    if (t->opt.bootstrap) {
        assert(!t->image);
        t_assert(cru_image_write_file(actual_image,
                                      string_data(&t->ref_image_filename)));
        t_pass();
    }

    assert(t->image);

    if (!cru_image_compare(actual_image, t_ref_image())) {
        cru_loge("actual and reference images differ");

        // Dump the actual image for inspection.
        //
        // FINISHME: Add a cmdline flag to toggle image dumping.
        // FINISHME: Dump the image diff too.
        string_t actual_path = STRING_INIT;
        string_copy(&actual_path, cru_prefix_path());
        path_append_cstr(&actual_path, "data");
        path_append_cstr(&actual_path, t_name);
        string_append_cstr(&actual_path, ".actual.png");
        cru_image_write_file(actual_image, string_data(&actual_path));

        t_fail_silent();
    }

    t_pass();
}

void cru_noreturn
t_pass(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_PASS);
}

void cru_noreturn
__t_skip(const char *file, int line)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_skipf(file, line, NULL);
}

void cru_noreturn
__t_skipf(const char *file, int line, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_skipfv(file, line, format, va);
    va_end(va);
}

void cru_noreturn
__t_skipfv(const char *file, int line, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_check_cancelled();

    string_t s = STRING_INIT;
    string_appendf(&s, "%s:%d", file, line);

    if (format) {
        string_append_cstr(&s, ": ");
        string_vappendf(&s, format, va);
    }

    cru_logi(string_data(&s));
    string_finish(&s);

    __t_skip_silent();
}

void cru_noreturn
__t_skip_silent(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_SKIP);
}

void cru_noreturn
__t_fail(const char *file, int line)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_failf(file, line, NULL);
}

void cru_noreturn
__t_failf(const char *file, int line, const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_failfv(file, line, format, va);
    va_end(va);
}

void cru_noreturn
__t_failfv(const char *file, int line, const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_check_cancelled();

    string_t s = STRING_INIT;
    string_appendf(&s, "%s:%d", file, line);

    if (format) {
        string_append_cstr(&s, ": ");
        string_vappendf(&s, format, va);
    }

    cru_loge(string_data(&s));
    string_finish(&s);

    __t_fail_silent();
}

void cru_noreturn
__t_fail_silent(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    t_end(TEST_RESULT_FAIL);
}

void
__t_assert(const char *file, int line, bool cond, const char *cond_string)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    __t_assertf(file, line, cond, cond_string, NULL);
}

void cru_printflike(5, 6)
__t_assertf(const char *file, int line, bool cond, const char *cond_string,
            const char *format, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, format);
    __t_assertfv(file, line, cond, cond_string, format, va);
    va_end(va);
}

void
__t_assertfv(const char *file, int line, bool cond, const char *cond_string,
             const char *format, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    // Check for cancellation because cancelled tests should produce no
    // messages.
    t_check_cancelled();

    if (cond)
        return;

    cru_loge("%s:%d: assertion failed: %s", file, line, cond_string);

    if (format) {
        string_t s = STRING_INIT;
        string_appendf(&s, "%s:%d: ", file, line);
        string_vappendf(&s, format, va);
        cru_loge(string_data(&s));
        string_finish(&s);
    }

    t_end(TEST_RESULT_FAIL);
}

/// Find the best VkMemoryType whose properties contain each flag in
/// required_flags and contain no flag in the union of required_flags and
/// allowed_flags.
///
/// On success, return the type's index into the
/// VkPhysicalDeviceMemoryProperties::memoryTypes array.
/// On failure, return UINT32_MAX.
static uint32_t
find_best_mem_type_index(
        const VkPhysicalDeviceMemoryProperties *mem_props,
        VkMemoryPropertyFlags required_flags,
        VkMemoryPropertyFlags allowed_flags)
{
    uint32_t best_type_index = UINT32_MAX;
    VkMemoryHeap best_heap;

    allowed_flags |= required_flags;

    for (uint32_t i = 0; i < mem_props->memoryTypeCount; ++i) {
        VkMemoryType type = mem_props->memoryTypes[i];
        VkMemoryHeap heap = mem_props->memoryHeaps[type.heapIndex];

        if ((type.propertyFlags & required_flags) != required_flags)
            continue;

        if ((type.propertyFlags & ~allowed_flags) != 0)
            continue;

        // Prefer the type with the largest heap.
        if (best_type_index == UINT32_MAX || heap.size > best_heap.size) {
            best_type_index = i;
            best_heap = heap;
        }
    }

    return best_type_index;
}

static void
t_init_physical_dev(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    // Crucible uses only the first physical device.
    // FINISHME: Add a command-line option to use non-default physical device.

    uint32_t count = 0;
    qoEnumeratePhysicalDevices(t_instance, &count, NULL);
    t_assertf(count > 0, "failed to enumerate any physical devices");

    count = 1;
    qoEnumeratePhysicalDevices(t_instance, &count, &t->physical_dev);
    t_assertf(count == 1, "enumerated %u physical devices, expected 1", count);
}

static void
t_init_physical_dev_mem_props(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    qoGetPhysicalDeviceMemoryProperties(t->physical_dev,
                                        &t->physical_dev_mem_props);

    // The Vulkan spec (git aaed022) requires the implementation to expose at
    // least one host-visible and host-coherent memory type.
    t->mem_type_index_for_mmap = find_best_mem_type_index(
        &t->physical_dev_mem_props,
        /*require*/ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        /*allow*/ ~VK_MEMORY_PROPERTY_HOST_NON_COHERENT_BIT);

    // The best memory type for device-access is one which gives the best
    // performance, which is likely one that is device-visible but not
    // host-visible.
    t->mem_type_index_for_device_access = find_best_mem_type_index(
        &t->physical_dev_mem_props,
        /*require*/ VK_MEMORY_PROPERTY_DEVICE_ONLY,
        /*allow*/ VK_MEMORY_PROPERTY_DEVICE_ONLY);

    if (t->mem_type_index_for_device_access == UINT32_MAX) {
        // There exists no device-only memory type. For device-access, then,
        // simply prefer the overall "best" memory type.
        t->mem_type_index_for_device_access = find_best_mem_type_index(
            &t->physical_dev_mem_props,
            /*require*/ 0, /*allow*/ ~0);
    }

    t_assertf(t->mem_type_index_for_mmap != UINT32_MAX,
              "failed to find a host-visible, host-coherent VkMemoryType in "
              "VkPhysicalDeviceMemoryProperties");

    t_assert(t->mem_type_index_for_device_access != UINT32_MAX);
}

static void
thread_bind_to_test(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    cru_cleanup_stack_t *cleanup;

    cleanup = cru_cleanup_create();
    if (!cleanup)
        goto fail_create_cleanup_stack;

    cru_slist_prepend_atomic(&t->cleanup_stacks, cleanup);

    current = (cru_current_test_t) {
        .test = t,
        .cleanup = cleanup,
    };

    ASSERT_IN_TEST_THREAD;

    // Die now if a different thread cancelled the test.
    t_check_cancelled();

    return;

fail_create_cleanup_stack:
    // Without a cleanup stack, this is not a well-formed test thread. That
    // prevents us from calling any "t_*" functions, even t_end(). So give up
    // and die.
    cru_loge("failed to create cleanup stack for test thread");
    abort();
}


static void
t_create_attachment(VkDevice dev,
                    VkFormat format,
                    VkImageUsageFlags image_usage_flags,
                    VkImageAspect image_aspect,
                    uint32_t width, uint32_t height,
                    VkImage *out_image,
                    VkAttachmentView *out_attachment_view,
                    VkImageView *out_image_view)
{
    ASSERT_TEST_IN_SETUP_PHASE;

    if (format == VK_FORMAT_UNDEFINED) {
        *out_image = QO_NULL_IMAGE;
        *out_attachment_view = QO_NULL_ATTACHMENT_VIEW;
        *out_image_view = QO_NULL_IMAGE_VIEW;
        return;
    }

    t_assert(width > 0);
    t_assert(height > 0);

    *out_image = qoCreateImage(dev,
        .format = format,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1,
        },
        .usage = image_usage_flags);

    VkDeviceMemory mem = qoAllocImageMemory(dev, *out_image,
        .memoryTypeIndex = t_mem_type_index_for_device_access);

    qoBindImageMemory(dev, *out_image, mem, /*offset*/ 0);

    *out_attachment_view = qoCreateAttachmentView(dev,
        .image = *out_image,
        .format = format);

    *out_image_view = qoCreateImageView(dev,
        .image = *out_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspect = image_aspect,
            .baseMipLevel = 0,
            .mipLevels = 1,
            .baseArraySlice = 0,
            .arraySize = 1,
        });
}

static void
t_create_framebuffer(void)
{
    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    VkAttachmentBindInfo bind_info[2];
    uint32_t att_count = 0;

    if (t->def->no_image)
        return;

    t_create_attachment(t->device, VK_FORMAT_R8G8B8A8_UNORM,
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                        VK_IMAGE_ASPECT_COLOR,
                        t->width, t->height,
                        &t->rt_image,
                        &t->color_attachment_view,
                        &t->color_texture_view);

    bind_info[att_count++] = (VkAttachmentBindInfo) {
        .view = t->color_attachment_view,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    if (t->def->depthstencil_format != VK_FORMAT_UNDEFINED) {
        t_create_attachment(t->device, t->def->depthstencil_format,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_BIT,
                            VK_IMAGE_ASPECT_DEPTH,
                            t->width, t->height,
                            &t->ds_image,
                            &t->ds_attachment_view,
                            &t->depth_image_view);

        bind_info[att_count++] = (VkAttachmentBindInfo) {
            .view = t->ds_attachment_view,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }

    t->framebuffer = qoCreateFramebuffer(t_device,
        .width = t->width,
        .height = t->height,
        .attachmentCount = att_count,
        .pAttachments = bind_info);
}

static void *
cru_alloc(void *pUserData, size_t size, size_t alignment,
          VkSystemAllocType allocType)
{
    void *mem = malloc(size);
    memset(mem, 139, size);
    return mem;
}

static void
cru_free(void *pUserData, void *pMem)
{
    free(pMem);
}

static const VkAllocCallbacks cru_alloc_cb = {
    .pfnAlloc = cru_alloc,
    .pfnFree = cru_free,
};

static void *
main_thread_start(void *arg)
{
    ASSERT_NOT_IN_TEST_THREAD;
    thread_bind_to_test((cru_test_t *) arg);
    ASSERT_IN_TEST_THREAD;

    ASSERT_TEST_IN_SETUP_PHASE;
    GET_CURRENT_TEST(t);

    t_assertf(t->def->start, "test defines no start function");

    if (!t->opt.bootstrap && !t->def->no_image)
        t_load_image_file();

    vkCreateInstance(
        &(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pAppInfo = &(VkApplicationInfo) {
                .pAppName = "crucible",
                .apiVersion = 1,
            },
            .pAllocCb = &cru_alloc_cb,
        },
        &t->instance);
    t_cleanup_push_vk_instance(t_instance);

    t_init_physical_dev();
    t_init_physical_dev_mem_props();

    vkCreateDevice(t->physical_dev,
        &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueRecordCount = 1,
            .pRequestedQueues = &(VkDeviceQueueCreateInfo) {
                .queueFamilyIndex = 0,
                .queueCount = 1,
            },
        },
        &t->device);
    t_cleanup_push_vk_device(t_device);

    vkGetDeviceQueue(t_device, 0, 0, &t->queue);

    t->pipeline_cache = qoCreatePipelineCache(t->device);

    t->dynamic_vp_state = qoCreateDynamicViewportState(t->device,
        .viewportAndScissorCount = 1,
        .pViewports = (VkViewport[]) {
            {
                .originX = 0,
                .originY = 0,
                .width = t->width,
                .height = t->height,
                .minDepth = 0,
                .maxDepth = 1
            },
        },
        .pScissors = (VkRect2D[]) {
            {{ 0, 0 }, { t->width, t->height }},
        }
    );
    t->dynamic_rs_state = qoCreateDynamicRasterState(t_device);
    t->dynamic_cb_state = qoCreateDynamicColorBlendState(t_device);
    t->dynamic_ds_state = qoCreateDynamicDepthStencilState(t_device);

    VkResult res = vkCreateCommandPool(t_device,
        &(VkCmdPoolCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO,
            .queueFamilyIndex = 0,
            .flags = 0,
        }, &t->cmd_pool);
    t_assert(res == VK_SUCCESS);
    t_cleanup_push_vk_cmd_pool(t->device, t->cmd_pool);

    t->cmd_buffer = qoCreateCommandBuffer(t_device, t_cmd_pool);

    qoBeginCommandBuffer(t_cmd_buffer);
    vkCmdBindDynamicViewportState(t->cmd_buffer, t->dynamic_vp_state);
    vkCmdBindDynamicRasterState(t->cmd_buffer, t->dynamic_rs_state);
    vkCmdBindDynamicColorBlendState(t->cmd_buffer, t->dynamic_cb_state);
    vkCmdBindDynamicDepthStencilState(t->cmd_buffer, t->dynamic_ds_state);

    t_create_framebuffer();

    t->phase = CRU_TEST_PHASE_MAIN;
    t->def->start();

    if (!t->def->no_image)
        t_compare_image();

    // Don't prematurely end the test before the test has completed executing.
    vkQueueWaitIdle(t->queue);

    // The framework must cancel the test when the test's start function
    // returns. Otherwise, cru_test_wait() will deadlock waiting for test
    // cancellation unless the test created a subthread that cancels later.
    t_pass();
}

static void *
user_thread_start(void *_arg)
{
    user_thread_arg_t arg;

    // To avoid leaking the thread argument, we must copy it.
    arg = *(user_thread_arg_t*) _arg;
    free(_arg);

    // Connect this newly created thread to the test framework before giving
    // control to the user-supplied thread start function.
    ASSERT_NOT_IN_TEST_THREAD;
    thread_bind_to_test(arg.test);
    ASSERT_IN_TEST_THREAD;

    arg.start_func(arg.start_arg);
    t_thread_exit();
}

/// \brief The cleanup thread's start function.
///
/// The cleanup thread runs after all other test thread's have exited
/// and unwinds all the test's cleanup stacks.
///
/// \see cru_test::cleanup_stacks
static void *
cleanup_thread_start(void *arg)
{
    cru_test_t *t = arg;
    cru_cleanup_stack_t *cleanup = NULL;

    ASSERT_IN_CLEANUP_THREAD(t);

    while ((cleanup = cru_slist_pop(&t->cleanup_stacks))) {
        if (t->opt.no_cleanup)
            cru_cleanup_pop_all_noop(cleanup);

        cru_cleanup_release(cleanup);
    }

    cru_test_stop(t);

    return NULL;
}

static bool
cru_test_create_thread(cru_test_t *t, void *(*start)(void *arg), void *arg,
                       pthread_t *out_thread)
{
    pthread_t *thread = NULL;
    int err;

    thread = xmalloc(sizeof(pthread_t));

    err = pthread_create(thread, NULL, start, arg);
    if (err) {
        cru_loge("%s: failed to start thread", t->def->name);
        goto fail;
    }

    cru_slist_prepend_atomic(&t->threads, thread);

    if (out_thread)
        *out_thread = *thread;

    return true;

fail:
    free(thread);
    return false;
}

void
cru_test_start(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;
    ASSERT_TEST_IN_PRESTART_PHASE(t);

    t->phase = CRU_TEST_PHASE_SETUP;

    if (t->def->skip) {
        t->phase = CRU_TEST_PHASE_STOPPED;
        t->result = TEST_RESULT_SKIP;
        return;
    }

    if (!cru_test_create_thread(t, main_thread_start, t, NULL)) {
        t->phase = CRU_TEST_PHASE_STOPPED;
        t->result = TEST_RESULT_FAIL;
        return;
    }
}

/// Create a new test thread. Must be called from an existing test thread.
///
/// TODO: Document join behavior. Users can't join test threads. Test runner
/// joins them.
void
t_create_thread(void (*start)(void *arg), void *arg)
{
    ASSERT_TEST_IN_MAJOR_PHASE;
    GET_CURRENT_TEST(t);

    user_thread_arg_t *test_arg = NULL;

    assert(start);

    test_arg = xzalloc(sizeof(*test_arg));
    test_arg->test = t;
    test_arg->start_func = start;
    test_arg->start_arg = arg;

    if (!cru_test_create_thread(t, user_thread_start, test_arg, NULL)) {
        t_fail_silent();
    }
}

/// All test threads must exit through this function.
static void cru_noreturn
t_thread_exit(void)
{
    pthread_exit(NULL);
}

void
cru_test_wait(cru_test_t *t)
{
    ASSERT_NOT_IN_TEST_THREAD;

    int err;

    err = pthread_mutex_lock(&t->stop_mutex);
    if (err) {
        cru_loge("%s: failed to lock test mutex", t->def->name);
        abort();
    }

    while (t->phase < CRU_TEST_PHASE_STOPPED) {
        err = pthread_cond_wait(&t->stop_cond, &t->stop_mutex);
        if (err) {
            cru_loge("%s: failed to wait on test's result condition",
                     t->def->name);
            abort();
        }
    }

    pthread_mutex_unlock(&t->stop_mutex);
}

void
t_cleanup_push_command(enum cru_cleanup_cmd cmd, ...)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    va_list va;

    va_start(va, cmd);
    t_cleanup_push_commandv(cmd, va);
    va_end(va);
}

void
t_cleanup_push_commandv(enum cru_cleanup_cmd cmd, va_list va)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    cru_cleanup_push_commandv(current.cleanup, cmd, va);
}

void
t_cleanup_pop(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    cru_cleanup_pop(current.cleanup);
}

void
t_cleanup_pop_all(void)
{
    ASSERT_TEST_IN_MAJOR_PHASE;

    cru_cleanup_pop_all(current.cleanup);
}

void
test_result_merge(test_result_t *accum,
                      test_result_t new_result)
{
    *accum = MAX(*accum, new_result);
}
