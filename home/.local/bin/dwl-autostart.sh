#!/bin/bash
# dwl session autostart — spawned by dwl via -s. stdin = dwl status protocol.
# Spawns session services, then EXECs dwlb so the bar reads dwl's status
# protocol (tags/title/layout/selmon) directly from stdin (see dwlb(1)).

# 1) status text feed (vol/cpu/mem/temp/net/bat/time) → dwlb daemon socket
#    (client retries every line until the daemon's socket exists)
~/.local/bin/dwlb-status.sh | ~/.local/bin/dwlb -status-stdin all &

# 2) wallpaper
~/.local/bin/wawa fill "$HOME/walls/wall3.jpg" &

# 3) session daemons
mako &
kanshi &
widle -t 300 wlock &
~/.local/bin/playerctl-daemon.sh &

# 4) bus + portals + clipboard daemon (kaprica → wclipmenu pickers)
dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP=wlroots DBUS_SESSION_BUS_ADDRESS
systemctl --user import-environment DBUS_SESSION_BUS_ADDRESS
systemctl --user start kaprica.service
systemctl --user restart xdg-desktop-portal xdg-desktop-portal-wlr

# 5) THE BAR: dwlb becomes the -s child; dwl's status pipe flows into its
#    stdin (tags/title/layout/selmon). MUST be exec — running dwlb as a
#    background job makes it exit silently within ~200ms.
exec ~/.local/bin/dwlb
