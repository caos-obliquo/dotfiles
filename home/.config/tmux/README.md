# tmux

Terminal multiplexer with vim-navigation grammar (consistent with dwl/zsh/nvim).

## Keybinds (tmux layer)

Prefix = **`Ctrl+Space`**.

| Key | Action |
|---|---|
| `C-Space` then `\|` / `-` | split horizontal / vertical (current path) |
| `C-Space` `h/j/k/l` | select pane left / down / up / right |
| `C-Space` `H/J/K/L` | resize pane 5 (repeatable) |
| `C-Space` `Esc` | copy-mode (vi: `v` select, `y` copy) |
| `C-Space` `r` | reload config |

## Files

- `tmux.conf` — prefix, colors, status-interval 1s, `mouse on`, history 50k

## Notes

- Default terminal `tmux-256color`; base index 1 (not 0).
