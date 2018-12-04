# Copyright 2015 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

export CRUCIBLE_TOP := $(srcdir)

AM_CPPFLAGS = \
   $(MESA_CPPFLAGS) \
   -D_DEFAULT_SOURCE \
   -D_GNU_SOURCE \
   -D_POSIX_C_SOURCE=200809L \
   -D_XOPEN_SOURCE=700 \
   -Iinclude

common_CFLAGS = -Wall -O0 -g3 \
	-Werror=format \
	-Werror=format-security \
	-Werror=incompatible-pointer-types \
	-Werror=int-conversion \
	$(libpng_CFLAGS) \
	$(LIBXML2_CFLAGS) \
	$(NULL)

AM_CFLAGS = -std=gnu11 \
	 $(common_CFLAGS) \
	-Werror=implicit-function-declaration \
	-Werror=implicit-int \
	-Werror=missing-prototypes

AM_CXXFLAGS = -std=c++11 $(common_CFLAGS)

noinst_PROGRAMS = \
	bin/crucible

bin_crucible_SOURCES = \
	src/cmd/cmd.c \
	src/cmd/bootstrap.c \
	src/cmd/dump-image.c \
	src/cmd/help.c \
	src/cmd/ls_tests.c \
	src/cmd/main.c \
	src/cmd/run.c \
	src/cmd/version.c \
	src/framework/runner/master.c \
	src/framework/runner/runner.c \
	src/framework/runner/slave.c \
	src/framework/test/t_cleanup.c \
	src/framework/test/t_data.c \
	src/framework/test/t_dump.c \
	src/framework/test/t_image.c \
	src/framework/test/t_phases.c \
	src/framework/test/t_phase_setup.c \
	src/framework/test/t_result.c \
	src/framework/test/t_thread.c \
	src/framework/test/test.c \
	src/framework/test/test_def.c \
	src/qonos/qonos.c \
	src/qonos/qonos_pipeline.c \
	src/tests/bug/104809.c \
	src/tests/bug/108909.c \
	src/tests/bench/copy-buffer.c \
	src/tests/example/basic.c \
	src/tests/example/images.c \
	src/tests/example/messages.c \
	src/tests/func/amd/amd_common.c \
	src/tests/func/amd/gcn_shader.c \
	src/tests/func/amd/shader_trinary_minmax.c \
	src/tests/func/cmd-buffer/secondary.c \
	src/tests/func/copy/copy-buffer.c \
	src/tests/func/4-vertex-buffers.c \
	src/tests/func/depthstencil/basic.c \
	src/tests/func/depthstencil/arrayed-clear.c \
	src/tests/func/depthstencil/stencil_triangles.c \
	src/tests/func/desc/dynamic.c \
	src/tests/func/desc/binding.c \
	src/tests/func/draw-indexed.c \
	src/tests/func/event.c \
	src/tests/func/gs/basic.c \
	src/tests/func/query/timestamp.c \
	src/tests/func/first.c \
	src/tests/func/bind-points.c \
	src/tests/func/compute.c \
	src/tests/func/compute-local-id.c \
	src/tests/func/compute-num-workgroups.c \
	src/tests/func/interleaved-cmd-buffers.c \
	src/tests/func/miptree/miptree.c \
	src/tests/func/multiview.c \
	src/tests/func/push-constants/basic.c \
	src/tests/func/shader/constants.c \
	src/tests/func/shader/fragcoord.c \
	src/tests/func/shader/pack_unpack.c \
	src/tests/func/shader_ballot/ext_shader_ballot.c \
	src/tests/func/shader_ballot/amd_shader_ballot.c \
	src/tests/func/shader_group_vote/ext_shader_subgroup_vote.c \
	src/tests/func/ssbo/interleave.c \
	src/tests/func/sync/semaphore-fd.c \
	src/tests/func/renderpass/clear.c \
	src/tests/stress/lots-of-surface-state.c \
	src/tests/stress/buffer_limit.c \
	src/tests/self/concurrent-output.c \
	src/tests/func/calibrated-timestamps.c \
	src/util/cru_cleanup.c \
	src/util/cru_format.c \
	src/util/cru_image.c \
	src/util/cru_vk_image.c \
	src/util/log.c \
	src/util/misc.c \
	src/util/cru_pixel_image.c \
	src/util/cru_png_image.c \
	src/util/cru_vec.c \
	src/util/string.c \
	src/util/xalloc.c \
	src/util/simple_pipeline.c \
	$(NULL)

BUILT_SOURCES = \
	src/qonos/qonos_pipeline-spirv.h \
	src/util/simple_pipeline-spirv.h \
	src/tests/bug/104809-spirv.h \
	src/tests/func/4-vertex-buffers-spirv.h \
	src/tests/func/depthstencil/basic-spirv.h \
	src/tests/func/depthstencil/arrayed-clear-spirv.h \
	src/tests/func/depthstencil/stencil_triangles-spirv.h \
	src/tests/func/depthstencil/stencil_triangles_gen.c \
	src/tests/func/desc/dynamic-spirv.h \
	src/tests/func/draw-indexed-spirv.h \
	src/tests/func/gs/basic-spirv.h \
	src/tests/func/first-spirv.h \
	src/tests/func/bind-points-spirv.h \
	src/tests/func/compute-spirv.h \
	src/tests/func/compute-local-id-spirv.h \
	src/tests/func/compute-num-workgroups-spirv.h \
	src/tests/func/miptree/miptree-spirv.h \
	src/tests/func/miptree/miptree_gen.c \
	src/tests/func/multiview-spirv.h \
	src/tests/func/push-constants/basic-spirv.h \
	src/tests/func/amd/gcn_shader-spirv.h \
	src/tests/func/amd/shader_trinary_minmax-spirv.h \
	src/tests/func/shader/constants-spirv.h \
	src/tests/func/shader/fragcoord-spirv.h \
	src/tests/func/shader/pack_unpack-spirv.h \
	src/tests/func/shader_ballot/ext_shader_ballot-spirv.h \
	src/tests/func/shader_ballot/amd_shader_ballot-spirv.h \
	src/tests/func/shader_group_vote/ext_shader_subgroup_vote-spirv.h \
	src/tests/func/ssbo/interleave-spirv.h \
	src/tests/func/sync/semaphore-fd-spirv.h \
	src/tests/stress/lots-of-surface-state-spirv.h

bin_crucible_LDADD = $(MESA_LDFLAGS) -lm -lvulkan -lpthread $(libpng_LIBS) \
		     $(LIBXML2_LIBS)

%-spirv.h: %.c misc/glsl_scraper.py
	$(AM_V_GEN) $(PYTHON3) $(srcdir)/misc/glsl_scraper.py --with-glslang=$(GLSLANG) -o $@ $<

%_gen.c: %_gen.py
	$(AM_V_GEN) $(PYTHON3) $<

man1_MANS = \
    doc/crucible-bootstrap.1 \
    doc/crucible-dump-image.1 \
    doc/crucible-help.1 \
    doc/crucible-tutorial.7 \
    doc/crucible-ls-tests.1 \
    doc/crucible-run.1 \
    doc/crucible-version.1

.PHONY: man
man: $(man1_MANS)
	@

%.1: %.1.txt
	$(AM_V_GEN)$(A2X) --format manpage $^

%.7: %.7.txt
	$(AM_V_GEN)$(A2X) --format manpage $^

built_data_files = \
	data/grass-2048x1024.jpg \
	data/grass-grayscale-2048x1024.png \
	data/grass-grayscale-1024x1024.png \
	data/grass-grayscale-1024x512.png \
	data/grass-grayscale-512x512.png \
	data/grass-grayscale-512x256.png \
	data/grass-grayscale-256x256.png \
	data/grass-grayscale-256x128.png \
	data/grass-grayscale-128x128.png \
	data/grass-grayscale-128x64.png \
	data/grass-grayscale-64x64.png \
	data/grass-grayscale-64x32.png \
	data/grass-grayscale-32x32.png \
	data/grass-grayscale-32x16.png \
	data/grass-grayscale-16x16.png \
	data/grass-grayscale-16x8.png \
	data/grass-grayscale-8x8.png \
	data/grass-grayscale-8x4.png \
	data/grass-grayscale-4x4.png \
	data/grass-grayscale-4x2.png \
	data/grass-grayscale-2x2.png \
	data/grass-grayscale-2x1.png \
	data/grass-grayscale-1x1.png \
	data/pink-leaves-2048x1024.jpg \
	data/pink-leaves-grayscale-2048x1024.png \
	data/pink-leaves-grayscale-1024x1024.png \
	data/pink-leaves-grayscale-1024x512.png \
	data/pink-leaves-grayscale-512x512.png \
	data/pink-leaves-grayscale-512x256.png \
	data/pink-leaves-grayscale-256x256.png \
	data/pink-leaves-grayscale-256x128.png \
	data/pink-leaves-grayscale-128x128.png \
	data/pink-leaves-grayscale-128x64.png \
	data/pink-leaves-grayscale-64x64.png \
	data/pink-leaves-grayscale-64x32.png \
	data/pink-leaves-grayscale-32x32.png \
	data/pink-leaves-grayscale-32x16.png \
	data/pink-leaves-grayscale-16x16.png \
	data/pink-leaves-grayscale-16x8.png \
	data/pink-leaves-grayscale-8x8.png \
	data/pink-leaves-grayscale-8x4.png \
	data/pink-leaves-grayscale-4x4.png \
	data/pink-leaves-grayscale-4x2.png \
	data/pink-leaves-grayscale-2x2.png \
	data/pink-leaves-grayscale-2x1.png \
	data/pink-leaves-grayscale-1x1.png \
	$(NULL)

.PHONY: data
all: data
data: $(built_data_files)
	@

data/grass-2048x1024.jpg: \
    $(srcdir)/data/grass-2014x1536.jpg \
    $(srcdir)/misc/gen_image
	$(AM_V_GEN) $(PYTHON2) $(srcdir)/misc/gen_image crop $< $@

data/grass-%.png: \
    $(srcdir)/data/grass-2048x1024.jpg \
    $(srcdir)/misc/gen_image
	$(AM_V_GEN) $(PYTHON2) $(srcdir)/misc/gen_image scale $< $@

data/pink-leaves-2048x1024.jpg: \
    $(srcdir)/data/pink-leaves-3264x2448.jpg \
    $(srcdir)/misc/gen_image
	$(AM_V_GEN) $(PYTHON2) $(srcdir)/misc/gen_image crop $< $@

data/pink-leaves-%.png: \
    $(srcdir)/data/pink-leaves-2048x1024.jpg \
    $(srcdir)/misc/gen_image
	$(AM_V_GEN) $(PYTHON2) $(srcdir)/misc/gen_image scale $< $@

TESTS = \
	src/tests/self/bad-test-names.bash \
	src/tests/self/concurrent-output.bash

CLEANFILES = $(man1_MANS) $(BUILT_SOURCES) $(built_data_files)
