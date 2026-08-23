# Changelog

All notable changes to this project are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/), and
this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Fixed

- First frame renders at the final pill position: wmenu now waits for the
  compositor's bar_geometry (second commit + roundtrip) before its first
  render, instead of drawing one fallback frame at the default position

## [0.2.1] - 2026-08-05

### Fixed

- Panel growing on every ArrowDown: height now computed from the `-l` lines
  setting, not a hardcoded 15-item cap
- Panel transparency fading to opaque: buffers cleared before each repaint
  instead of compositing over the previous frame

## [0.2.0] - 2026-08-04

First release.

### Added

- `[img:]` PNG thumbnail protocol: images rendered 128px on the longest side,
  vertically centered in 160px rows, prefix stripped before stdout
- Vertical image list layout (paged, `-l` entries per page)
- dwlb-style positioning: `-t` top-center title bar (reads
  `/tmp/dwlb-geometry`), `-c` centered
- `wmenu-run` companion binary

### Changed

- Project renamed from wmenu-dwlb to **wmenu-caos**
- `config.h` tracked with the dwlb palette (`dwlb_middle_bg`/`fg`,
  `dwlb_bar_height` 30)
- Zero-warning builds (`-Wall -Wextra -Werror`, c11)

### Fixed

- Transparent image-picker background (`CAIRO_OPERATOR_OVER`)
- Thumbnail vertical centering in rows (actual drawn height, not constant)
