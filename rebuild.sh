#!/usr/bin/env bash
set -euo pipefail

# Usage: ./rebuild.sh [Debug|Release]
# Defaults to Release if you don’t pass anything.
BUILD_TYPE=${1:-Release}

echo ">>> Cleaning old build artifacts"
rm -rf build/ CMakeCache.txt CMakeFiles/

echo ">>> Configuring for ${BUILD_TYPE}"
# Force a Makefile generator to avoid stale Ninja caches.
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

echo ">>> Building (${BUILD_TYPE})"
# --config is a no-op for single-config generators (Makefile/Ninja),
#  but required for multi-config ones (Visual Studio, Xcode).
cmake --build build --config ${BUILD_TYPE}

echo ">>> Done."
