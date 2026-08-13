export XDG_CONFIG_HOME="$HOME/.config"
export XDG_CACHE_HOME="$HOME/.cache"
export XDG_DATA_HOME="$HOME/.local/share"
export XDG_STATE_HOME="$HOME/.local/state"
export MOZ_ENABLE_WAYLAND=1
export QT_QPA_PLATFORM=wayland
export SDL_VIDEODRIVER=wayland

# start the dwl session on the primary tty at login (persists across reboots)
if [[ "$(tty)" == /dev/tty1 ]] && [[ -z "$WAYLAND_DISPLAY" ]] && [[ -z "$DISPLAY" ]]; then
    exec start-dwl.sh
fi
