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

/// Is a test running in this thread?
bool t_is_current(void);

void t_end(cru_test_result_t result) cru_noreturn;
void t_pass(void) cru_noreturn;

#define t_skip() __t_skip(__FILE__, __LINE__)
#define t_skipf(format, ...) __t_skipf(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define t_skipvf(format, va) __t_skipfv(__FILE__, __LINE__, format, va)
#define t_skip_silent() __t_skip_silent()
void __t_skip(const char *file, int line) cru_noreturn;
void __t_skipf(const char *file, int line, const char *format, ...) cru_noreturn cru_printflike(3, 4);
void __t_skipfv(const char *file, int line, const char *format, va_list va) cru_noreturn;
void __t_skip_silent(void) cru_noreturn;

#define t_fail() __t_fail(__FILE__, __LINE__)
#define t_failf(format, ...) __t_failf(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define t_failvf(format, va) __t_failfv(__FILE__, __LINE__, format, va)
#define t_fail_silent() __t_fail_silent()
void __t_fail(const char *file, int line) cru_noreturn;
void __t_failf(const char *file, int line, const char *format, ...) cru_noreturn cru_printflike(3, 4);
void __t_failfv(const char *file, int line, const char *format, va_list va) cru_noreturn;
void __t_fail_silent(void) cru_noreturn;

#define t_assert(cond) __t_assert(__FILE__, __LINE__, (cond), #cond)
#define t_assertf(cond, format, ...) __t_assertf(__FILE__, __LINE__, (cond), #cond, format, ##__VA_ARGS__)
#define t_assertfv(cond, format, va) __t_assertfv(__FILE__, __LINE__, (cond), #cond, format, va)
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
#define t_dynamic_vp_state (*__t_dynamic_vp_state())
#define t_dynamic_rs_state (*__t_dynamic_rs_state())
#define t_dynamic_cb_state (*__t_dynamic_cb_state())
#define t_dynamic_ds_state (*__t_dynamic_ds_state())
#define t_image (*__t_image())
#define t_image_color_view (*__t_image_color_view())
#define t_image_texture_view (*__t_image_texture_view())
#define t_framebuffer (*__t_framebuffer())
#define t_pipeline_cache (*__t_pipeline_cache())
#define t_width (*__t_width())
#define t_height (*__t_height())
cru_image_t *t_ref_image(void);
#define t_use_spir_v (*__t_use_spir_v())
/// \}

const char *__t_name(void);
const void *__t_user_data(void);
const VkInstance *__t_instance(void);
const VkDevice *__t_device(void);
const VkPhysicalDevice *__t_physical_device(void);
const VkQueue *__t_queue(void);
const VkCmdBuffer *__t_cmd_buffer(void);
const VkDynamicVpState *__t_dynamic_vp_state(void);
const VkDynamicRsState *__t_dynamic_rs_state(void);
const VkDynamicCbState *__t_dynamic_cb_state(void);
const VkDynamicDsState *__t_dynamic_ds_state(void);
const VkImage *__t_image(void);
const VkColorAttachmentView *__t_image_color_view(void);
const VkImageView *__t_image_texture_view(void);
const VkFramebuffer *__t_framebuffer(void);
const VkPipelineCache *__t_pipeline_cache(void);
const uint32_t *__t_height(void);
const uint32_t *__t_width(void);
const bool * __t_use_spir_v(void);

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
static inline void t_cleanup_push_vk_device_memory(VkDevice d, VkDeviceMemory m)                         { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_DEVICE_MEMORY, d, m); }
static inline void t_cleanup_push_vk_memory_map(VkDevice d, VkDeviceMemory m)                            { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_MEMORY_MAP, d, m); }
static inline void t_cleanup_push_vk_object(VkDevice dev, VkObjectType obj_type, VkObject obj)           { t_cleanup_push_command(CRU_CLEANUP_CMD_VK_OBJECT, dev, obj_type, obj); }

static inline void t_cleanup_push_vk_physical_device(VkDevice dev, VkPhysicalDevice x)                   { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_PHYSICAL_DEVICE, x); }
static inline void t_cleanup_push_vk_queue(VkDevice dev, VkQueue x)                                      { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_QUEUE, x); }
static inline void t_cleanup_push_vk_command_buffer(VkDevice dev, VkCmdBuffer x)                         { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_COMMAND_BUFFER, x); }
static inline void t_cleanup_push_vk_buffer(VkDevice dev, VkBuffer x)                                    { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_BUFFER, x); }
static inline void t_cleanup_push_vk_buffer_view(VkDevice dev, VkBufferView x)                           { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_BUFFER_VIEW, x); }
static inline void t_cleanup_push_vk_image(VkDevice dev, VkImage x)                                      { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_IMAGE, x); }
static inline void t_cleanup_push_vk_image_view(VkDevice dev, VkImageView x)                             { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_IMAGE_VIEW, x); }
static inline void t_cleanup_push_vk_color_attachment_view(VkDevice dev, VkColorAttachmentView x)        { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_COLOR_ATTACHMENT_VIEW, x); }
static inline void t_cleanup_push_vk_depth_stencil_view(VkDevice dev, VkDepthStencilView x)              { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DEPTH_STENCIL_VIEW, x); }
static inline void t_cleanup_push_vk_shader(VkDevice dev, VkShader x)                                    { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_SHADER, x); }
static inline void t_cleanup_push_vk_shader_module(VkDevice dev, VkShader x)                             { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_SHADER_MODULE, x); }
static inline void t_cleanup_push_vk_pipeline_cache(VkDevice dev, VkRenderPass x)                        { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_PIPELINE_CACHE, x); }
static inline void t_cleanup_push_vk_pipeline(VkDevice dev, VkPipeline x)                                { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_PIPELINE, x); }
static inline void t_cleanup_push_vk_pipeline_layout(VkDevice dev, VkPipelineLayout x)                   { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_PIPELINE_LAYOUT, x); }
static inline void t_cleanup_push_vk_sampler(VkDevice dev, VkSampler x)                                  { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_SAMPLER, x); }
static inline void t_cleanup_push_vk_descriptor_set(VkDevice dev, VkDescriptorSet x)                     { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DESCRIPTOR_SET, x); }
static inline void t_cleanup_push_vk_descriptor_set_layout(VkDevice dev, VkDescriptorSetLayout x)        { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, x); }
static inline void t_cleanup_push_vk_descriptor_pool(VkDevice dev, VkDescriptorPool x)                   { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DESCRIPTOR_POOL, x); }
static inline void t_cleanup_push_vk_dynamic_vp_state(VkDevice dev, VkDynamicVpState x)                  { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DYNAMIC_VP_STATE, x); }
static inline void t_cleanup_push_vk_dynamic_rs_state(VkDevice dev, VkDynamicRsState x)                  { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DYNAMIC_RS_STATE, x); }
static inline void t_cleanup_push_vk_dynamic_cb_state(VkDevice dev, VkDynamicCbState x)                  { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DYNAMIC_CB_STATE, x); }
static inline void t_cleanup_push_vk_dynamic_ds_state(VkDevice dev, VkDynamicDsState x)                  { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_DYNAMIC_DS_STATE, x); }
static inline void t_cleanup_push_vk_fence(VkDevice dev, VkFence x)                                      { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_FENCE, x); }
static inline void t_cleanup_push_vk_semaphore(VkDevice dev, VkSemaphore x)                              { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_SEMAPHORE, x); }
static inline void t_cleanup_push_vk_event(VkDevice dev, VkEvent x)                                      { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_EVENT, x); }
static inline void t_cleanup_push_vk_query_pool(VkDevice dev, VkQueryPool x)                             { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_QUERY_POOL, x); }
static inline void t_cleanup_push_vk_framebuffer(VkDevice dev, VkFramebuffer x)                          { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_FRAMEBUFFER, x); }
static inline void t_cleanup_push_vk_render_pass(VkDevice dev, VkRenderPass x)                           { t_cleanup_push_vk_object(dev, VK_OBJECT_TYPE_RENDER_PASS, x); }

#ifdef __cplusplus
}
#endif
