# wmenu-caos

dmenu-style Wayland picker with image thumbnail previews and dwl-IPC
positioning. Fork of [wmenu](https://sr.ht/~adnano/wmenu/), tuned for use with
[dwl](https://codeberg.org/dwl/dwl).

## Features

- `[img:<path>]` PNG thumbnail protocol: an item line may carry an inline
  image. The PNG is scaled to 128px on its longest side and drawn vertically
  centered in a 160px row; the `[img:]` prefix is stripped before the selection
  is printed to stdout, so callers receive the plain text line.
- `-t` top-center (title-bar) positioning: reads the dwlb active-monitor
  geometry from `/tmp/dwlb-geometry` when present, so the bar overlays the
  correct monitor.
- `-c` centered positioning.
- Vertical lists page with arrow keys: `-l` entries per page, image rows 160px tall.
- dwlb-matching color defaults in `config.h`.

## Requirements

- wayland-client, wayland-protocols, xkbcommon
- cairo, pango, pangocairo
- meson, ninja, a C11 compiler

## Build and install

	meson setup build
	ninja -C build
	sudo ninja -C build install

Builds `wmenu` (the picker) and `wmenu-run` (run-mode helper). Default prefix is
`/usr/local`; change it with `meson setup build -Dprefix=$HOME/.local`.

## Options

	-b        position menu at the bottom
	-c        position menu centered
	-t        position menu at the top center (title-bar mode)
	-i        case-insensitive matching
	-P        password mode (hide input)
	-h        print usage
	-v        print version
	-f FONT   pango font description
	-l LINES  number of lines to show
	-o X      offset of the menu from the prompt
	-p PROMPT prompt text
	-N COLOR  normal background color (#RRGGBBAA)
	-n COLOR  normal foreground color
	-M COLOR  selection background color
	-m COLOR  selection foreground color
	-S COLOR  selection highlight background (#RRGGBBAA)
	-s COLOR  selection highlight foreground
	-B COLOR  menu background color
	-w WIDTH  menu width in pixels

## Keyboard shortcuts

Hardcoded in `menu.c`; no config. Emacs-style bindings with Ctrl/Alt.

### Navigation

| Key | Action |
|---|---|
| `Up` / `Ctrl+p` | previous match (or cursor up) |
| `Down` / `Ctrl+n` | next match (or cursor down) |
| `PgUp` / `Alt+k` | previous page |
| `PgDn` / `Alt+j` | next page |
| `Home` / `Ctrl+a` / `Alt+g` | first match / start of line |
| `End` / `Ctrl+e` / `Alt+G` | last match / end of line |
| `Left` / `Ctrl+b` | cursor left |
| `Right` / `Ctrl+f` | cursor right |
| `Alt+b` / `Alt+f` | word left / word right |

### Editing

| Key | Action |
|---|---|
| `Backspace` / `Ctrl+h` | delete char left |
| `Delete` / `Ctrl+d` | delete char right |
| `Ctrl+k` | delete to end of line |
| `Ctrl+u` | delete to start of line |
| `Ctrl+w` | delete word left |
| `Ctrl+Y` | paste clipboard |
| `Tab` / `Ctrl+i` | complete input from selected item |

### Select / cancel

| Key | Action |
|---|---|
| `Return` / `Ctrl+j` / `Ctrl+m` / KP_Enter | submit selected item |
| `Shift+Return` | submit typed input (ignore selection) |
| `Escape` / `Ctrl+c` / `Ctrl+g` / `Ctrl+[` | cancel |
| printable | insert character (live filtering) |

## Image thumbnails

Feed items as `[img:/path/to.png]<text>` lines. The thumbnail is scaled to
128px on its longest side (a wide 16:9 image draws ~72px tall) and centered in a
160px row, keeping mixed-aspect clips aligned. The selection written to stdout
is the plain `<text>` line, `[img:]` prefix stripped.

The [wclipmenu](https://github.com/caos-obliquo/wclipmenu) clipboard picker uses
this protocol to show clipboard image history:

	wclipmenu image

## Configuration

Colors live in `config.h` (tracked; defaults match the dwlb active-monitor
palette):

	dwlb_middle_bg    bar mode (-t) background, #RRGGBBAA
	dwlb_middle_fg    bar mode (-t) foreground
	dwlb_bar_height   bar mode (-t) height in pixels

Edit `config.h` and rebuild.

## Credits

- [wmenu](https://sr.ht/~adnano/wmenu/): upstream project
- [wclipmenu](https://github.com/caos-obliquo/wclipmenu): consumer of the
  `[img:]` protocol
- [dwl](https://codeberg.org/dwl/dwl): the window manager this targets

## caos fork additions

- **`POSITION_TOP_CENTER` pill over the dwl bar** (`-t` / `wmenu-run -t`,
  dwl keybind `Super+d`): subscribes to `zdwl_ipc_manager_v2` and repositions
  onto the `bar_geometry` event dwl sends on every bar draw. Margins are set
  so the pill sits exactly over the bar's **middle title area**
  (left = `middle_x`, right = rest). Height = bar height + list rows, and is
  drift-free across redraws (list height captured after render).
- **Colors from IPC**: the pill adopts dwl's `SchemeNorm` bg/fg from the
  event (normal/prompt bg = bar bg; normal/prompt fg = bar fg; selection
  colors swapped), so it always matches the bar.
- **No geometry files**: the old `/tmp/dwl-bar-geometry` file fallback was
  removed — IPC is the only positioning path. Without IPC, `-t` falls back
  to a centered `wmenu_width` window.
- **Full event listeners**: all `zdwl_ipc_output_v2` events have registered
  (noop) listeners — libwayland aborts on any NULL listener slot.
