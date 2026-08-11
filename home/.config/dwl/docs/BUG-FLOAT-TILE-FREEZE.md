# Bug: Tile layout breaks after Super+F (float) — output freezes until reboot

Status: **fixed** on branch `fix/float-tile-render-freeze`
Patch: `patches/fix-rendermon-pending-resize-freeze.patch`
Upstream: dwl issue [#1203](https://codeberg.org/dwl/dwl/issues/1203), fix PR [dwl !1208](https://codeberg.org/dwl/dwl/pulls/1208)

## Symptoms

1. Press `Super+F` — monitor switches to the floating layout (`><>`).
2. Press `Super+T` — monitor switches back to tile (`[]=`), but windows stay
   stacked on top of each other / at their old floating positions.
3. Nothing you do (layout toggles, tag switches, spawning windows) redraws the
   screen correctly. Only a reboot restores normal behavior.

## Root cause

dwl's `rendermon()` deliberately skipped presenting a frame while *any* visible
tiled client had an outstanding resize configure:

```c
/* Render if no XDG clients have an outstanding resize and are visible on
 * this monitor. */
wl_list_for_each(c, &clients, link) {
    if (c->resize && !c->isfloating
            && client_is_rendered_on_mon(c, m) && !client_is_stopped(c))
        goto skip;
}
```

`resize()` records the configure serial in `c->resize` via
`client_set_size()`. It is cleared in `commitnotify()` only when the client
commits a buffer acknowledging that serial:

```c
if (c->resize && c->resize <= c->surface.xdg->current.configure_serial)
    c->resize = 0;
```

The **atomic layout** feature (dwl #19/#198) relies on this to avoid drawing
half-resized frames. The failure mode: if one background client never commits
after its configure — apps that defer commits while occluded, suspended, or
briefly wedged — `c->resize` stays non-zero forever. `rendermon()` then skips
every frame for that output, the screen keeps showing the last successfully
rendered frame (the overlapping floating layout), and the compositor looks
dead until restart.

### Why Super+F / Super+T triggers it

Switching the whole monitor to the float layout and back makes `tile()` issue
resize configures to *every* visible tiled client at once (their floating
geometry differs from the tiled geometry). Multiple outstanding configures at
the same time sharply increases the chance that at least one background client
does not ack promptly — leaving `c->resize` stuck and the output frozen.

This matches the upstream report #1203: *"one of the resize configure serials
is never cleared (confirmed) from `c->resize`. This is always a background
application."*

## The fix

Remove the manual pending-resize frame skip from `rendermon()`. wlroots >= 0.19
already handles intermediate surface states internally (scene-level damage
tracking + linux-drm-syncobj explicit sync), so the skip is redundant and is
the thing that turns a slow client into a permanent freeze:

```c
if (!wlr_scene_output_needs_frame(m->scene_output))
    return;

wlr_scene_output_commit(m->scene_output, NULL);
clock_gettime(CLOCK_MONOTONIC, &now);
wlr_scene_output_send_frame_done(m->scene_output, &now);
```

- `wlr_scene_output_needs_frame()` replaces the skip logic: only render when
  there is actual damage, which wlroots computes correctly including in-flight
  resizes.
- No XDG client can ever block presentation again.

This is the exact change proposed upstream in dwl PR !1208, applied to this
tree and adapted to keep the `client-opacity` per-frame buffer update.

## Compatibility

- Requires **wlroots >= 0.19** (`wlr_scene_output_needs_frame`). This system
  has wlroots 0.19.3.
- dwl built clean with `-Wall -Wextra -Werror=*` after the change.

## Verification

1. `make` — compiles clean.
2. Reboot into the new binary, open 3+ windows, `Super+F`, then `Super+T` —
   tile layout must restore immediately.
3. Repeat with a background window that is slow to commit (e.g. a game, a
   fullscreen app on another tag) to confirm no freeze.
