#!/bin/bash
# src/tools/tests/run_tests.sh
# For each fixture pair <name>.vert/.frag:
#   1. glslc -> .spv
#   2. rlvk-shdc -> <name>.rlvk.h
#   3. diff against <name>.rlvk.h.expected
set -e

DIR=$(dirname "$0")
FIXTURES="$DIR/fixtures"
OUT="$DIR/out"
TOOL="$DIR/../rlvk-shdc"

mkdir -p "$OUT"
FAIL=0

normalize() { sed -E 's/\{[[:space:]]*0x[0-9a-fA-F]+(,[[:space:]]*0x[0-9a-fA-F]+)*[[:space:]]*\};?/{ \/*SPIRV_BYTES*\/ };/g'; }

for vs in "$FIXTURES"/*.vert; do
    name=$(basename "$vs" .vert)
    fs="$FIXTURES/$name.frag"
    expected="$FIXTURES/$name.rlvk.h.expected"
    [ -f "$fs" ] || { echo "skip $name: no .frag"; continue; }
    [ -f "$expected" ] || { echo "skip $name: no .expected"; continue; }

    glslc -o "$OUT/$name.vert.spv" "$vs"
    glslc -o "$OUT/$name.frag.spv" "$fs"
    "$TOOL" -v "$OUT/$name.vert.spv" -f "$OUT/$name.frag.spv" \
            -n "$name" -o "$OUT/$name.rlvk.h"

    if diff -u <(normalize <"$expected") <(normalize <"$OUT/$name.rlvk.h"); then
        echo "PASS $name"
    else
        echo "FAIL $name"
        FAIL=$((FAIL+1))
    fi
done

exit $FAIL
