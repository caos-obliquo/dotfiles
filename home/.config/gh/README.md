# gh

GitHub CLI config.

## Files

- `config.yml` — protocol, editor, pager, aliases

## Notes

- `git_protocol: https`; pager unset (uses `$PAGER`).
- Alias: `gh co` = `gh pr checkout`.
- Used everywhere in this stack's workflow: every repo merges via PR, never
  direct pushes to main.
