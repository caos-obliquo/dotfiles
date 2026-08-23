#!/bin/sh
# Local CI-equivalent build: cd into repo root and build like GitHub Actions does.
set -e
cd "$(dirname "$0")/.."
make
