# unclutter

Hide the cursor after `cursor_timeout` seconds of inactivity (default 5s);
any pointer activity restores it. Replaces the `wlr_cursor_unset_image`
approach without an external unclutter daemon.

- Source: https://codeberg.org/dwl/dwl-patches/patches/unclutter (unclutter-0.8.patch)
- Applied: clean, 0.8-dev fork compatible
- Config: `cursor_timeout` (seconds) in config.h/config.def.h
- Behavior: `handlecursoractivity()` re-arms the timer and un-hides; the last
  cursor shape/surface is remembered so a hidden cursor can be restored.
- Deps: none
