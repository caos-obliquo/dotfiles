#!/bin/bash
pkill -9 dwlb 2>/dev/null
pkill -9 swaybg 2>/dev/null
pkill -9 -f dwlb-status 2>/dev/null
pkill -9 wawa 2>/dev/null
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
    kapd &
    widle -t 300 wlock &
    playerctl-daemon.sh &
    (sleep 15 && mkdir -p /tmp/kt && cclip list | awk -F'\t' '/image/ {print $1}' | head -20 | while read -r id; do
        [ -f "/tmp/kt/$id.png" ] && continue
        cclip get "$id" | magick - -resize 96x96 "/tmp/kt/$id.png" 2>/dev/null
        sleep 0.5
    done) &
    ~/.local/bin/dwlb-status.sh | dwlb -status-stdin all
) &
exec dwl -s 'sh -c "wawa fill ~/walls/wall3.jpg & dwlb -font \"JetBrainsMono Nerd Font:size=16\" -no-ipc -no-hidden"'
