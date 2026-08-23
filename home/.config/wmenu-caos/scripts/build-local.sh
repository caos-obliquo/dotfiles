#!/bin/sh
# Local CI-equivalent build: cd into repo root and build like GitHub Actions does.
set -e
cd "$(dirname "$0")/.."
meson setup build 2>/dev/null || meson setup --wipe build
ninja -C build
