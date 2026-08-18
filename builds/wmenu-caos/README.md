# builds/wmenu-caos — wmenu build config

Deploy overrides for [caos-obliquo/wmenu-caos](https://github.com/caos-obliquo/wmenu-caos)
(meson+ninja; `autorice-deploy.sh` copies these into the clone before build):

- `config.h` — Dracula colors + `wmenu_width 720` (centered fallback width)
- `menu.c` — pill styling (borderless, height = bar height)
- `wayland.c` — the dwl-IPC positioning path (`POSITION_TOP_CENTER`)

## How the pill works

`wmenu-run -t` (bound to `Super+d` in dwl) opens a layer-shell surface
anchored top-left. In `POSITION_TOP_CENTER` mode it subscribes to
`zdwl_ipc_manager_v2` and waits for the `bar_geometry` event from dwl:

- **Position**: margins `left = middle_x`, `right = output_width − middle_x − middle_width` → exactly over the bar's middle title area.
- **Height**: `bar_height + list_height` (list rows only when `-l N`), drift-free on redraws.
- **Colors**: adopts dwl's `SchemeNorm` bg/fg from the event (normal+prompt bg/fg; selection fg/bg swapped).

Fallback without IPC: centered at `wmenu_width` (no bar geometry available).
