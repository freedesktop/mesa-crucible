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

CPPFLAGS = \
   $(MESA_CPPFLAGS) \
   -D_POSIX_C_SOURCE=200809L \
   -Iinclude

common_CFLAGS = -Wall -O0 -g3

CFLAGS = -std=gnu11 \
	 $(common_CFLAGS) \
	-Werror=implicit-function-declaration \
	-Werror=missing-prototypes

CXXFLAGS = -std=c++11 $(common_CFLAGS)

noinst_PROGRAMS = \
	bin/crucible

bin_crucible_SOURCES = \
	src/crucible/cru_array.c \
	src/crucible/cru_cleanup.c \
	src/crucible/cru_cmd.c \
	src/crucible/cru_cmd_bootstrap.c \
	src/crucible/cru_cmd_help.c \
	src/crucible/cru_cmd_ls_example_tests.c \
	src/crucible/cru_cmd_ls_tests.c \
	src/crucible/cru_cmd_main.c \
	src/crucible/cru_cmd_run.c \
	src/crucible/cru_cmd_version.c \
	src/crucible/cru_image.c \
	src/crucible/cru_log.c \
	src/crucible/cru_misc.c \
	src/crucible/cru_pixel_image.c \
	src/crucible/cru_png_image.c \
	src/crucible/cru_slist.c \
	src/crucible/cru_test.c \
	src/crucible/cru_vk_pipeline.c \
	src/crucible/qonos.c \
	src/crucible/string.c \
	src/crucible/xalloc.c \
	src/tests/example/basic.c \
	src/tests/example/images.c \
	src/tests/example/messages.c \
	src/tests/func/copy/copy-buffer.c \
	src/tests/func/desc/dynamic.c \
	src/tests/func/draw_indexed.c \
	src/tests/func/ds/basic.c \
	src/tests/func/query/timestamp.c \
	src/tests/func/first.c \
	src/tests/func/interleaved-cmd-buffers.c \
	src/tests/stress/lots-of-surface-state.c

bin_crucible_LDADD = $(MESA_LDFLAGS) -lm -lvulkan -lpthread -lpng16

man1_MANS = \
    doc/crucible-bootstrap.1 \
    doc/crucible-help.1 \
    doc/crucible-ls-example-tests.1 \
    doc/crucible-ls-tests.1 \
    doc/crucible-run.1 \
    doc/crucible-version.1

.PHONY: man
man: $(man1_MANS)
	@

%.1: %.1.txt
	$(AM_V_GEN)$(A2X) --format manpage $^

CLEANFILES = $(man1_MANS)
