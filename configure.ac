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
AC_CONFIG_MACRO_DIR([m4])
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
    if test -d "$with_mesa/lib64"; then
        MESA_LDFLAGS="-L$with_mesa/lib64 -R$with_mesa/lib64"
    else
        MESA_LDFLAGS="-L$with_mesa/lib -R$with_mesa/lib"
    fi
fi

AC_SUBST([MESA_CPPFLAGS])
AC_SUBST([MESA_LDFLAGS])

AC_CHECK_PROGS([PYTHON3], [python3])
AX_CHECK_PYTHON3_MODULE_OPENCV()

AC_ARG_VAR([GLSLANG], [path to glslangValidator executable])
AC_SUBST([GLSLANG])
AC_CHECK_PROGS([GLSLANG], [glslangValidator])
if test "x$GLSLANG" = "x"; then
    AC_MSG_ERROR([failed to find glslangValidator])
fi

PKG_CHECK_MODULES([libpng16], [libpng16], [HAVE_LIBPNG16=1], [HAVE_LIBPNG16=0])
AM_CONDITIONAL([NO_PNG16], [test $HAVE_LIBPNG16 = 0])
AM_COND_IF([NO_PNG16],
           [PKG_CHECK_MODULES([libpng12], [libpng12])])
AM_COND_IF([NO_PNG16],
           [libpng_CFLAGS="$libpng12_CFLAGS"],
           [libpng_CFLAGS="$libpng16_CFLAGS"])
AC_SUBST(libpng_CFLAGS)
AM_COND_IF([NO_PNG16],
           [libpng_LIBS="$libpng12_LIBS"],
           [libpng_LIBS="$libpng16_LIBS"])
AC_SUBST(libpng_LIBS)

PKG_CHECK_MODULES([LIBXML2], [libxml-2.0])

AC_ARG_VAR([ASCIIDOC], [path to asciidoc executable])
AC_SUBST([ASCIIDOC])
AC_CHECK_PROGS([ASCIIDOC], [asciidoc])
if test "x$ASCIIDOC" = "x"; then
    AC_MSG_ERROR([AsciiDoc not found - unable to build documentation])
fi

AC_ARG_VAR([A2X], [path to a2x, the AsciiDoc toolchain manager])
AC_SUBST([A2X])
AC_CHECK_PROGS([A2X], [a2x])
if test "x$A2X" = "x"; then
    AC_MSG_ERROR([a2x not found - unable to build documentation])
fi

AC_OUTPUT(Makefile)
