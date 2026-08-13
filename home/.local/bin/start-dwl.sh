#!/bin/bash
# dwl session launcher — run from tty1 login (see ~/.config/zsh/.zprofile)
# dwl spawns dwl-autostart.sh as its -s child; dwl's status protocol
# (tags/title/layout/selmon) flows into that script's stdin, which the
# dwlb daemon consumes.

pkill -9 dwlb 2>/dev/null
pkill -9 wawa 2>/dev/null
pkill -9 -f dwl-autostart 2>/dev/null

export PATH="$HOME/.local/bin:/usr/local/bin:$PATH"
export XDG_SESSION_TYPE=wayland
export XDG_CURRENT_DESKTOP=wlroots
export XDG_SESSION_DESKTOP=wlroots
export __EGL_VENDOR_LIBRARY_FILENAMES=/usr/share/glvnd/egl_vendor.d/50_mesa.json
export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/$UID/bus"
export WLR_RENDERER=gles2

exec dwl -s "$HOME/.local/bin/dwl-autostart.sh"
