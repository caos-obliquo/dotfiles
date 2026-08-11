# dwlb

Status bar (dwl fork of dwlb). The bar itself is configured/launched from
`start-dwl.sh`; this folder holds the **status feed scripts** it renders.

## Files

- `scripts/` — one script per module, emitted into the `dwlb-status.sh` feed:
  - `battery.sh` — charge %
  - `cpu.sh` / `memory.sh` / `temp.sh` — system stats
  - `network.sh` — connectivity
  - `volume.sh` — audio %

## How it fits

`~/.local/bin/dwlb-status.sh | dwlb -status-stdin all` — the feed pipes into
dwlb; geometry + colors are written to `/tmp/dwlb-geometry` for wmenu.
