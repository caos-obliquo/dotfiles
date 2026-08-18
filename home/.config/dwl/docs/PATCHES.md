# dwl patches — applied state

Applied and committed in the caos-obliquo/dwl fork. Patches live in the fork's
`patches/` directory (dwl-patches convention).

## Applied patches

| Patch | Notes |
|---|---|
| `bar` (sewn) | Native dwl bar (30px, top). Draws tags, layout symbol, title, status, tray. |
| `barcolors` | Bar colors follow `SchemeNorm`/`SchemeSel` from config. |
| `appicons` | Appicon rules in `config.h` — Nerd Font glyphs replace tag labels (waterfox 󰈹, chromium 󰊯, steam , youtui 󰑈). |
| `systray` | SNI tray at the right end of the bar; dwl owns `org.kde.StatusNotifierWatcher`. |
| `client-opacity` | `Super+o` opacity +10%, `Super+Shift+O` opacity −10% (bound in `config.h`; `default_opacity 1.0f`). |
| `dwl-ipc-unstable-v2` | `zdwl_ipc_manager_v2`/`zdwl_ipc_output_v2`; fork adds a `bar_geometry` event (since=2) publishing the middle title area on every bar draw. wmenu-caos positions its pill from it. |
| `fix-rendermon-pending-resize-freeze` (in-tree) | Fixes the Super+F/Super+T output freeze. See `docs/BUG-FLOAT-TILE-FREEZE.md`. |
| `attachbottom` (in-tree) | New windows open at the bottom of the stack. |
| `spawnorfocus` (in-tree) | `Super+b` focuses firefox if running, spawns it otherwise. |
| `warpcursor` (in-tree) | Cursor jumps into the newly focused window (pairs with `sloppyfocus`). |
| `alwayscenter` (in-tree) | Floating windows spawn centered on the monitor. |
| `movestack` (in-tree) | `Super+Shift+J/K` reorders the stack. |
| `swallow` | Terminal swallow — child clients take over the terminal's slot (terminal hidden while child open). `Mod+a` toggle swallow focused into next client, `Mod+Shift+a` toggle auto-swallow (on by default). foot is the `isterm=1, noswallow=1` host rule. |
| `gaps` | Tiled window gutters + `smartgaps` (no outer gap with one window). `Mod+Shift+g` toggles. `gappx = 6`. |
| `unclutter` | Auto-hide the cursor after `cursor_timeout` (5s) idle; returns on activity. |
| `bottomstack` | `bstack`/`bstackhoriz` layouts (`TTT`, `===`). `Mod+u` / `Mod+Shift+U` select them. |

## Notable deviations from upstream

- **All tag cells render as purple pills** — `SchemeSel` unconditionally,
  regardless of tagset/occupancy (upstream draws unselected tags with
  `SchemeNorm`).
- **`bar_geometry` = middle title area** — upstream/original IPC semantics
  send the status area; the fork repurposed the payload to the middle region
  (tags+layout symbol → status text), output-relative logical pixels, sent
  for every monitor on every bar draw.
- **No `/tmp/dwl-bar-geometry` file writer** — removed; IPC replaces it.
- The fork historically tried an external `dwlb-geometry` overlay bar; it is
  retired (segfaulted on first frame) — the native bar is the bar.

## Not applied (deliberately)

- `hide_vacant_tags` — all tags stay visible (purple pill look).
- `borders`/`smartborders`/`bar-*` — `borderpx = 0`, native bar is used.
- `autostart` — session uses `dwl -s` (start-dwl.sh).
- `systemd`, `tearing`, `touch-input` — not applicable to this hardware.

## Notes

- Rebase/refresh patches against `upstream` before applying; dwl moves fast.
- Regenerate after upstream merges the equivalent fix (dwl !1208) so
  `fix-rendermon-pending-resize-freeze` can be dropped.
