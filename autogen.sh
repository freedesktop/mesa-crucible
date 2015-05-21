#!/bin/sh

set -eu

: ${NOCONFIGURE:=}

srcdir="`dirname "$0"`"
test -z "$srcdir" && srcdir=.

ORIGDIR="`pwd`"
cd "$srcdir"

autoreconf -v --install
cd "$ORIGDIR"

if test -z "$NOCONFIGURE"; then
    "$srcdir"/configure "$@"
fi
