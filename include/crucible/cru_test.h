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
///
///   - The test ends when the start function returns even if the test has live
///     subthreads.
///
///   - The framework automatically calls t_compare_image() when the
///     test's start function returns, unless it's a no-image test.
///
///   - No test is current when the cleanup commands run.
///
///   - Each test thread owns its own cleanup stack. Therefore concurrent calls
///     to t_cleanup*() are safe.

#pragma once

#define VK_PROTOTYPES

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#include <crucible/cru_cleanup.h>
#include <crucible/cru_macros.h>

typedef struct cru_test_def cru_test_def_t;

typedef enum cru_test_result {
    CRU_TEST_RESULT_PASS,
    CRU_TEST_RESULT_SKIP,
    CRU_TEST_RESULT_FAIL,
} cru_test_result_t;

struct cru_test_def {
    /// The test name must be a valid filename with no path separator.
    const char *name;

    /// Reserved for the test author. The test framework never touches this
    /// data.
    const void *user_data;

    /// \brief Filename of the test's reference image.
    ///
    /// The filename is relative to Crucible's "img" directory. If unset, then
    /// the default filename "{test_name}.ref.png" is used.
    const char *image_filename;

    void (*start)(void);
    uint32_t samples;
    bool no_image;

    /// \brief Skip this test.
    ///
    /// This is useful for work-in-progress tests.
    bool skip;
} __attribute__((aligned(32)));

/// Example usage:
///
///    static void
///    draw_a_triangle(void)
///    {
///       ...
///    }
///
///    cru_define_test {
///       .name = "draw-a-triangle",
///       .start = draw_a_triangle,
///    }
///
#define cru_define_test \
    static const cru_test_def_t \
    CRU_CAT(__cru_test_def_, __COUNTER__) \
        __attribute__((__section__("cru_test_defs"))) \
        __attribute__((__used__)) =

void t_end(cru_test_result_t result) cru_noreturn;
void t_pass(void) cru_noreturn;
void t_skip(void) cru_noreturn;
void t_fail(void) cru_noreturn;

#define t_assert(cond) __t_assert(__FILE__, __LINE__, (cond), #cond)
#define t_assertf(cond, format, ...) __t_assertf(__FILE__, __LINE__, (cond), #cond, format, ##__VA_ARGS__)
#define t_assertfv(cond, format, va) __t_assertfv(__FILE__, __LINE__, (cond), #cond, format, va)
#define t_failif(cond, format, ...) t_assert(!cond) // TODO: Print error message
#define t_try(error) t_assert(error == 0)
void __t_assert(const char *file, int line, bool cond, const char *cond_string);
void __t_assertf(const char *file, int line, bool cond, const char *cond_string, const char *format, ...) cru_printflike(5, 6);
void __t_assertfv(const char *file, int line, bool cond, const char *cond_string, const char *format, va_list va);

/// Compare the test's rendered image against its reference image, ending the
/// test.
///
/// \see t_image
/// \see t_ref_image
void t_compare_image(void) cru_noreturn;

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

/// \defgroup Test data
/// \{
#define t_name __t_name()
#define t_user_data __t_user_data()
#define t_instance (*__t_instance())
#define t_physical_device (*__t_physical_device())
#define t_device (*__t_device())
#define t_queue (*__t_queue())
#define t_cmd_buffer (*__t_cmd_buffer())
#define t_image (*__t_image())
#define t_image_color_view (*__t_image_color_view())
#define t_image_texture_view (*__t_image_texture_view())
#define t_framebuffer (*__t_framebuffer())
#define t_width (*__t_width())
#define t_height (*__t_height())
cru_image_t *t_ref_image(void);
/// \}

const char *__t_name(void);
const void *__t_user_data(void);
const VkInstance *__t_instance(void);
const VkDevice *__t_device(void);
const VkPhysicalDevice *__t_physical_device(void);
const VkQueue *__t_queue(void);
const VkCmdBuffer *__t_cmd_buffer(void);
const VkImage *__t_image(void);
const VkColorAttachmentView *__t_image_color_view(void);
const VkImageView *__t_image_texture_view(void);
const VkFramebuffer *__t_framebuffer(void);
const uint32_t *__t_height(void);
const uint32_t *__t_width(void);

/// Create a new test thread.
///
/// \section Attributes
///   - test cancellation point
///
void t_create_thread(void (*start)(void *arg), void *arg);

/// Cancel this test thread if another thread terminated the test.
///
/// \section Attributes
///   - test cancellation point
///
void t_check_cancelled(void);

void t_cleanup_push_command(enum cru_cleanup_cmd cmd, ...);
void t_cleanup_push_commandv(enum cru_cleanup_cmd cmd, va_list va);
void t_cleanup_pop(void);
void t_cleanup_pop_all(void);

#ifdef DOXYGEN
void t_cleanup_push(T obj, ...);
#else
#define t_cleanup_push(x, ...) \
    _Generic((x), \
        cru_cleanup_callback_func_t     : t_cleanup_push_callback, \
        cru_cleanup_stack_t *           : t_cleanup_push_cru_cleanup_stack, \
        cru_image_t *                   : t_cleanup_push_cru_image \
    )((x), ##__VA_ARGS__)
#endif

static inline void t_cleanup_push_callback(void (*func)(void* data), void *data)                         { t_cleanup_push_command(CRU_CLEANUP_CMD_CALLBACK, func, data); }
static inline void t_cleanup_push_cru_cleanup_stack(cru_cleanup_stack_t *x)                              { t_cleanup_push_command(CRU_CLEANUP_CMD_CRU_CLEANUP_STACK, x); }
static inline void t_cleanup_push_cru_image(cru_image_t *x)                                              { t_cleanup_push_command(CRU_CLEANUP_CMD_CRU_IMAGE, x); }
static inline void t_cleanup_push_vk_instance(VkInstance x)                                              { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_INSTANCE, x); }
static inline void t_cleanup_push_vk_device(VkDevice x)                                                  { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DEVICE, x); }
static inline void t_cleanup_push_vk_object(VkDevice dev, VkObjectType obj_type, VkObject obj)           { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_OBJECT, dev, obj_type, obj); }

#ifdef __cplusplus
}
#endif
