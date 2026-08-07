# wmenu-dwlb

wmenu fork that positions itself in dwlb's title section.

Fork of [wmenu](https://git.sr.ht/~adnano/wmenu).

## What it does

Positions wmenu in the middle section of dwlb by reading bar geometry from `/tmp/dwlb-geometry`. Use `-t` flag to enable.

## Requirements

- Modified dwlb that writes geometry file (see [dwlb-geometry fork](link-to-your-dwlb))
- dwl compositor

## Installation
```bash
cd build && meson .. && ninja
sudo ninja install
```

## Usage
```bash
wmenu-run -t
```

In dwl config.h:
```c
static const char *menucmd[] = { "wmenu-run", "-t", "-i", NULL };
```

## Configuration

Colors defined in `menu.c`. Modify to match your dwlb theme.
