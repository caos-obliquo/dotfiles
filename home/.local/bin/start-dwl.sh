#!/bin/bash
pkill -9 dwlb 2>/dev/null
pkill -9 swaybg 2>/dev/null
pkill -9 -f dwlb-status 2>/dev/null
export XDG_SESSION_TYPE=wayland
export XDG_CURRENT_DESKTOP=wlroots
export XDG_SESSION_DESKTOP=wlroots
(
    sleep 8
    dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP=wlroots
    systemctl --user restart xdg-desktop-portal xdg-desktop-portal-wlr
    kapd &
    ~/.local/bin/dwlb-status.sh | dwlb -status-stdin all
) &
exec dbus-run-session dwl -s 'sh -c "swaybg -i ~/walls/wall3.jpg -m fill & dwlb -font \"JetBrainsMono Nerd Font:size=16\" -no-ipc -no-hidden"'
