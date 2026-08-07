# dotfiles

My daily driver config, affectionately called DraculaWL by me. HP EliteBook 645 G11 (AMD based), Arch Linux, DWL on Wayland.

## Stack

| Tool              | Role                                                                             |
| ----------------- | -------------------------------------------------------------------------------- |
| **dwl**           | Wayland compositor (patched fork)                                                |
| **dwlb**          | Status bar (dwl fork) - fed by `dwlb-status.sh | dwlb -status-stdin`; writes geometry + colors to `/tmp/dwlb-geometry` on every render |
| **wmenu-caos**    | Menus (fork of wmenu-dwlb, binary stays `wmenu`) - app launcher + picker host; reads `/tmp/dwlb-geometry`, patched with `-c` centering, `-B` border, PNG thumbnail rendering (`[img:path]` rows) |
| **wclipmenu**     | Clipboard picker - `wclipmenu` (text, Super+P) and `wclipmenu image` (thumbnails, Super+Shift+P) |
| **kaprica**       | Clipboard history store - `kapd` watcher daemon + `kapc` CLI (search/copy/paste, file-based db) |
| **foot**          | Terminal                                                                          |
| **nvim**          | Editor                                                                            |
| **tmux**          | Multiplexer                                                                       |
| **zsh**           | Shell, with atuin for history                                                     |
| **mako**          | Notifications                                                                     |
| **htop**          | System monitor                                                                    |
| **ccze**          | Log colorizer                                                                     |
| **zathura**       | PDF viewer                                                                        |
| **widle**         | Idle daemon - `widle -t 300 wlock` locks after 5min idle                          |
| **wlock**         | Screen locker (ext-session-lock) - solid color rendered via wl_shm                |
| **wawa**          | Wallpaper - swaybg replacement; `wawa fill ~/walls/wall3.jpg` (fill mode)         |
| **pass**          | Password manager                                                                  |

Everything uses the [Dracula](https://draculatheme.com) color palette. Where an official theme exists it's used directly; where it doesn't, the colors are matched manually in the config.

## How the bar/launcher integration works

`dwlb` (the status bar) writes to `/tmp/dwlb-geometry` on every frame:

```
<middle_x> <middle_width> <bar_height> <bg_color> <fg_color>
```

`wmenu-caos` reads this file to position and color itself. Two modes:

- `-t` - positions wmenu inside the bar's title section (app launcher)
- `-c` - centers wmenu on screen (clipboard picker, passmenu)

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
| Super+Shift+L        | Lock screen (wlock)                |
| Ctrl+Up              | Atuin shell history search         |
| vi `k` (normal mode) | Atuin shell history search         |

## Setup

Clone the repo, then copy `home/` contents to `~/`:

```bash
cp -r home/. ~/
chmod +x ~/.local/bin/*.sh
```

Then clone and build the forks. Each one expects its `config.h` (and in wmenu-caos's case, `menu.c`) to be copied from this repo before building:

```bash
# dwl
git clone <your-dwl-fork> ~/builds/dwl
cd ~/builds/dwl
cp ~/dotfiles/builds/dwl/config.h .
make clean && make && sudo make install

# dwlb (status bar - fork writes /tmp/dwlb-geometry itself)
git clone <your-dwlb-fork> ~/builds/dwlb-geometry
cd ~/builds/dwlb-geometry
make clean && make && sudo make install

# wmenu-caos (fork of wmenu-dwlb, binary stays wmenu)
git clone <your-wmenu-caos-fork> ~/builds/wmenu-caos
cd ~/builds/wmenu-caos
cp ~/dotfiles/builds/wmenu-caos/config.h .
cp ~/dotfiles/builds/wmenu-caos/menu.c .
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

# wlock (screen locker)
git clone <your-wlock-fork> ~/builds/wlock
cd ~/builds/wlock
make clean && make && sudo make install  # installs suid-root (4755)

# wawa (wallpaper)
git clone <your-wawa-fork> ~/builds/wawa
cd ~/builds/wawa
make clean && make && sudo make install
```

Start everything:

```bash
start-dwl.sh
```

kapd (the clipboard watcher), dwlb, the status feed, the wallpaper (`wawa fill ~/walls/wall3.jpg`), and the idle→lock chain (`widle -t 300 wlock`) all start automatically from there.

## Dependencies

```bash
pacman -S grim slurp wl-clipboard pamixer atuin pass \
          foot mako htop ccze zathura zathura-pdf-mupdf widle
```

wmenu-caos needs cairo, pango, wayland, xkbcommon, and the `wlr-layer-shell-unstable-v1` protocol (meson + ninja). `wclipmenu image` needs ImageMagick (`magick`) for thumbnail generation.

## Notes

- the status feed, wallpaper, and idle→lock chain are all launched from `start-dwl.sh`
- wmenu uses `-t` for the bar-positioned launcher and `-c` for centered menus; `-B <color>` sets the 2px border, `-N`/`-n`/`-S`/`-s` set panel/selection colors (RRGGBBAA)
- kapd watches the clipboard and writes history; `kapc search -t text/plain -L` is the query wclipmenu uses
- widle and wlock are integrated - `widle -t 300 wlock` (in start-dwl.sh) locks after 5min idle; Super+Shift+L locks manually
