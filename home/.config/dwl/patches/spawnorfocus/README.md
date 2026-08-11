# spawnorfocus

One key per app: **focus an already-running client, or spawn it if absent**.

`Super+b` → `firefox`: existing firefox window gets focused (tag view switches
to it), otherwise firefox starts.

Source: [dwl-patches spawnorfocus](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/spawnorfocus)
Author: GravityShark, Ben Collerson, clicseo, wochap, Louis-Michel Raynauld
Applied: in-tree (see `spawnorfocus()` in dwl.c)

## arg->v convention (important)

Commands bound to `spawnorfocus` must have **3 elements**:

```c
{ "firefox", NULL, "firefox" }  /* cmd, NULL, match-substring */
```

- 3rd element = substring matched against `app_id`/`title` (`strstr`).
- If 3rd element is `NULL`, the command name (1st element) is used as the
  needle, e.g. `{ "foot", NULL, NULL }`.
- The matcher loop counts to the first `NULL`, so the 3rd slot is required —
  a 2-element array reads past the end (upstream patch quirk, kept as-is).

## Config (this fork)

```c
static const char *browsercmd[] = { "firefox", NULL, "firefox" };
{ MODKEY, XKB_KEY_b, spawnorfocus, { .v = browsercmd } },
```

`config.h` and `config.def.h` both updated. Terminal (`Super+Shift+Return`)
stays a plain `spawn` — new terminals should always open, not re-focus.

## Behavior details

- On match: `selmon = c->mon`, tag view switches to the client's tag,
  `focusclient(c, 1)`, `arrange()`.
- No match: falls through to `spawn(arg)`.

## Verify

- Build: clean (`-Wall -Wextra -Werror=*`).
- Manual: open firefox → `Super+b` focuses it; close it → `Super+b` reopens.
