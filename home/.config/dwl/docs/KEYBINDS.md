# dwl keybinds (this build)

Source of truth: `config.h`. Layout: **ABNT2 (Brazilian)**, `MODKEY` = Super.

## Launchers

| Key | Action |
|---|---|
| `Super+d` | `wmenu-run` (app menu) |
| `Super+b` | `spawnorfocus` firefox — focus if running, else spawn |
| `Super+g` | `passmenu` (password store; requires passmenu) |
| `Super+Shift+D` | `makoctl dismiss` — dismiss notification |
| `Super+Shift+Return` | `foot` terminal |
| `Super+Shift+L` | `wlock` (screen lock) |
| `Super+p` | `wclipmenu` (clipboard text picker) |
| `Super+Shift+P` | `wclipmenu image` (clipboard image picker) |

## Window navigation

| Key | Action |
|---|---|
| `Super+j` / `Super+k` | focus next / prev window |
| `Super+Shift+j` / `Super+Shift+k` | `movestack` — move window down / up the stack |
| `Super+Return` | `zoom` — promote focused window to master |

## Master area

| Key | Action |
|---|---|
| `Super+Shift+I` | `incnmaster +1` — one more master slot |
| `Super+Shift+Y` | `incnmaster -1` — one fewer master slot |
| `Super+h` / `Super+l` | `setmfact` — shrink / expand master area (5%) |
| `Super+o` / `Super+Shift+O` | `setopacity` +10% / -10% (client-opacity patch) |

## Layouts

| Key | Action |
|---|---|
| `Super+t` | tile layout `[]=` |
| `Super+f` | float layout `><>` |
| `Super+m` | monocle `[M]` |
| `Super+Space` | toggle layout |
| `Super+Shift+Space` | `togglefloating` — float only the focused window |
| `Super+e` | `togglefullscreen` |
| `Super+Tab` | view last tag |

## Window kill

| Key | Action |
|---|---|
| `Super+q` | `killclient` — close focused window |

## Tags

| Key | Action |
|---|---|
| `Super+1..9` | view tag 1..9 |
| `Super+Ctrl+1..9` | `toggleview` tag |
| `Super+Shift+1..9` (`!@#...`) | `tag` window to tag |
| `Super+Ctrl+Shift+1..9` | `toggletag` window tag |
| `Super+0` | view all tags |
| `Super+Shift+`)` (parenright) | tag window to all tags |

## Monitors

| Key | Action |
|---|---|
| `Super+,` / `Super+.` | focus left / right monitor |
| `Super+Shift+<` / `Super+Shift+>` | send window to left / right monitor |

## Media (FN keys)

| Key | Action |
|---|---|
| `XF86MonBrightnessDown/Up` | `brightnessctl -5%` / `+5%` |
| `XF86AudioMute` | `wpctl set-mute` toggle |
| `XF86AudioLowerVolume/RaiseVolume` | `wpctl` 5%- / 5%+ |

## Screenshots

| Key | Action |
|---|---|
| `Super+s` | area screenshot (grim → wl-copy) |
| `Super+Shift+S` | full screenshot |
| `Super+Ctrl+s` | focused window |
| `Super+Alt+s` | current monitor |

## Session

| Key | Action |
|---|---|
| `Super+Shift+Q` | quit dwl |
| `Ctrl+Alt+Backspace` (Terminate_Server) | quit dwl |
| `Ctrl+Alt+F1..F12` | `chvt` — switch VT |

## Mouse

| Modifier | Button | Action |
|---|---|---|
| `Super` | left | `moveresize` — move window (floats it) |
| `Super` | middle | `togglefloating` |
| `Super` | right | `moveresize` — resize window (floats it) |

## Rules (config.h)

| App | Rule |
|---|---|
| `wmenu-center` | floating, opacity 0.85 |
| Gimp (example) | floating, opacity 1.0 |
| firefox (example) | tag 9 |

## Layouts available

| Symbol | Name |
|---|---|
| `[]=` | tile (default, `mfact 0.55`, `nmaster 1`) |
| `><>` | floating |
| `[M]` | monocle |

## Keyboard (layout)

ABNT2 Brazilian. `CapsLock` acts as **Escape** (`xkb_rules.options =
"caps:swapescape"`) — remap your muscle memory accordingly.
