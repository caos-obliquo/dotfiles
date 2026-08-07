# dwlb-geometry

dwlb fork that exports bar geometry for external tools.

Fork of [dwlb](https://github.com/kolunmi/dwlb).

## What it does

Writes middle section geometry (x, width, height) to `/tmp/dwlb-geometry` on every render. Allows tools like wmenu to position themselves in the title area.

## Installation
```bash
make && sudo make install
```

Restart dwl after installation.

## Configuration

Colors defined in `config.h`. Standard dwlb configuration.

## Compatible tools

- [wmenu-dwlb](link-to-your-wmenu) - positions wmenu in title section
