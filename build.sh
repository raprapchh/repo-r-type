#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VCPKG_DIR="${SCRIPT_DIR}/vcpkg"
VCPKG_TOOLCHAIN="${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"

if [ ! -f "$VCPKG_TOOLCHAIN" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    "$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics
fi

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN"

cmake --build build --parallel

mkdir -p bin/linux/client
mkdir -p bin/linux/server
mkdir -p bin/linux/config

cp -r client/assets bin/linux/client/ 2>/dev/null || true
cp -r client/fonts bin/linux/client/ 2>/dev/null || true
cp -r client/sprites bin/linux/client/ 2>/dev/null || true
cp -r server/assets bin/linux/server/ 2>/dev/null || true
cp -r config/* bin/linux/config/ 2>/dev/null || true
