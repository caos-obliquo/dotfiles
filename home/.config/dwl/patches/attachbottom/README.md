# attachbottom

New windows are placed at the **bottom** of the client stack instead of the
master slot. Focus still goes to the new window; it just tiles in the stack
area. Promote with `Super+Return` (`zoom`) or reorder with `movestack`.

Source: [dwl-patches attachbottom](https://codeberg.org/dwl/dwl-patches/src/branch/main/patches/attachbottom)
Author: Ben Collerson, Aurel Weinhold
Applied: in-tree (see `mapnotify()` in dwl.c)

## Behavior

- `wl_list_insert(clients.prev, &c->link)` inserts the mapped client at the
  tail of the `clients` list → last stack position.
- No config needed. No keybinds.

## Verify

- Build: clean (`-Wall -Wextra -Werror=*`).
- Manual: open 3+ terminals — each new one must tile in the stack area, not
  replace the master.
