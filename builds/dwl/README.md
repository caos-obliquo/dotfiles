# builds/dwl — dwl build config

This `config.h` is copied into a fresh clone of
[caos-obliquo/dwl](https://github.com/caos-obliquo/dwl) before `make`
(`autorice-deploy.sh` phase 10). The fork itself already ships the applied
patches (this folder only overrides the build-time config).

## What this config does

- **Dracula theme** — `SchemeNorm` bg `0x222222ee` / fg `0xeeeeeeff`;
  `SchemeSel` bg `0xbd93f9dd` / fg `0x1e1e2eff` (purple pill, dark text).
- **Native bar** (sewn bar patch), 30px, top.
- **All tag cells render as purple pills** — `SchemeSel` unconditionally,
  regardless of tagset/occupancy (deliberate deviation from upstream dwm/dwl
  look; see the commit "render all tag cells as purple pills").
- **Appicon rules** (`rules[]` with `appicon` field) — Nerd Font glyphs for
  waterfox `󰈹`, steam ``, pipemixer `󱄠`, plus ~140 app-specific rules and a
  generic fallback icon; `wmenu-center` floats at opacity 0.85.
- **Opacity** — `default_opacity 1.0f`; bindings live in the fork's
  `config.def.h`/`config.h` keybinds:
  - `Super+o` → `setopacity +0.1`
  - `Super+Shift+O` → `setopacity -0.1`
- **Systray** patch applied but **disabled** (`showsystray 0`) — the SNI tray
  is not rendered and dwl does not own `org.kde.StatusNotifierWatcher`.

## IPC bar geometry

dwl publishes the **middle title area** (tags+layout symbol → status text) on
every bar draw via `zdwl_ipc_output_v2.bar_geometry` (middle_x, middle_width,
bar_height, bg/fg colors, logical px, output-relative). wmenu-caos consumes
this to position its pill over exactly that region. The old
`/tmp/dwl-bar-geometry` file writer was removed — IPC replaced it.

Full keybinds: `../home/.config/dwl/docs/KEYBINDS.md`.
