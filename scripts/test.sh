#!/usr/bin/env bash
# scripts/test.sh — Run LazyParser tests
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build"

if [ ! -d "${BUILD_DIR}" ]; then
    echo "Build directory not found. Run scripts/build.sh first."
    exit 1
fi

echo "=== LazyParser test run ==="
cd "${BUILD_DIR}"
ctest --output-on-failure --parallel "$(nproc)"
