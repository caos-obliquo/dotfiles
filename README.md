# dwl + dwlb-geometry setup

## Components

- dwl: Wayland compositor
- dwlb-geometry: Status bar that exports geometry to /tmp/dwlb-geometry
- wmenu-dwlb: Application launcher that reads bar geometry to position itself

## How it works

dwlb-geometry writes bar geometry (x, width, height) to /tmp/dwlb-geometry on every render.
wmenu-dwlb reads this file to position itself in the bar's title section.

## Installation

1. Compile dwl first:
```bash
cd ~/builds/dwl
cp config.h from dotfiles
make clean && make && sudo make install
```

2. Compile dwlb-geometry (requires dwl protocols):
```bash
cd ~/builds/dwlb-geometry
cp config.h from dotfiles
make clean && make && sudo make install
```

3. Compile wmenu-dwlb:
```bash
cd ~/builds/wmenu-dwlb
cd build && meson .. && ninja && sudo ninja install
```

4. Install scripts:
```bash
cp home/.local/bin/* ~/.local/bin/
chmod +x ~/.local/bin/*.sh
```

## Usage

Start dwl:
```bash
~/.local/bin/start-dwl.sh
```

Start status bar:
```bash
~/.local/bin/start-status.sh
```

## Dependencies

- grim, slurp: Screenshots
- wl-clipboard: Clipboard support
- pamixer: Volume control
- swaybg: Wallpaper
