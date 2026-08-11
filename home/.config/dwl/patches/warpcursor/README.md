# warpcursor

Warp the cursor to the center of the newly focused client — only when the
cursor is currently outside that client, and never while dragging/resizing
(`cursor_mode != CurNormal`).

Source: [dwl-patches warpcursor](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/warpcursor)
Author: Ben Collerson, Faerryn
Applied: in-tree (see `warpcursor()` in dwl.c)

## Behavior

- `focusclient()` with `lift` → cursor warps into the focused client's center.
- `arrange()` → warps to the focused client (or `selmon` center if none), but
  only when the cursor is outside the target rect, so pointer-driven layouts
  are not yanked around.
- Complements `sloppyfocus = 1`: keyboard focus and pointer position always
  agree.

## Deviations from upstream patch

- Brace style normalized to dwl's Allman convention (upstream used K&R).
- No functional changes.

## Config

None — no keybinds, always on.

## Verify

- Build: clean (`-Wall -Wextra -Werror=*`).
- Manual: `Super+j`/`Super+k` between windows — cursor follows into the
  focused window; mouse-only focus changes don't yank the cursor.
