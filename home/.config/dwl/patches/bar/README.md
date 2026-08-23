# bar (sewn's dwm bar clone)

## Purpose
Native dwl status bar (tags, layout symbol, window title, status via stdin) with click regions.

## Source
- Author: sewn <sewn@disroot.org>
- Date: 2026-01-05
- Original: [dwl-patches/bar](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/bar)

## Changes
- `drwl.h` — minimal drawing abstraction (text, rect, image, font, colors)
- `dwl.c` — `drawbar()`, `drawstatus()`, `drawbars()`, click handling (`ClkTagBar`, `ClkLtSymbol`, `ClkStatus`, `ClkTitle`, `ClkClient`, `ClkRoot`)
- `config.def.h` — `showbar`, `topbar`, `fonts[]`, `colors[][3]` (SchemeNorm/Sel/Urg), `tags[]`, `Clk*` buttons
- `Makefile` — `pixman-1`, `fcft` deps

## Applied
Commit `c16de37` — "feat: native dwl bar + barcolors patch (verified working baseline)"

## Notes
- Status text read from stdin (pipe into dwl), parsed for `^fg()`/`^bg()` markup
- `statusin` buffer 256→512 to avoid overflow with colored status
- Bar height = font height + 2px
