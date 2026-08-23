# Recommended dwl patches

Curated from the official [dwl-patches](https://codeberg.org/dwl/dwl-patches)
repository, matching this setup: `foot` terminal, `wmenu`/`wclipmenu`, sewn-in
dwl bar (dwl-status.sh stdin pipe), `wlock`, Dracula theme, XWayland enabled, trackpad.

Apply with: `patch -p1 < patches/<name>.patch` (from the repo root), then
`make`.

## Fixes / quality of life (recommended)

| Patch | Why |
|---|---|
| `fix-rendermon-pending-resize-freeze` (in-tree) | Fixes the Super+F/Super+T output freeze. See `docs/BUG-FLOAT-TILE-FREEZE.md`. |
| `attachbottom` (in-tree) | New windows open at the bottom of the stack instead of stealing the master slot. Applied. |
| `spawnorfocus` (in-tree) | `Super+b` focuses waterfox if running, spawns it otherwise. Applied. |
| `warpcursor` (in-tree) | Cursor jumps into the newly focused window — pairs with `sloppyfocus`. Applied. |
| `alwayscenter` (in-tree) | Floating windows spawn centered on the monitor. Applied. |
| `movestack` (in-tree) | `Super+Shift+J/K` reorders the stack. Applied. |
| `xwayland-handle-minimize` | XWayland is enabled. Without it X11 windows that minimize can leave focus/rendering in a weird state. |
| `foreign-toplevel-management` | Lets tools like `wlopm`, some screenshot/manage apps and newer status bars see and control windows. Cheap, stable. |
| `primaryselection` | Adds middle-click primary selection paste for Wayland clients — near-mandatory muscle memory for X users. |

## Layout / window management (choose what fits your flow)

| Patch | Why |
|---|---|
| `pertag` | Per-tag layout memory: keep tag 1 tiled, tag 2 floating, etc. Directly complements the Super+F/Super+T workflow and removes the "global layout" surprise. |
| `gaps` / `vanitygaps` | Window gaps. Pure aesthetic; only if you like the look. |
| `centeredmaster` / `dwindle` / `column` | Alternative tiling layouts beyond `[]=`/`[M]`. |

## Scratchpads / workflow

| Patch | Why |
|---|---|
| `namedscratchpads` | Drop-down terminals/utilities (e.g. a scratch `foot`), like sway scratchpads. |
| `swallow` | Terminals swallow child GUI apps; close the app, the terminal returns. Very dwm-feel. |

## Bar-related (only if you move off `-no-ipc`)

| Patch | Why |
|---|---|
| `ipc` | Enables the `dwl-ipc-unstable-v2` protocol that `dwlb` needs for full bar mode (tags, layout, titles, per-monitor). This fork compiles the protocol in-tree (used by `wmenu-caos` for `bar_geometry`). Combine with the `ipc` patch and drop `-no-ipc` from `start-dwl.sh`. |
| `hide_vacant_tags` | With the `ipc` patch: hides empty tags in the bar. |

## Not recommended here

- `autostart` — you already use `dwl -s`; no need.
- `systemd` — only if you want dwl itself as a systemd user service.
- `borders`/`smartborders`/`bar-*` — `borderpx = 0` and the bar is sewn-in; not applicable.
- `simpleborders` — reported as aggravating the very freeze fixed above (issue #1203 thread); avoid.
- `tearing`/`fullscreenadaptivesync`/`gamepad-bindings` — hardware/gaming specific; skip unless you game on this machine.
- `touch-input`/`tablet-input`/`virtual-pointer` — no touch/tablet hardware present.

## Notes

- `Super+F` in the default config switches the **whole monitor** to the
  floating layout. To float **just the focused window**, use
  `Super+Shift+Space` (bound to `togglefloating`).
- Rebase/refresh patches against `upstream` before applying; dwl moves fast.
- Keep patches in `patches/` and regenerate after upstream merges the 
  equivalent fix (dwl !1208) so `fix-rendermon-pending-resize-freeze` can be dropped.

## Personal baseline (own stack)

This section is the **authoritative personal validation baseline** for the
13-patch dwl restack. Every claim below is backed by a `grep` of the live
config or a `readlink` of the path. The executor must validate the 13 patches
against THIS config, not against any doc that drifts from it.

### 1. Canonical build repo (symlink + twin audit)

`readlink -f` evidence (run 2026-08-19):

| Path | Type | `readlink -f` resolves to |
|---|---|---|
| `/home/caos/dwl` | symlink (26B) | `/home/caos/builds/dwl-user` |
| `/home/caos/builds/dwl-user` | real dir | `/home/caos/builds/dwl-user` |
| `/home/caos/builds/dwl` | real dir | `/home/caos/builds/dwl` |
| `/home/caos/.config/dwl` | real dir | `/home/caos/.config/dwl` |

- `/home/caos/dwl` → `/home/caos/builds/dwl-user` (literal symlink). **`~/dwl` and `~/builds/dwl-user` are the same canonical repo.**
- `/home/caos/builds/dwl` is a **separate, divergent twin** (7 patch folders vs 13; its `config.h` lacks bstack/swallow/gaps).
- `/home/caos/.config/dwl` is a **third real dir** (its `config.h` also lacks bstack/swallow/gaps).
- IMPORTANT: `/home/caos/dotfiles/builds/dwl/config.h` is a symlink → `../../../builds/dwl/config.h` (i.e. the **7-patch twin**, NOT `dwl-user`). The dotfiles tree therefore points at the OLD twin, not the canonical repo.

**Conclusion: THE canonical repo is `/home/caos/builds/dwl-user` (= `~/dwl`). Its compiled `config.h` is `/home/caos/builds/dwl-user/config.h`. Treat `builds/dwl` and `.config/dwl` as stale twins; ignore their `config.h` for validation.**

### 2. Canonical config & keybinds

Source of truth: `/home/caos/builds/dwl-user/config.h` (the file actually compiled by
the canonical repo). `MODKEY` is Super — `config.h:244`:
`#define MODKEY WLR_MODIFIER_LOGO /* Super/Windows key */`.

Each bind grepped directly from `builds/dwl-user/config.h`:

| Keybind | Action | Evidence (`builds/dwl-user/config.h`) |
|---|---|---|
| `Super+Shift+J` / `Super+Shift+K` | `movestack` down / up stack | `config.h:318-319` |
| `Super+u` | bottomstack layout (`TTT`/bstack) | `config.h:341` (`setlayout &layouts[3]`) |
| `Super+Shift+U` | bottomstack-horiz (`===`) | `config.h:342` (`setlayout &layouts[4]`) |
| `Super+o` / `Super+Shift+O` | `setopacity` +0.1 / -0.1 | `config.h:333-334` |
| `Super+b` | `spawnorfocus` waterfox | `config.h:306` → `browsercmd[]={"waterfox","Waterfox",NULL}` `config.h:270` |
| `Mod+a` | (freed — no binding) | swallow runs via `enableautoswallow=1` (`config.h:22`); toggle handlers + binds removed |
| `Mod+Shift+G` | `togglegaps` | `config.h:349` |
| `Super+0` | view all tags (`~0`) | `config.h:355` |
| `Super+s` / `Super+Shift+S` | area / full screenshot | `config.h:397-398` |
| `Super+Return` | `zoom` (promote to master) | `config.h:320` |
| `Super+Tab` | view last tag | `config.h:337` |
| `Ctrl+Alt+Backspace` | quit dwl (`Terminate_Server`) | `config.h:417-420` |

Window rules (grepped from `config.h:46-195`):

| App | Rule | Evidence |
|---|---|---|
| `wmenu-center` | floating, opacity `0.85f` | `config.h:49` |
| `waterfox` | no tag (`tags = 0`), opacity 1.0 | `config.h:192` |

NOTE: bstack (`Super+u/U`), swallow (now unbound — autoswallow only), and gaps (`Mod+Shift+G`)
exist ONLY in `builds/dwl-user/config.h`. They are ABSENT from the `config.h`
in `builds/dwl` (the dotfiles symlink target) and from `.config/dwl/config.h` —
which is why `docs/KEYBINDS.md` (both the `dwl-user` and `dotfiles` copies)
omits them. The canonical live config has them; the docs are stale.

### 3. Status-feed protocol

Script: `/home/caos/.local/bin/dwl-status.sh`.

- Dracula `^fg()` markup fed to dwl's stdin bar (sewn-in bar, NOT dwlb).
- Segment order (single `echo`, `dwl-status.sh:97`):
  `vol | cpu | mem | zsh | temp | net | bat | time`
- Colors (Dracula palette, all from `dwl-status.sh`):
  - vol — pink `ff79c6` (`dwl-status.sh:15`)
  - cpu — red `ff5555` (`dwl-status.sh:97`)
  - mem — yellow `f1fa8c` (`dwl-status.sh:97`)
  - zsh — cyan `8be9fd` (`dwl-status.sh:94`)
  - temp — orange `ffb86c` (`dwl-status.sh:30`)
  - net — green `50fa7b` (`dwl-status.sh:49`)
  - bat — cyan `8be9fd` (discharging) / green `50fa7b` (charging) (`dwl-status.sh:82,84`)
  - time — purple `bd93f9` (`dwl-status.sh:97`)
- Loop cadence: `sleep 2` (`dwl-status.sh:99`).

### 4. Session launcher env

Script: `/home/caos/.local/bin/start-dwl.sh`.

- Renderer: `export WLR_RENDERER=gles2` (`start-dwl.sh:21`).
- XDG env: `XDG_SESSION_TYPE=wayland`, `XDG_CURRENT_DESKTOP=wlroots`,
  `XDG_SESSION_DESKTOP=wlroots`, `__EGL_VENDOR_LIBRARY_FILENAMES`,
  `DBUS_SESSION_BUS_ADDRESS` (`start-dwl.sh:5-9`).
- Launch line (`start-dwl.sh:22`):
  `exec ~/.local/bin/dwl-status.sh | dwl -s 'sh -c "wawa fill ~/walls/wall5-16x10.jpg"'`
  → `dwl-status.sh` is piped into **dwl stdin**; `-s` only runs the wallpaper
  fill. No `dwlb` process is spawned here.

### 5. Doc mismatches (reconcile)

**(a) README "freeze fix + 7 patches" vs 13 patch folders — OPEN.**
`/home/caos/dotfiles/README.md:16` claims "patched fork: freeze fix + 7 patches",
but the canonical repo `/home/caos/builds/dwl-user/patches/` has **13** folders:
`alwayscenter, attachbottom, bar, bar-systray, bottomstack, client-opacity,
fix-rendermon-pending-resize-freeze, gaps, movestack, spawnorfocus, swallow,
unclutter, warpcursor`.
The "7" matches the STALE twin `/home/caos/builds/dwl/patches/` (7 folders), not
the canonical repo. **Recommended fix:** update `dotfiles/README.md:16` to
"freeze fix + 13 patches" and repoint `dotfiles/builds/dwl/config.h` symlink
from `builds/dwl/config.h` to `builds/dwl-user/config.h`.

**(b) README/PATCHES.md list dwlb as the status bar vs live sewn bar — OPEN.**
`dotfiles/README.md:17` and `docs/PATCHES.md:3-5,43,55` describe `dwlb` as the
bar, but `start-dwl.sh:22` pipes `dwl-status.sh` into dwl stdin — the live bar
is the sewn-in `^fg()` status feed from §3, and NO `dwlb` is launched anywhere
(only referenced in docs). **Recommended fix:** rewrite the bar description to
"sewn-in dwl status via `dwl-status.sh` stdin pipe (Dracula `^fg()` markup)";
move dwlb to "Not recommended here" or delete.

**Addendum (doc gap, not in the original 2):** both `docs/KEYBINDS.md` copies
(`builds/dwl-user/docs/KEYBINDS.md`, `dotfiles/docs/KEYBINDS.md`) omit the
bstack (`Super+u/U`), swallow (unbound — autoswallow only), and gaps (`Mod+Shift+G`)
binds present in the canonical `config.h`. The executor should trust
`builds/dwl-user/config.h`, not the KEYBINDS docs, for these three.

## Patch provenance (git history + upstream + contributor configs)

Upstream dwl baseline: development migrated from `github.com/djpohly/dwl`
(archived 2023-11, v0.5) to **`codeberg.org/dwl/dwl`** (canonical, C, 684★).
Patches live in the user-maintained **`codeberg.org/dwl/dwl-patches`** repo
(`patches/<name>/<name>.patch` + `README.md`). This fork (`caos-obliquo/dwl`)
tracks 0.8-dev against wlroots 0.19.

### Upstream + contributor configs

**bar** — sewn `<sewn@disroot.org>`
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/bar>
- sewn's dwl fork (bar branch): <https://codeberg.org/sewn/dwl>
- Contributor dotfiles: **not found** (sewn maintains a personal dwl fork but no public dotfiles repo)
- Key config: `showbar`, `topbar`, `fonts[]`, `colors[][3]` (SchemeNorm/Sel/Urg), `tags[]`, `ClkTagBar`/`ClkLtSymbol`/`ClkStatus`/`ClkTitle`/`ClkClient`/`ClkRoot` buttons, `statusin` buffer 512, `^fg()`/`^bg()` markup, bar height = font height + 2px

**bar-systray** — vetu104 (janetski) `<vetu104@proton.me>`
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/bar-systray>
- vetu104's dwl fork (systray/): <https://codeberg.org/janetski/dwl>; patches fork: <https://codeberg.org/janetski/dwl-patches>; also `github.com/vetu104/dwlb` (KStatusNotifierItem dwlb fork)
- Contributor dotfiles: **not found as standalone dotfiles repo**; live config in `janetski/dwl` as `config.h.desktop` / `config.h.laptop`
- Key config: `systrayspacing`, `showsystray`, `dmenucmd`, `ClkTray` buttons; left-click activate, right-click menu via `dmenucmd`

**swallow** — Nikita Ivanov `<nikita.vyach.ivanov@gmail.com>`
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/swallow>
- Nikita's forks: <https://codeberg.org/nikitaivanov/dwl> (swallow branch), <https://codeberg.org/nikitaivanov/dwl-patches>
- GitHub: <https://github.com/Evgaizen> — `dotfiles` repo exists but is web-dev focused (Lua), **no dwl config**
- Key config: `isterm`, `noswallow` Rule fields, `enableautoswallow` (on by default) — **toggle binds removed, `Super+a` freed** (no key); uses `/proc/<pid>/stat` parent walk

**gaps** — peesock `<kcormn@gmail.com>`, co-authors sewn `<sewn@disroot.org>` + serenevoid `<ajuph9224@gmail.com>`
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/gaps>
- peesock's forks: <https://codeberg.org/bigman/dwl>, <https://codeberg.org/bigman/dwl-patches>
- Contributor dotfiles: **not found** for peesock/bigman or serenevoid (`github.com/serenevoid` exists, no dwl config)
- Key config: `gappx = 6`, `gaps = 1`, `smartgaps = 0`, `Mod+Shift+G` per-monitor toggle (upstream was `Mod+g`; fork has `Mod+g` = passmenu)

**unclutter** — nate zhou `<gnuunixchad@outlook.com>` (unixchad)
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/unclutter>
- Contributor dotfiles: **<https://codeberg.org/unixchad/dotfiles>** (comprehensive, dwl in window manager list; mirror: `github.com/gnuunixchad/dotfiles`)
- Key config: `cursor_timeout = 5` (seconds); `handlecursoractivity()` re-arms timer and un-hides

**bottomstack** — nate zhou `<gnuunixchad@outlook.com>` (unixchad); 0.8 update adopted by unixchad
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/bottomstack>
- Contributor dotfiles: same as unclutter — <https://codeberg.org/unixchad/dotfiles>
- Key config: `bstack` ("TTT"), `bstackhoriz` ("==="), `Mod+u` / `Mod+Shift+U` (fork-adapted; upstream `Mod+o` collides with `setopacity`)

**client-opacity** — Leonardo Hernández Hernández `<leohdz172@proton.me>` (author found; patch header `Modified-by: Yuki <yukii.senp@gmail.com>`)
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/client-opacity>
- Contributor dotfiles: **not found** for leohdz172
- Key config: `default_opacity = 0.75`, `Mod+o` increase focus opacity, `Mod+Shift+O` decrease; per-rule opacity override via `scenebuffersetopacity()`

**fix-rendermon-pending-resize-freeze** — this fork (`caos-obliquo`), based on upstream dwl PR !1208
- Upstream issue: <https://codeberg.org/dwl/dwl/issues/1203> ("Display stops rendering frames when switching to tags under certain conditions")
- Upstream PR: <https://codeberg.org/dwl/dwl/pulls/1208>
- Local commit: `d9a2bc5` "fix: stop output freeze after float/tile layout toggle"
- Contributor dotfiles: n/a (in-tree fork fix)
- Key config: none (code fix — drops manual `c->resize` skip in `rendermon()`, gates on `wlr_scene_output_needs_frame()` instead; keeps client-opacity per-frame update)

**attachbottom** — Ben Collerson, Aurel Weinhold
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/attachbottom>
- Ben Collerson forks: <https://codeberg.org/bencc/dwl>, <https://codeberg.org/bencc/dwl-patches>; GitHub: <https://github.com/bencollerson> (somebar author)
- Contributor dotfiles: **not found**
- Key config: none (always on — `wl_list_insert(clients.prev, &c->link)` in `mapnotify()`)

**spawnorfocus** — GravityShark, Ben Collerson, clicseo, wochap, Louis-Michel Raynauld
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/spawnorfocus>
- GravityShark: <https://github.com/GravityShark/dwl-grav/tree/spawnorfocus>; clicseo: <https://github.com/clicseo> (originated "Run or Raise" idea, `djpohly/dwl#519`); wochap: <https://codeberg.org/wochap/dwl>; Louis-Michel Raynauld: <https://github.com/loumray/dwl>
- Contributor dotfiles: **not found** for any of the five authors
- Key config: `browsercmd = { "waterfox", "Waterfox", NULL }`, `Mod+b`; argv layout `{cmd, needle, NULL}` (fork deviation from upstream `{cmd, NULL, needle}` to avoid OOB read on 2-element arrays)

**movestack** — wochap, sam-barr, Dmitry Zakharchenko (dm1tz), Abanoub8, Nikita Ivanov
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/movestack>
- wochap fork: <https://codeberg.org/wochap/dwl>
- Contributor dotfiles: **not found**
- Key config: `Mod+Shift+J` move down, `Mod+Shift+K` move up; cyclic wrap via `wl_container_of(&clients, c, link)` sentinel

**warpcursor** — Ben Collerson, Faerryn
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/warpcursor>
- Faerryn: **not found** (only referenced in legacy `djpohly/dwl` wiki `cursorwarp` page)
- Contributor dotfiles: **not found**
- Key config: none (always on — warps to focused client center on `focusclient(..., lift)` and `arrange()`, skipped while `cursor_mode != CurNormal`)

**alwayscenter** — Guido Cella `<guido@guidocella.xyz>`
- Canonical: <https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/alwayscenter>
- Guido's dwl fork: <https://codeberg.org/guidocella/dwl>; GitHub: <https://github.com/guidocella> (mpv scripts, no dwl dotfiles)
- Contributor dotfiles: **not found** (Guido is dwl patch maintainer — layer-shell, output management — but no public dwl dotfiles)
- Key config: none (always on for floating clients); fork extends upstream 2026-01-15 patch (`48110f0`) by also centering on `mapnotify()` (upstream only centers the fullscreen-restore position in `setmon()`)
