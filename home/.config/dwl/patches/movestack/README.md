# movestack

Move the focused window up/down in the tiling stack with the keyboard.

- `Super+Shift+J` — move window **down** the stack
- `Super+Shift+K` — move window **up** the stack

Source: [dwl-patches movestack (0.8)](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/movestack)
Author: wochap, sam-barr, Dmitry Zakharchenko, Abanoub8, Nikita Ivanov
Applied: in-tree (see `movestack()` in dwl.c)

## Behavior

- `arg->i > 0`: scan the `clients` list forward from the focused client for
  the next **visible** client, insert the focused client after it.
- `arg->i < 0`: scan backward for the previous **visible** client and insert
  before it.
- Hidden (other-tag) clients are skipped, so moves track the on-screen stack
  order.
- **Cyclic**: moving down from the last visible window wraps to the top of the
  stack; moving up from the first wraps to the bottom.

## Implementation note (validated)

The wrap past the list sentinel uses
`c = wl_container_of(&clients, c, link)`, which yields a pointer whose
`->link` member is the real `clients` head node. Only `c->link` is ever used
(insert target), and `VISIBLEON()` is short-circuited by the sentinel check,
so no garbage dereference or list corruption occurs. Verified against both the
0.7 and 0.8 upstream variants (identical logic).

## Config (this fork)

```c
{ MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_j, movestack, { .i = +1 } },
{ MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_k, movestack, { .i = -1 } },
```

`config.h` and `config.def.h` both updated. No conflicts with existing binds
(`Super+Shift+J/K` were free).

## Verify

- Build: clean (`-Wall -Wextra -Werror=*`).
- Manual: 3 tiled windows, focus the middle one, `Super+Shift+j`/`k` — order
  changes, `Super+Shift+j` from the bottom wraps to the top.
