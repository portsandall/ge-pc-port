#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/work/goldeneye-pc-port"

echo "== GEAND Android preflight =="
echo "repo: $ROOT"

: "${ANDROID_NDK_HOME:?ANDROID_NDK_HOME must point to an Android NDK}"

HOST_TAG=linux-x86_64
TOOLCHAIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$HOST_TAG"
CLANG="$TOOLCHAIN/bin/aarch64-linux-android29-clang"
CLANGXX="$TOOLCHAIN/bin/aarch64-linux-android29-clang++"

test -x "$CLANG"
test -x "$CLANGXX"
command -v cmake
command -v ninja
command -v python3

echo
"$CLANG" --version | head -n 1
cmake --version | head -n 1
ninja --version

echo
echo "== Source portability audit =="

INHERITS_COUNT="$(grep -R --include='*.h' --include='*.c' -E '(^|[[:space:]])inherits[[:space:]]+[A-Za-z_]' "$SRC" | wc -l | tr -d ' ')"
echo "inherits declarations: $INHERITS_COUNT"

if grep -n -- '-fplan9-extensions' "$SRC/CMakeLists.txt"; then
  echo "GCC Plan-9 extension dependency: present"
fi

if grep -n -- '-fno-pie\|-no-pie\|-Ttext-segment' "$SRC/CMakeLists.txt"; then
  echo "Linux fixed-address/non-PIE logic: present"
fi

cat > /tmp/geand-android-smoke.c <<'EOF'
#include <android/api-level.h>
#include <stdint.h>
int geand_android_smoke(void) { return (int)sizeof(uintptr_t) * 8 + __ANDROID_API__; }
EOF

"$CLANG" -fPIC -c /tmp/geand-android-smoke.c -o /tmp/geand-android-smoke.o
file /tmp/geand-android-smoke.o

echo
echo "Android NDK arm64-v8a toolchain is operational."
echo "Next source milestone: remove GCC-only struct inheritance dependency while preserving layouts."
