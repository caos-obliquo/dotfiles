# Recommended dwl patches

Curated from the official [dwl-patches](https://codeberg.org/dwl/dwl-patches)
repository, matching this setup: `foot` terminal, `wmenu`/`wclipmenu`, `dwlb`
bar (no-ipc), `wlock`, Dracula theme, XWayland enabled, trackpad.

Apply with: `patch -p1 < patches/<name>.patch` (from the repo root), then
`make`.

## Fixes / quality of life (recommended)

| Patch | Why |
|---|---|
| `fix-rendermon-pending-resize-freeze` (in-tree) | Fixes the Super+F/Super+T output freeze. See `docs/BUG-FLOAT-TILE-FREEZE.md`. |
| `attachbottom` (in-tree) | New windows open at the bottom of the stack instead of stealing the master slot. Applied. |
| `spawnorfocus` (in-tree) | `Super+b` focuses firefox if running, spawns it otherwise. Applied. |
| `warpcursor` (in-tree) | Cursor jumps into the newly focused window — pairs with `sloppyfocus`. Applied. |
| `alwayscenter` (in-tree) | Floating windows spawn centered on the monitor. Applied. |
| `movestack` (in-tree) | `Super+Shift+J/K` reorders the stack. Applied. |
| `xwayland-handle-minimize` | XWayland is enabled. Without it X11 windows that minimize can leave focus/rendering in a weird state. |
| `foreign-toplevel-management` | Lets tools like `wlopm`, some screenshot/manage apps and newer status bars see and control windows. Cheap, stable. |
| `primaryselection` | Adds middle-click primary selection paste for Wayland clients — near-mandatory muscle memory for X users. |

> The five movement patches above (`attachbottom`, `spawnorfocus`, `movestack`,
> `warpcursor`, `alwayscenter`) are applied and committed — see
> `docs/KEYBINDS.md` for their binds and `patches/<name>/README.md` for each
> patch's details and deviations.

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
| `ipc` | Enables the `dwl-ipc-unstable-v2` protocol that `dwlb` needs for full bar mode (tags, layout, titles, per-monitor). Your `dwlb-geometry` build already ships the protocol stubs. Combine with the `ipc` patch and drop `-no-ipc` from `start-dwl.sh`. |
| `hide_vacant_tags` | With the `ipc` patch: hides empty tags in the bar. |

## Not recommended here

- `autostart` — you already use `dwl -s`; no need.
- `systemd` — only if you want dwl itself as a systemd user service.
- `borders`/`smartborders`/`bar-*` — `borderpx = 0` and external `dwlb`; not applicable.
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
