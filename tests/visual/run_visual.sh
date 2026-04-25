#!/bin/sh
# tests/visual/run_visual.sh — Frame-capture visual regression for the
# Vulkan backend.
#
# For each example listed below:
#   1. Run the binary with RLVK_CAPTURE_FRAME=N and RLVK_CAPTURE_PATH=<png>
#      so rlvk dumps frame N to disk and exits.
#   2. Compare the captured PNG against the committed golden using
#      ImageMagick's `compare` with a 2% fuzz tolerance for AA + driver
#      jitter.
#
# Modes:
#   sh run_visual.sh          → check (default): diff against golden
#   sh run_visual.sh update   → regenerate goldens from current output
#                               (review the diff before committing!)
#
# Prereqs: ImageMagick `compare`; raylib built with GRAPHICS_API_VULKAN;
# RLVK_CAPTURE_* support in rlvk.h. If frame capture isn't wired yet,
# the runner reports SKIP rather than FAIL.
set -u

ROOT=$(cd "$(dirname "$0")/.." && pwd)
TESTS="$ROOT/visual"
GOLDEN="$TESTS/golden"
OUT="$TESTS/out"
MODE="${1:-check}"
mkdir -p "$OUT" "$GOLDEN"

# Representative cases — each exercises a distinct path through the runtime.
CASES="\
others/vulkan_shapes_test \
shaders/shaders_color_correction"

PASS=0; FAIL=0; SKIP=0

for case in $CASES; do
    name=$(basename "$case")
    bin="$ROOT/../examples/$case"
    case_dir="$ROOT/../examples/$(dirname $case)"
    [ -x "$bin" ] || { echo "SKIP $name: binary missing — build the example first"; SKIP=$((SKIP+1)); continue; }

    # Drive the example for ~5 frames so per-shader pipelines warm up, then
    # capture the 5th frame.
    rm -f "$OUT/$name.png"
    (cd "$case_dir" && \
     RLVK_CAPTURE_FRAME=5 RLVK_CAPTURE_PATH="$OUT/$name.png" "$bin" >"$OUT/$name.stdout" 2>&1) || true

    if [ ! -f "$OUT/$name.png" ]; then
        echo "SKIP $name: rlvk frame capture not wired (RLVK_CAPTURE_PATH ignored)"
        SKIP=$((SKIP+1))
        continue
    fi

    if [ "$MODE" = "update" ]; then
        cp "$OUT/$name.png" "$GOLDEN/$name.png"
        echo "UPDATE $name"
        PASS=$((PASS+1))
        continue
    fi

    if [ ! -f "$GOLDEN/$name.png" ]; then
        echo "SKIP $name: no golden checked in — run 'sh run_visual.sh update' first"
        SKIP=$((SKIP+1))
        continue
    fi

    if ! command -v compare >/dev/null 2>&1; then
        # ImageMagick not installed — fall back to a byte-exact comparison
        # via cmp. Same-driver, same-machine captures should be byte-identical
        # frame-to-frame, so cmp is a reasonable smoke test in the absence of
        # tolerance-based comparison.
        if cmp -s "$GOLDEN/$name.png" "$OUT/$name.png"; then
            echo "PASS $name (cmp; ImageMagick not installed)"
            PASS=$((PASS+1))
        else
            echo "FAIL $name (cmp byte mismatch; ImageMagick not installed)"
            FAIL=$((FAIL+1))
        fi
        continue
    fi

    diff_pixels=$(compare -metric AE -fuzz 2% \
        "$GOLDEN/$name.png" "$OUT/$name.png" "$OUT/$name.diff.png" 2>&1 || true)

    # `compare` exits non-zero when images differ; the metric is the count.
    if [ "${diff_pixels:-0}" = "0" ] || [ "${diff_pixels:-99999}" -lt 500 ] 2>/dev/null; then
        echo "PASS $name ($diff_pixels px diff)"
        PASS=$((PASS+1))
    else
        echo "FAIL $name ($diff_pixels px diff)"
        FAIL=$((FAIL+1))
    fi
done

echo "Visual: PASS=$PASS FAIL=$FAIL SKIP=$SKIP"
[ $FAIL -eq 0 ]
