#!/usr/bin/sh

cmake -S . -B build -DGGML_VULKAN=ON -DGGML_NATIVE=ON
cmake --build build -j"$(nproc)"
