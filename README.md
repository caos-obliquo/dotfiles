# dotfiles

My daily driver config, affectionately called DraculaWL by me. HP EliteBook 645 G11 (AMD based), Arch Linux, DWL on Wayland.

## Stack

| Tool              | Role                                                                             |
| ----------------- | -------------------------------------------------------------------------------- |
| **dwl**           | Wayland compositor (patched fork)                                                |
| **dwlb-geometry** | Status bar — writes geometry + colors to `/tmp/dwlb-geometry` on every render    |
| **wmenu-dwlb**    | Menus — app launcher + picker host; reads `/tmp/dwlb-geometry`, patched with `-c` centering, `-B` border, PNG thumbnail rendering (`[img:path]` rows) |
| **wclipmenu**     | Clipboard picker — `wclipmenu` (text, Super+P) and `wclipmenu image` (thumbnails, Super+Shift+P) |
| **kaprica**       | Clipboard history store — `kapd` watcher daemon + `kapc` CLI (search/copy/paste, file-based db) |
| **foot**          | Terminal                                                                          |
| **nvim**          | Editor                                                                            |
| **tmux**          | Multiplexer                                                                       |
| **zsh**           | Shell, with atuin for history                                                     |
| **mako**          | Notifications                                                                     |
| **htop**          | System monitor                                                                    |
| **ccze**          | Log colorizer                                                                     |
| **zathura**       | PDF viewer                                                                        |
| **widle**         | Idle daemon                                                                       |
| **wlock**         | Screen locker                                                                     |
| **pass**          | Password manager                                                                  |

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

## How the clipboard picker works

`kapd` watches the Wayland clipboard and stores entries into its history db. `wclipmenu` queries it on demand via `kapc` and pipes the results into wmenu:

- **Text**: `wclipmenu` → `kapc search -t text/plain -L -l 100 | wmenu` → pick → `kapc copy -i <id>`
- **Images**: `wclipmenu image` → `kapc search -t image/png` → magick-resized 96px thumbnails emitted as `[img:<path>]row` lines → wmenu renders them → pick → `kapc copy -i <id>`

The wmenu fork renders PNG thumbs from `[img:path]text` stdin rows (64px, 96px row height) and centers itself both axes with `-c`. Corrupt/truncated clipboard images degrade to plain text rows that remain pickable.

## Keybindings worth knowing

| Key                  | Action                             |
| -------------------- | ---------------------------------- |
| Super+D              | App launcher (wmenu in bar)        |
| Super+P              | Text clipboard history (wclipmenu) |
| Super+Shift+P        | Image clipboard history (wclipmenu image, thumbnails) |
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

# wclipmenu
git clone <your-wclipmenu-fork> ~/builds/wclipmenu
cd ~/builds/wclipmenu
make clean && make && sudo make install

# kaprica (clipboard history store)
git clone <kaprica-repo> ~/builds/kaprica
cd ~/builds/kaprica
# build per its meson instructions, installs kapd + kapc to /usr/local/bin
```

Start everything:

```bash
start-dwl.sh
```

kapd (the clipboard watcher), dwlb, and the status feed all start automatically from there.

## Dependencies

```bash
pacman -S grim slurp wl-clipboard pamixer swaybg atuin pass \
          foot mako htop ccze zathura zathura-pdf-mupdf widle
```

wmenu-dwlb needs cairo, pango, wayland, xkbcommon, and the `wlr-layer-shell-unstable-v1` protocol (meson + ninja). `wclipmenu image` needs ImageMagick (`magick`) for thumbnail generation.

## Notes

- `start-status.sh` is deprecated — the status feed is now launched from `start-dwl.sh`
- wmenu uses `-t` for the bar-positioned launcher and `-c` for centered menus; `-B <color>` sets the 2px border, `-N`/`-n`/`-S`/`-s` set panel/selection colors (RRGGBBAA)
- kapd watches the clipboard and writes history; `kapc search -t text/plain -L` is the query wclipmenu uses
- wlock and widle are integrated — idle timeout triggers the lock screen
