#!/bin/bash

set -eu

die() {
    printf >&2 "concurrent-output: error: %s\n" "$*"
    exit 1
}

name='concurrent-output'
stdout_log="$CRUCIBLE_TOP/src/tests/self/${name}.stdout"
stderr_log="$CRUCIBLE_TOP/src/tests/self/${name}.stderr"

if ! "$CRUCIBLE_TOP"/bin/crucible run -j8 "self.${name}.*" \
        1>"$stdout_log" 2>"$stderr_log"; then
    die "crucible run failed"
fi

if grep -q 'aabb' "$stdout_log"; then
    die "interleaved output in stdout"
fi

if grep -q 'aabb' "$stderr_log"; then
    die "interleaved output in stderr"
fi
