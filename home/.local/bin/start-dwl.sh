#!/bin/bash
# dwl session launcher — wallpaper + colored status feed (^fg markup, piped into dwl stdin).
pkill -9 wawa 2>/dev/null
export PATH="$HOME/.local/bin:/usr/local/bin:$PATH"
export XDG_SESSION_TYPE=wayland
export XDG_CURRENT_DESKTOP=wlroots
export XDG_SESSION_DESKTOP=wlroots
export __EGL_VENDOR_LIBRARY_FILENAMES=/usr/share/glvnd/egl_vendor.d/50_mesa.json
export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$UID/bus"
(
    sleep 8
    dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP=wlroots DBUS_SESSION_BUS_ADDRESS
    systemctl --user import-environment DBUS_SESSION_BUS_ADDRESS
    systemctl --user start kaprica.service
    systemctl --user restart xdg-desktop-portal xdg-desktop-portal-wlr
    mako &
    kanshi &
    widle -t 300 wlock &
    playerctl-daemon.sh &
) &
export WLR_RENDERER=gles2
exec ~/.local/bin/dwl-status.sh | dwl -s 'sh -c "wawa fill ~/walls/wall5-16x10.jpg"'
