# ccze

Log colorizer with a hand-tuned **Dracula semantic scheme**: tiered by meaning,
not rainbow noise.

## Files

- `cczerc` — the scheme

## Tiers

| Tier | What | Colors |
|---|---|---|
| 1 | errors, warnings | bold red / orange |
| 2 | structure — timestamps, host, pid | dim |
| 3 | data — process names, paths, addresses | blue / purple / green |

## Integration

Wired into zsh: `journalctl`, `dmesg`, `docker`, `kubectl`, `ping`,
`ps`, `df`, … all pipe through `ccze -F ~/.config/ccze/cczerc`.
