# kanshi

Automatic output configuration for Wayland (autorandr-style). Applies the
matching profile whenever outputs connect/disconnect — no manual
`wlr-randr` when you dock.

## Files

- `config` — profiles (laptop-only by default, docked example commented out)

## Notes

- Requires `kanshi` (Arch: `sudo pacman -S kanshi`); launch it from
  `start-dwl.sh` (it runs in the background and reacts to hotplug).
- Works with dwl via the wlr-output-management-v1 protocol.
- Inspect outputs: `wlr-randr`.
- First profile in the file wins on match; `output ... disable` turns a
  screen off for that profile.
