#!/bin/bash
# dwl session autostart — spawned by dwl via -s.
# stdin = dwl status protocol (title/tags/layout/selmon) → dwlb daemon.
# Env (WAYLAND_DISPLAY, DBUS, XDG_*) is inherited from start-dwl.sh.

# 1) bar daemon: renders the bar, reads dwl status from stdin
~/.local/bin/dwlb &
sleep 1 # let dwlb bind its IPC socket before the status feed connects

# 2) status text feed (vol/cpu/mem/temp/net/bat/time) → daemon socket
~/.local/bin/dwlb-status.sh | ~/.local/bin/dwlb -status-stdin all &

# 3) wallpaper
~/.local/bin/wawa fill "$HOME/walls/wall3.jpg" &

# 4) session daemons
mako &
kanshi &
widle -t 300 wlock &
~/.local/bin/playerctl-daemon.sh &

# 5) bus + portals + clipboard daemon (kaprica → wclipmenu pickers)
dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP=wlroots DBUS_SESSION_BUS_ADDRESS
systemctl --user import-environment DBUS_SESSION_BUS_ADDRESS
systemctl --user start kaprica.service
systemctl --user restart xdg-desktop-portal xdg-desktop-portal-wlr

# keep the -s child alive so dwl's status pipe never closes
wait
