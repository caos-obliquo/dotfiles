# zsh

Dracula-shell: vi-mode, colorized logs, atuin history.

## Files

- `.zshrc` — main config (no personal paths — `$HOME`-relative, API keys empty)
- `.zprofile` / `.zshenv` — startup/env scaffolding

## What's in here

- **zsh-vi-mode** — insert/normal/visual cursors, `Esc` → normal mode
- **Dracula prompt** — `user@host path$ ❯` (magenta/blue)
- **ccze log wrappers** — `journalctl`, `dmesg`, `docker`, `kubectl`, `ping`, … piped through the Dracula ccze scheme
- **Atuin history** — `Ctrl+Up` or vi-normal `k` opens search; up-arrow/ctrl-r disabled (atuin owns them)
- **zsh-autosuggestions + zsh-syntax-highlighting + zsh-z** (Dracula-styled)
- **`lfcd`** — `Ctrl+o` drops into `lf`, `Ctrl+e` edits the command line in vim

## Keybinds (shell layer)

| Key | Action |
|---|---|
| `Esc` / vi-normal `hjkl` | vim mode |
| `Ctrl+Up` or vi-`k` | atuin history search |
| `Ctrl+o` | `lf` file manager → cd to last dir |
| `Ctrl+e` | edit command line in nvim |
| menuselect `h/j/k/l` | vim-style completion menu nav |

## Secrets hygiene

API-key exports are placeholders (`""`). Fill them locally — never commit values.
