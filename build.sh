#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
TARGET="${1:-all}"

cd "${SCRIPT_DIR}"

if [ ! -d "${BUILD_DIR}" ]; then
    mkdir -p "${BUILD_DIR}"
fi

cd "${BUILD_DIR}"

cmake ..

case "${TARGET}" in
    server)
        make r-type_server
        ;;
    client)
        make r-type_client
        ;;
    all|*)
        make
        ;;
esac

