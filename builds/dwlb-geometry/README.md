# builds/dwlb-geometry — retired

Kept for reference only. dwlb-geometry was the external layer-shell bar that
segfaulted on its first frame (`draw_frame` during `layer_surface_configure`,
seen in journalctl core dumps) and was **dropped from the session launch**
(commit "fix: drop dwlb overlay from session launch"). The native dwl bar +
IPC `bar_geometry` (see `builds/dwl/README.md`) replaced it.
