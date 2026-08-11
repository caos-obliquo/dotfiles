# mako

Wayland notification daemon. Black/purple Dracula aesthetic, top-right.

## Files

- `config` — colors, position, style, urgency

## Notes

- **No keybinds configured** — notifications are handled from the keyboard
  via dwl: `Super+Shift+D` = `makoctl dismiss` (dismiss latest).
- `makoctl dismiss --all` clears everything; `makoctl list` inspects.
- High-urgency notifications get a red border (`#ff5555`).
- Default timeout 5s.
