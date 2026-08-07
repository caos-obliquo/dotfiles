# wclipmenu

wclipmenu is a clipboard picker for Wayland. It shows the clipboard history of
the kaprica daemon in a wmenu menu — text entries or image entries with
thumbnail previews — and copies the selection back to the clipboard. It runs
on demand from a keybind, picks once and exits: no resident process, no
polling, no bloat.

## Building

wclipmenu needs no libraries beyond libc. At runtime it expects:

* [kaprica](https://github.com/ArtsyMacaw/kaprica) — `kapd` daemon + `kapc`
  CLI (installed at /usr/local/bin/kapc)
* [wmenu-caos](https://github.com/caos-obliquo/wmenu-caos) — the wmenu fork
  with `[img:]` PNG thumbnail rendering (image picking)
* [ImageMagick](https://imagemagick.org/) — thumbnail rendering (`magick`)
* [dwl](https://codeberg.org/dwl/dwl) — optional, provides the keybinds

Afterwards, run:

```
make
make install
```

## Usage

Run `wclipmenu` to pick from the text history, `wclipmenu image` to pick from
the image history. Press Enter to copy the selection, Esc to cancel. The
filter is case-insensitive; arrow keys page through the whole history (kapd
keeps up to 10000 entries).

See the man page (`man wclipmenu`) for the full synopsis and options.

Typical dwl keybinds:

```
Super+P        wclipmenu
Super+Shift+P  wclipmenu image
```

Screenshots taken with Super+S / Super+Shift+S show up in the image picker on
their own, because kapd observes the `wl-copy` selection.

## Comparison

* [clipmenu](https://github.com/cdown/clipmenu) — the dmenu-based original
  this is modeled on; X11-centric, uses clipnotify.
* [cclip](https://github.com/erebe/cclip) — Rust Wayland clipboard manager;
  resident daemon, different selection model.
* [kaprica](https://github.com/ArtsyMacaw/kaprica) — the daemon/CLI backend
  this builds on; not a picker itself.

## Credits

* [wmenu](https://sr.ht/~adnano/wmenu/) — the picker this project drives
* [kaprica](https://github.com/ArtsyMacaw/kaprica) — clipboard daemon + CLI
* [wmenu-caos](https://github.com/caos-obliquo/wmenu-caos) — the wmenu fork
  with image thumbnails
* [wl-clipboard](https://github.com/bugaevc/wl-clipboard) — Wayland clipboard
  utilities (`wl-copy`)
* [dwl](https://codeberg.org/dwl/dwl) — the window manager / keybind host
* [clipmenu](https://github.com/cdown/clipmenu) — the original this is modeled
  on
  
