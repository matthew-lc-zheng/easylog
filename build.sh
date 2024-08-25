#!/bin/bash
set -euo pipefail
if [[ -d output ]]; then
    rm -rf output
else
    mkdir output
fi
if [[ -d build ]]; then
    rm -rf build
fi
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=output
cmake --build build --parallel $(nproc) --target install --config Release
