#!/bin/bash

set -e

# Configure with vcpkg
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --parallel
