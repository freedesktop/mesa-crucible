# Copyright 2015 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

#
# SYNOPSIS
#
#   AX_CHECK_PYTHON3_MODULE_OPENCV()
#
# DESCRIPTION
#
#   Check that the Python3 OpenCV module is installed.
#

AC_DEFUN([AX_CHECK_PYTHON3_MODULE_OPENCV],
[AC_MSG_CHECKING(if Python3 OpenCV modules is installed)
    $PYTHON3 -c "
try:
    import cv2
except ImportError as err:
    import sys
    sys.exit(err)
"

    if test $? -ne 0 ; then
       AC_MSG_ERROR([Python3 OpenCV module not found])
    else
       AC_MSG_RESULT([yes])
    fi
])
