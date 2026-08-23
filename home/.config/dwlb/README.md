# dwlb — RETIRED

The sewn-in dwl bar (dwl-status.sh stdin pipe) replaced dwlb. This folder is
kept for reference only.

## What it was

Status bar (dwl fork of dwlb). The bar itself was configured/launched from
`start-dwl.sh`; this folder held the **status feed scripts** it rendered.

## Files

- `scripts/` — one script per module, emitted into the `dwlb-status.sh` feed:
  - `battery.sh` — charge %
  - `cpu.sh` / `memory.sh` / `temp.sh` — system stats
  - `network.sh` — connectivity
  - `volume.sh` — audio %

## How it fit (historical)

`~/.local/bin/dwlb-status.sh | dwlb -status-stdin all` — the feed piped into
dwlb; geometry + colors were written to `/tmp/dwlb-geometry` for wmenu.

## Replacement

`~/.local/bin/dwl-status.sh | dwl -s ...` — the same feed now pipes into dwl's
sewn-in bar (Dracula `^fg()` markup); geometry is pushed to wmenu via
`zdwl_ipc_output_v2.bar_geometry` instead of `/tmp/dwlb-geometry`.