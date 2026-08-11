# rescrobbled

last.fm scrobbler (the `rescrobbled` daemon + `listenbrainz`-style pipeline).

## Files

- `config.toml` — **gitignored, not in the repo**; contains your personal
  last.fm credentials
- `config.toml.example` — template with placeholders

## Setup

```bash
cp config.toml.example config.toml
# fill lastfm-key, lastfm-secret, session-key
```

## 🔐 Security note

Real API credentials were previously committed to this public repo — the
session key has been exposed in git history. **Rotate it** (re-authenticate
with last.fm) and never commit `config.toml` again (`.gitignore` enforces it).

## Notes

- Scrobbles from `youtui` (whitelisted player).
