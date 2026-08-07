# Changelog

All notable changes to this project are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/), and
this project adheres to [Semantic Versioning](https://semver.org/).

## [0.1.2] - 2026-08-05

### Fixed

- Malformed image paste: wl-copy's stdin was clobbered by /dev/null in the
  pipeline helper, advertising an empty image

## [0.1.1] - 2026-08-05

### Fixed

- Image picker pasted truncated images in browsers: copy-back now uses
  `kapc paste | wl-copy --foreground` (kapd serves only wlr-data-control;
  wl-copy also serves wl_data_device, which browsers read)

## [0.1.0] - 2026-08-04

First release.

### Added

- Text picker: pick from kaprica `text/plain` history via a wmenu menu,
  copy the selection back with `kapc copy`
- Image picker: `image/png` history with PNG thumbnails rendered through the
  wmenu-caos `[img:]` protocol (160x160 thumbs, cached, parallel forks)
- Subcommands: `list`, `image`, `copy`, `-h`
- Env overrides: `WCLIPMENU_WMENU`, `WCLIPMENU_LIMIT`, `WCLIPMENU_DB`
- Test harness: 6 C suites locking the kaprica CLI contract
- Man page (`wclipmenu.1`), sibling-style README, GitHub Actions CI
