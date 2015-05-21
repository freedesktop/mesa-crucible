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

AC_INIT([crucible], [0.1])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES(yes)
LT_INIT

AC_PROG_CXX

PKG_PROG_PKG_CONFIG()

AC_ARG_WITH([mesa],
    [AS_HELP_STRING([--with-mesa@<:@=PATH@:>@],
                    [Path to mesa repo for headers and libraries])])
if test "x$with_mesa" != x; then
    MESA_CPPFLAGS=-I$with_mesa/include
    MESA_LDFLAGS="-L$with_mesa/lib -R$with_mesa/lib"
fi

AC_SUBST([MESA_CPPFLAGS])
AC_SUBST([MESA_LDFLAGS])

AC_ARG_WITH([minigbm],
    [AS_HELP_STRING([--with-minigbm@<:@=PATH@:>@],
                    [Path to minigbm repo for headers and libraries])])
if test "x$with_minigbm" != x; then
    MINIGBM_CPPFLAGS=-I$with_minigbm/usr/include
    MINIGBM_LDFLAGS="-L$with_minigbm/usr/lib -R$with_minigbm/usr/lib"
fi

AC_SUBST([MINIGBM_CPPFLAGS])
AC_SUBST([MINIGBM_LDFLAGS])

AC_OUTPUT(Makefile)
