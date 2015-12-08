#!/usr/bin/env bash
#
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

#
# This test fails if Crucible contains any bad test names.
#

set -o errexit
set -o nounset
set -o pipefail

err() {
    printf "error: %s\n" "$*"
} >&2

die() {
    if [[ $# -ne 0 ]]; then
        err "$@"
    fi

    exit 1
} >&2

bad_test_names() {
    "$CRUCIBLE_TOP"/bin/crucible ls-tests |
    grep -E -v '^[a-zA-Z0-9_.-]+$'
}

main() {
    if bad_test_names >&/dev/null; then
        err "found bad test names:"
        bad_test_names
        exit 1
    fi
}

main "$@"
