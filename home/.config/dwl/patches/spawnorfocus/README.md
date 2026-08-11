# spawnorfocus

One key per app: **focus an already-running client, or spawn it if absent**.

`Super+b` → `firefox`: existing firefox window gets focused (tag view switches
to it), otherwise firefox starts.

Source: [dwl-patches spawnorfocus](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/spawnorfocus)
Author: GravityShark, Ben Collerson, clicseo, wochap, Louis-Michel Raynauld
Applied: in-tree (see `spawnorfocus()` in dwl.c)

## arg->v convention

Commands bound to `spawnorfocus`:

```c
{ "firefox", "firefox", NULL }  /* cmd, needle, terminator */
```

- 2nd element = substring matched against `app_id`/`title` (`strstr`).
- If 2nd element is `NULL`, the command name (1st element) is used as the
  needle, e.g. `{ "foot", NULL, NULL }`.
- `v[1]` is always readable — argv is NULL-terminated, so no out-of-bounds
  read (upstream's `{cmd, NULL, needle}` layout read `v[2]` past short
  arrays; fixed in-tree, see "Deviations").

## Config (this fork)

```c
static const char *browsercmd[] = { "firefox", "firefox", NULL };
{ MODKEY, XKB_KEY_b, spawnorfocus, { .v = browsercmd } },
```

`config.h` and `config.def.h` both updated. Terminal (`Super+Shift+Return`)
stays a plain `spawn` — new terminals should always open, not re-focus.

## Deviations from upstream

- argv layout is `{cmd, needle, NULL}` (upstream: `{cmd, NULL, needle}`).
  Same semantics, no OOB read on 2-element arrays.
- Redundant `selmon = c->mon` kept only for the tagset check before
  `focusclient()` (which sets `selmon` itself).

## Behavior details

- On match: `selmon = c->mon`, tag view switches to the client's tag,
  `focusclient(c, 1)`, `arrange()`.
- No match: falls through to `spawn(arg)`.

## Verify

- Build: clean (`-Wall -Wextra -Werror=*`).
- Manual: open firefox → `Super+b` focuses it; close it → `Super+b` reopens.
