# alwayscenter

New floating windows spawn **centered** on the monitor work area. Floating
windows that exit fullscreen also restore centered.

Source: [dwl-patches alwayscenter](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/alwayscenter)
Author: Guido Cella
Applied: in-tree (see `mapnotify()` + `setmon()` in dwl.c)

## Behavior

- **On map** (`mapnotify`): floating, non-fullscreen, managed clients are
  resized to the center of `c->mon->w` (work area). Unmanaged surfaces
  (menus, tooltips, dropdowns) are excluded — they keep their own position.
- **On monitor assignment** (`setmon`, upstream hunk): `c->prev` (the
  fullscreen-restore geometry) is centered, so leaving fullscreen returns the
  window to center rather than its pre-fullscreen spot.

## Deviation from upstream

Upstream's 2026-01-15 patch (`48110f0`) only adds the `setmon` hunk — it
centers the *restore position*, not the initial spawn. This fork additionally
centers on map so the patch actually does what its README promises ("center
floating windows"). The `setmon` hunk is kept verbatim.

## Config

None — always on for floating clients.

## Verify

- Build: clean (`-Wall -Wextra -Werror=*`).
- Manual: spawn `wmenu-center` (rule-floated) — must open centered; open a
  tooltip/menu from an app — must keep its original position.
