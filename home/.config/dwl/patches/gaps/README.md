# gaps

Gaps between tiled windows and outer screen edges.

- Source: https://codeberg.org/dwl/dwl-patches/patches/gaps
- Applied: hand-ported to 0.8-dev (tile() rewritten with gap math; Rule struct
  and Client struct are fork-extended so the stock hunk did not apply cleanly)
- Config: `gappx` (gap pixel, 6), `gaps` (on by default), `smartgaps` (0 = keep
  outer gap even with a single window). `Mod+Shift+G` toggles per-monitor gaps
  (upstream used `Mod+g`; our fork has `Mod+g` = passmenu)
- Deps: none
