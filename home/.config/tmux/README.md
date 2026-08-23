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

## Status bar

Design policy (v1.0.0): keep the **arch glyph** + **window-tab nerd icons**, and
restrict the **right module to the music scroller only** (system icons script is
not shown on the bar).

| Segment | Source | Content |
|---|---|---|
| `status-left` | `tmux.conf` | arch glyph `󰣇` + session name `#S` |
| window tabs | `tmux-nerd-font-window-name` plugin | `#I` index + `#W` name (name prefixed with the nerd window icon) |
| `status-right` | `~/.local/bin/tmux-music` | now-playing scroller: `<icon> artist - title` |

- Window-tab icons come from the `tmux-nerd-font-window-name` TPM plugin, configured in
  `tmux-nerd-font-window-name.yml` (icon map for `youtui`/`pi`/`opencode`, `fallback-icon: "?"`,
  `show-name: true`). The plugin rewrites `automatic-rename-format` into a
  `pane_current_command`→icon map (it substitutes `#{window_icon}` only if you placed that token in your format).
- `tmux-music` reads `/tmp/youtui-state` (written by `playerctl-daemon.sh`, player `youtui`) and
  marquee-scrolls when longer than 40% of client width (1 char/sec via `status-interval 1`).
- `~/.local/bin/tmux-icons` exists but is **currently unused** — it is text-only
  (`BAT %` / `VOL %`) and is not referenced by `status-right`.

### Music scroller icons

| State | Glyph | Color |
|---|---|---|
| Playing | `` (nf-fa-play_circle) | green `#50fa7b` |
| Paused | `` (nf-fa-pause_circle) | grey `#6272a4` |
| Stopped / unknown | `` (nf-fa-stop_circle) | grey `#6272a4` |

Text format: `<icon> artist - title`.

## Files

- `tmux.conf` — prefix, keybinds, status-bar layout, nerd-font window icons, plugins, history 50k
- `tmux-nerd-font-window-name.yml` — icon map + options for the window-name plugin
- `~/.local/bin/tmux-music` — now-playing status-right script (play/pause/stop icons, marquee)
- `~/.local/bin/tmux-icons` — text-only system icons (BAT%/VOL%), currently unused on the bar

## Notes

- Default terminal `tmux-256color`; base index 1 (not 0); plugins via tpm
- Versioned as `tmux-v1.0.0` — see the GitHub release notes for the full change set

## Known issues

- Window-tab nerd icons disappear from a window once it is renamed manually
  (`prefix` + `,`): tmux sets that window's `automatic-rename` to off, so the
  icon-prefixed name is no longer applied. Re-enable per window with
  `tmux set-window-option -t <win> automatic-rename on`, or open a new window
  (new windows inherit the global `automatic-rename on`). The plugin and
  `automatic-rename-format` are correct — only the per-window flag is the issue.
