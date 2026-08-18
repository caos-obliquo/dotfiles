# dotfiles — DraculaWL

**Purpose: one keyboard-first Wayland desktop, fully declarative and
reproducible.** Every config in this repo mirrors exactly what runs on the
machine (HP EliteBook 645 G11, Arch Linux, dwl). The whole desktop is driven
from the keyboard — the mouse is optional.

Each piece of the stack is **self-documenting**: every folder has its own
`README.md` with what it does, what's configured, and its keybinds. This
README is the map, not the encyclopedia.

## The stack

| Tool | Role | Layer |
|---|---|---|
| **dwl** | Wayland compositor (patched fork: freeze fix + 7 patches) | WM |
| **dwlb** | Status bar — geometry/colors to `/tmp/dwlb-geometry` | UI |
| **wmenu-caos** | Menus (launcher, pickers, passmenu) | UI |
| **wclipmenu** | Clipboard picker (text + image thumbnails) | UI |
| **kaprica** | Clipboard history store (`kapd` daemon + `kapc` CLI) | data |
| **mako** | Notifications | UI |
| **wlock** | Screen locker (ext-session-lock) | security |
| **widle** | Idle daemon — locks after 5 min idle | security |
| **wawa** | Wallpaper (swaybg replacement) | UI |
| **kanshi** | Auto monitor profiles on dock/undock | hardware |
| **foot** | Terminal (Dracula, transparent) | UI |
| **tmux** | Multiplexer (vim-grammar binds) | shell |
| **zsh** | Shell — vi-mode, Dracula prompt, atuin history | shell |
| **atuin** | History search (`Ctrl+Up` / vi-`k`) | shell |
| **ccze** | Log colorizer (Dracula semantic scheme) | shell |
| **nvim** | Editor | apps |
| **zathura** | PDF viewer | apps |
| **htop / btop** | System monitors | apps |
| **pass** | Password manager (via `passmenu`) | security |
| **playerctl** | Media control (play/pause/next/prev) | media |
| **youtui** | YT Music TUI — native Last.fm scrobbler (Genius/MusicBrainz/ListenBrainz metadata) | media |

Everything is **Dracula** — official themes where they exist, hand-matched
otherwise.

## Keyboard layers (how the whole desktop responds)

The keyboard is organized in **layers**, each with the same vim grammar:

| Layer | Tool | `j/k/h/l` | leader |
|---|---|---|---|
| WM | dwl | window focus / master size | `Super` |
| Multiplexer | tmux | pane navigation | `Ctrl+Space` |
| Shell | zsh | vi-mode | `Esc` |
| Editor | nvim | native vim | — |
| Menus | wmenu | navigate matches | `Ctrl` |
| Browser | **Vimium** (vimium-c on Waterfox) | scroll / hints / tabs | `f` hints |

> 🏆 **Honor mention — Vimium.** Not in this repo (it's a browser extension,
> not a config file), but fundamental to the keyboard-warrior setup: `j/k`
> scroll, `f` link hints, `H/L` history, `J/K` tabs. It completes the promise
> that *nothing on this desktop requires a mouse* — including the browser.
> Install vimium-c on Waterfox and the loop is closed.

**Complete keybind reference:** [`docs/KEYBINDS.md`](docs/KEYBINDS.md)
**Deployment script:** [`docs/AUTORICE-DEPLOY.md`](docs/AUTORICE-DEPLOY.md)

## Repository layout

```
home/.config/<tool>/   → mirrors ~/.config; each folder self-documenting
                        (README.md + config + keybinds inside)
builds/<tool>/         → per-fork build configs (config.h, menu.c, …)
                        copied into the source before `make`
docs/                  → cross-cutting references (KEYBINDS.md, AUTORICE-DEPLOY.md)
autorice-deploy.sh     → one-shot: deploys home/, clones + builds every fork
start-dwl.sh           → session entry (bar, wallpaper, clipboard, idle→lock)
```

## How the bar/launcher integration works

`dwlb` writes `<middle_x> <middle_width> <bar_height> <bg> <fg>` to
`/tmp/dwlb-geometry` every frame; `wmenu-caos` reads it to position and
color itself — `-t` parks the launcher in the bar's title section, `-c`
centers pickers on screen. wmenu always looks like part of the bar with zero
runtime color flags.

## How the clipboard picker works

`kapd` watches the clipboard → history db. `wclipmenu` queries it on demand
via `kapc` and pipes into wmenu:
- **Text**: `kapc search -t text/plain -L -l 100` → pick → `kapc copy -i <id>`
- **Images**: PNGs → magick-resized 96px thumbnails → `[img:path]` rows →
  wmenu renders them → pick → copy

Corrupt images degrade to plain text rows that stay pickable.

## Setup

**Option A — automated (recommended):**

```bash
./autorice-deploy.sh
```

[Full documentation →](docs/AUTORICE-DEPLOY.md)

Clones every fork, copies the per-fork build configs from `builds/`, builds,
installs, and copies `home/` → `~/`.

**Option B — manual:** copy `home/.` to `~/`, then per fork:
clone → copy its `builds/<tool>/config.h` → `make && sudo make install`
(exact commands for each tool live in their vendored folder READMEs).

Start the session:

```bash
start-dwl.sh
```

kapd, dwlb + the status feed, wawa wallpaper, kanshi (if installed), and the
`widle -t 300 wlock` idle→lock chain all start from there.

## Dependencies

```bash
pacman -S grim slurp wl-clipboard pamixer playerctl atuin pass \
          foot mako htop ccze zathura zathura-pdf-mupdf widle kanshi
```

wmenu-caos needs cairo/pango/wayland/xkbcommon + wlr-layer-shell protocol
(meson+ninja). `wclipmenu image` needs ImageMagick.<br>
**Secrets**: Last.fm/Genius/MusicBrainz/ListenBrainz keys live in
`~/.config/youtui/config.toml` — never commit (gitignored).

## Notes

- **Secrets**: never commit credentials. `config.toml.example` files ship
  placeholders; real files are gitignored (see `rescrobbled/README.md`).
- **youtui fork build** (`~/builds/youtui`): rustup installed with
  `--no-modify-path` — run `. "$HOME/.cargo/env"` before `cargo build`.
  `~/.local/bin/youtui` symlinks `target/release/youtui` (upstream 1.0.9
  kept as `~/.local/bin/youtui-1.0.9.bak`). Keys live in
  `~/.config/youtui/config.toml` (gitignored).
- PR workflow everywhere: feature branch → PR → merge; no direct pushes.
- Per-fork patches live in the dwl repo's `patches/` (dwl-patches convention).
