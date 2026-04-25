#!/bin/bash
# update_golden.sh - Regenerate every <name>.rlvk.h.expected from current tool output.
# Use after a deliberate tool change. Hand-review the diff before committing.
set -e

DIR=$(dirname "$0")
FIXTURES="$DIR/fixtures"
OUT="$DIR/out"
TOOL="$DIR/../rlvk-shdc"

mkdir -p "$OUT"

for vs in "$FIXTURES"/*.vert; do
    name=$(basename "$vs" .vert)
    fs="$FIXTURES/$name.frag"
    [ -f "$fs" ] || continue
    # Skip expect-fail fixtures (they have *.stderr.expected, not *.rlvk.h.expected)
    if [ -f "$FIXTURES/$name.stderr.expected" ]; then
        echo "skip $name (stderr fixture)"
        continue
    fi

    glslc -o "$OUT/$name.vert.spv" "$vs"
    glslc -o "$OUT/$name.frag.spv" "$fs"
    "$TOOL" -v "$OUT/$name.vert.spv" -f "$OUT/$name.frag.spv" \
            -n "$name" -o "$FIXTURES/$name.rlvk.h.expected"
    echo "regen $name"
done
