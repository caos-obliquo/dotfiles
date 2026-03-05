# dotfiles

My daily driver config, affectionately called DraculaWL by me. HP EliteBook 645 G11 (AMD based), Arch Linux, DWL on Wayland.

## Stack

| Tool              | Role                                                                             |
| ----------------- | -------------------------------------------------------------------------------- |
| **dwl**           | Wayland compositor (patched fork)                                                |
| **dwlb-geometry** | Status bar — writes geometry + colors to `/tmp/dwlb-geometry` on every render    |
| **wmenu-dwlb**    | App launcher and menus — reads `/tmp/dwlb-geometry` to position and color itself |
| **foot**          | Terminal                                                                         |
| **nvim**          | Editor                                                                           |
| **tmux**          | Multiplexer                                                                      |
| **zsh**           | Shell, with atuin for history                                                    |
| **mako**          | Notifications                                                                    |
| **htop**          | System monitor                                                                   |
| **ccze**          | Log colorizer                                                                    |
| **zathura**       | PDF viewer                                                                       |
| **widle**         | Idle daemon                                                                      |
| **wlock**         | Screen locker                                                                    |
| **cliphist**      | Clipboard history                                                                |
| **pass**          | Password manager                                                                 |

Everything uses the [Dracula](https://draculatheme.com) color palette. Where an official theme exists it's used directly; where it doesn't, the colors are matched manually in the config.

## How the bar/launcher integration works

`dwlb-geometry` writes to `/tmp/dwlb-geometry` on every frame:

```
<middle_x> <middle_width> <bar_height> <bg_color> <fg_color>
```

`wmenu-dwlb` reads this file to position and color itself. Two modes:

- `-t` — positions wmenu inside the bar's title section (app launcher)
- `-c` — centers wmenu on screen (clipboard picker, passmenu)

This means wmenu always looks like part of the bar without any runtime color flags.

## Keybindings worth knowing

| Key                  | Action                             |
| -------------------- | ---------------------------------- |
| Super+D              | App launcher (wmenu in bar)        |
| Super+P              | Clipboard history (wmenu centered) |
| Super+S              | Screenshot area                    |
| Ctrl+Up              | Atuin shell history search         |
| vi `k` (normal mode) | Atuin shell history search         |

## Setup

Clone the repo, then copy `home/` contents to `~/`:

```bash
cp -r home/. ~/
chmod +x ~/.local/bin/*.sh
```

Then clone and build the forks. Each one expects its `config.h` (and in wmenu-dwlb's case, `menu.c`) to be copied from this repo before building:

```bash
# dwl
git clone <your-dwl-fork> ~/builds/dwl
cd ~/builds/dwl
cp ~/dotfiles/builds/dwl/config.h .
make clean && make && sudo make install

# dwlb-geometry
git clone <your-dwlb-geometry-fork> ~/builds/dwlb-geometry
cd ~/builds/dwlb-geometry
cp ~/dotfiles/builds/dwlb-geometry/config.h .
make clean && make && sudo make install

# wmenu-dwlb
git clone <your-wmenu-dwlb-fork> ~/builds/wmenu-dwlb
cd ~/builds/wmenu-dwlb
cp ~/dotfiles/builds/wmenu-dwlb/config.h .
cp ~/dotfiles/builds/wmenu-dwlb/menu.c .
rm -rf build && meson setup build && ninja -C build && sudo ninja -C build install
```

Start everything:

```bash
start-dwl.sh
```

dwlb, the status feed, and cliphist all start automatically from there.

## Dependencies

```bash
pacman -S grim slurp wl-clipboard pamixer swaybg cliphist atuin pass \
          foot mako htop ccze zathura zathura-pdf-mupdf widle
```

## Notes

- `start-status.sh` is deprecated — the status feed is now launched from `start-dwl.sh`
- wmenu uses `-t` for the bar-positioned launcher and `-c` for centered menus
- cliphist daemon starts automatically on login via `start-dwl.sh`
- wlock and widle are integrated — idle timeout triggers the lock screen
