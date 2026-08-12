# tmux

Terminal multiplexer with vim-navigation grammar (consistent with dwl/zsh/nvim).

## Keybinds (tmux layer)

Prefix = **`Ctrl+Space`**.

### Panes

| Key | Action |
|---|---|
| `C-Space` `\|` / `-` | split horizontal / vertical (current path) |
| `C-Space` `h/j/k/l` | select pane left / down / up / right |
| `C-Space` `H/J/K/L` | resize pane 5 (repeatable) |
| `C-Space` `o` | swap pane with next |

### Copy mode (vi)

| Key | Action |
|---|---|
| `C-Space` `Esc` | enter copy-mode |
| `v` | begin selection |
| `y` | copy (wl-copy), exit |

### Session / window

| Key | Action |
|---|---|
| `C-Space` `s` | choose session |
| `C-Space` `w` | choose window |
| `c` | new window (default) |
| `,` | rename window (default) |

### Misc

| Key | Action |
|---|---|
| `C-Space` `r` | reload config |
| `C-l` | unbound (pi agent model switcher) |

## Files

- `tmux.conf` — prefix, keybinds, nerd-font icons statusbar, plugins, history 50k

## Notes

- Nerd-font icons: arch `󰣇`, clock `󰥔`, window `󰕰`, music `󰎈`/`󰏤`/`󰓇`
- Music status reads `/tmp/youtui-state` (written by `playerctl-daemon.sh`, player `youtui`)
- Default terminal `tmux-256color`; base index 1 (not 0); plugins via tpm