# dwl patches (this fork)

Each patch lives in its own folder with the `.patch` and a `README.md`,
mirroring the [dwl-patches](https://codeberg.org/dwl/dwl-patches) convention.

Apply with `patch -p1 < patches/<name>/<name>.patch` from the repo root, then
`make`.

## Applied in-tree

| Patch | Purpose | Source |
|---|---|---|
| `client-opacity` | Per-client opacity + `Super+O` / `Super+Shift+O` binds | dwl-patches |
| `fix-rendermon-pending-resize-freeze` | Fixes output freeze after layout/float toggles (dwl #1203, PR !1208) | this fork |
| `attachbottom` | New windows tile at stack bottom | dwl-patches |
| `spawnorfocus` | `Super+b` raises or spawns firefox | dwl-patches (adapted) |
| `movestack` | `Super+Shift+J/K` reorder stack | dwl-patches 0.8 |
| `warpcursor` | Cursor follows keyboard focus | dwl-patches (style-normalized) |
| `alwayscenter` | Floating windows spawn centered | dwl-patches (extended) |

## Notes

- `git diff` on a feature branch regenerates a patch. Keep each patch in sync
  with the applied tree so `git apply --check` stays green.
- `config.h` is the live config; `config.def.h` is the template — both stay in
  sync for every patch that adds binds.
- Drop `fix-rendermon-pending-resize-freeze` once upstream merges !1208.
- Full rationale: `docs/BUG-FLOAT-TILE-FREEZE.md`, `docs/PATCHES.md`,
  `docs/KEYBINDS.md`.
