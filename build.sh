#!/bin/bash

set -e

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel


