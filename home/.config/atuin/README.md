# atuin

Shell history database with fuzzy/global search, sync-ready. Replaces up-arrow
and `Ctrl+R` (both disabled in `.zshrc`).

## Files

- `config.toml` — atuin settings

## Notes

- Search bound in zsh: `Ctrl+Up` (insert mode) and `k` (vi normal mode).
- Session/auth token lives outside the repo (`~/.local/share/atuin/`).
- `secrets_filter` keeps password-ish commands out of history.
