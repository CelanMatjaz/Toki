#!/bin/bash
set -e

clear
mkdir -p build
pushd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DTOKI_USE_GLFW=ON -GNinja
cmake --build . -j$(nproc)
popd
