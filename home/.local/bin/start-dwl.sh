#!/bin/bash
pkill -9 dwlb 2>/dev/null
pkill -9 -f dwlb-status 2>/dev/null
pkill -9 wawa 2>/dev/null
export PATH="$HOME/.local/bin:/usr/local/bin:$PATH"
export XDG_SESSION_TYPE=wayland
export XDG_CURRENT_DESKTOP=wlroots
export XDG_SESSION_DESKTOP=wlroots
export __EGL_VENDOR_LIBRARY_FILENAMES=/usr/share/glvnd/egl_vendor.d/50_mesa.json
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus
(
    sleep 8
    dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP=wlroots DBUS_SESSION_BUS_ADDRESS
    systemctl --user import-environment DBUS_SESSION_BUS_ADDRESS
    systemctl --user restart xdg-desktop-portal xdg-desktop-portal-wlr
    mako &
    kanshi &
    widle -t 300 wlock &
    playerctl-daemon.sh &
    ~/.local/bin/dwlb-status.sh | dwlb -status-stdin all
) &
export WLR_RENDERER=gles2
exec dwl -s 'sh -c "wawa fill ~/walls/wall3.jpg"'
