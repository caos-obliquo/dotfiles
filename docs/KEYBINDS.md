# Keybinds (DraculaWL)

`MODKEY` = Super. Keyboard layout ABNT2 (Brazilian). Source of truth:
`home/.config/dwl/config.h` (vendored dwl), `wmenu` README, `wlock.c`.

## dwl — compositor

### Launchers

| Key | Action |
|---|---|
| `Super+d` | `wmenu-run` (app menu) |
| `Super+b` | `spawnorfocus` waterfox — focus if running, else spawn |
| `Super+Shift+Return` | `foot` terminal |
| `Super+Shift+L` | `wlock` (screen lock) |
| `Super+p` | `wclipmenu` (clipboard text picker) |
| `Super+Shift+P` | `wclipmenu image` (clipboard image picker) |

### Window navigation

| Key | Action |
|---|---|
| `Super+j` / `Super+k` | focus next / prev window |
| `Super+Shift+j` / `Super+Shift+k` | `movestack` — move window down / up the stack |
| `Super+Return` | `zoom` — promote focused window to master |

### Master area

| Key | Action |
|---|---|
| `Super+Shift+I` | `incnmaster +1` — more master slots |
| `Super+Shift+Y` | `incnmaster -1` — fewer master slots |
| `Super+h` / `Super+l` | `setmfact` — shrink / expand master area (5%) |
| `Super+o` / `Super+Shift+O` | `setopacity` +10% / -10% |

### Layouts

| Key | Action |
|---|---|
| `Super+t` | tile layout `[]=` |
| `Super+f` | float layout `><>` |
| `Super+m` | monocle `[M]` |
| `Super+Space` | toggle layout |
| `Super+Shift+Space` | `togglefloating` — float only focused window |
| `Super+e` | `togglefullscreen` |
| `Super+Tab` | view last tag |

### Kill / session

| Key | Action |
|---|---|
| `Super+q` | kill focused window |
| `Super+Shift+Q` | quit dwl |
| `Ctrl+Alt+Backspace` | quit dwl |
| `Ctrl+Alt+F1..F12` | switch VT |

### Tags

| Key | Action |
|---|---|
| `Super+1..9` | view tag 1..9 |
| `Super+Ctrl+1..9` | `toggleview` tag |
| `Super+Shift+1..9` | tag window to tag |
| `Super+Ctrl+Shift+1..9` | `toggletag` |
| `Super+0` | view all tags |
| `Super+Shift+)` | tag window to all tags |

### Monitors

| Key | Action |
|---|---|
| `Super+,` / `Super+.` | focus left / right monitor |
| `Super+Shift+<` / `>` | send window to left / right monitor |

### Media (FN keys)

| Key | Action |
|---|---|
| `XF86MonBrightnessDown/Up` | `brightnessctl` -5% / +5% |
| `XF86AudioMute` | `wpctl` mute toggle |
| `XF86AudioLowerVolume/RaiseVolume` | `wpctl` 5%- / 5%+ |
| `XF86AudioPlay` | `playerctl -p youtui` play/pause |
| `XF86AudioNext` | `playerctl -p youtui` next |
| `XF86AudioPrev` | `playerctl -p youtui` previous |

### Screenshots

| Key | Action |
|---|---|
| `Super+s` | area screenshot |
| `Super+Shift+S` | full screenshot |
| `Super+Ctrl+s` | focused window |
| `Super+Alt+s` | current monitor |

### Mouse

| Modifier | Button | Action |
|---|---|---|
| `Super` | left | move window (floats it) |
| `Super` | middle | `togglefloating` |
| `Super` | right | resize window (floats it) |

### Rules

| App | Rule |
|---|---|
| `wmenu-center` | floating, opacity 0.85 |
| waterfox | tag 9 |

## wmenu-caos — pickers

Emacs-style, hardcoded. Full table in the [wmenu-caos README](https://github.com/caos-obliquo/wmenu-caos).

| Key | Action |
|---|---|
| `Up`/`Ctrl+p`, `Down`/`Ctrl+n` | navigate matches |
| `PgUp`/`Alt+k`, `PgDn`/`Alt+j` | page up / down |
| `Return` / `Ctrl+j` / `Ctrl+m` | submit selected |
| `Shift+Return` | submit typed input |
| `Tab` | complete from selection |
| `Escape` / `Ctrl+c` / `Ctrl+g` / `Ctrl+[` | cancel |
| `Ctrl+a/e` | line start / end |
| `Ctrl+u/k` | delete to line start / end |
| `Ctrl+w` | delete word |
| `Ctrl+y` | paste clipboard |

## wlock — screen locker

Launched by `Super+Shift+L` and `widle -t 300 wlock` (5 min idle).

| Key | Action |
|---|---|
| printable | type password (masked) |
| `Return` / KP_Enter | submit |
| `Backspace` | delete last char |
| `Escape` | clear input |

Failed attempt turns the overlay red (`failonclear = 1`).

## mako — notifications

No keybinds configured (`home/.config/mako/config`) — click to dismiss, timeout 5s.

## Browser layer — Vimium (vimium-c on Waterfox)

Not a config in this repo — a browser extension that closes the keyboard
loop. Fundamental companion.

| Key | Action |
|---|---|
| `j` / `k` | scroll down / up |
| `f` | link hints (type chars to follow) |
| `H` / `L` | back / forward |
| `J` / `K` | prev / next tab |
| `d` / `u` | half-page down / up |
| `x` / `X` | close tab / restore tab |
| `/` | find in page |
| `g` + `g` / `G` | top / bottom of page |
