#!/usr/bin/env bash
# scripts/build.sh — Build LazyParser on Arch Linux
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build"

echo "=== LazyParser build ==="
echo "Source root : ${ROOT}"
echo "Build dir   : ${BUILD_DIR}"
echo

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${ROOT}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=OFF

ninja -j"$(nproc)"

echo
echo "=== Build successful! ==="
echo "Binaries in: ${BUILD_DIR}/bin/"
ls "${BUILD_DIR}/bin/"
