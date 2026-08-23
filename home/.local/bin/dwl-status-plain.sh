#!/bin/bash
# Plain-text status for dwl native bar (no ^fg() markup — drwl renders raw).
while true; do
    if command -v pamixer &> /dev/null; then
        vol=$(pamixer --get-volume)
        if [ "$(pamixer --get-mute)" = "true" ]; then
            vol_icon="󰖁"
        elif [ "$vol" -ge 70 ]; then
            vol_icon="󰕾"
        elif [ "$vol" -ge 30 ]; then
            vol_icon="󰖀"
        else
            vol_icon="󰕿"
        fi
        vol_display="$vol_icon $vol"
    else
        vol_display=""
    fi

    cpu=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{printf "%.0f", 100 - $1}')

    mem=$(free -h | awk '/^Mem:/ {print $3}')

    if command -v sensors &> /dev/null; then
        temp=$(sensors 2>/dev/null | grep -m1 'Package id 0:\|Core 0:' | awk '{print $3}' | tr -d '+°C')
        [ -z "$temp" ] && temp=$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null | awk '{printf "%.0f", $1/1000}')
        temp_display="󰔏 ${temp}°"
    else
        temp_display=""
    fi

    if grep -q "^\s*w" /proc/net/wireless 2>/dev/null; then
        wifi=$(grep "^\s*w" /proc/net/wireless | awk '{print int($3 * 100 / 70)}')
        if [ "$wifi" -ge 80 ]; then
            wifi_icon="󰤨"
        elif [ "$wifi" -ge 60 ]; then
            wifi_icon="󰤥"
        elif [ "$wifi" -ge 40 ]; then
            wifi_icon="󰤢"
        elif [ "$wifi" -ge 20 ]; then
            wifi_icon="󰤟"
        else
            wifi_icon="󰤯"
        fi
        net_display="$wifi_icon $wifi%"
    else
        net_display="󰤮"
    fi

    if [ -d /sys/class/power_supply/BAT0 ]; then
        bat=$(cat /sys/class/power_supply/BAT0/capacity)
        status=$(cat /sys/class/power_supply/BAT0/status)

        if [ "$bat" -ge 90 ]; then
            bat_icon="󰁹"
        elif [ "$bat" -ge 80 ]; then
            bat_icon="󰂂"
        elif [ "$bat" -ge 70 ]; then
            bat_icon="󰂁"
        elif [ "$bat" -ge 60 ]; then
            bat_icon="󰂀"
        elif [ "$bat" -ge 50 ]; then
            bat_icon="󰁿"
        elif [ "$bat" -ge 40 ]; then
            bat_icon="󰁾"
        elif [ "$bat" -ge 30 ]; then
            bat_icon="󰁽"
        elif [ "$bat" -ge 20 ]; then
            bat_icon="󰁼"
        else
            bat_icon="󰁺"
        fi

        if [ "$status" = "Charging" ]; then
            bat_display="󰂄 $bat%"
        else
            bat_display="$bat_icon $bat%"
        fi
    else
        bat_display=""
    fi

    time=$(date '+%H:%M')

    zsh_display="󰌓"
    echo "$vol_display | 󰻠 $cpu% | 󰍛 $mem | $zsh_display | $temp_display | $net_display | $bat_display | $time"

    sleep 2
done
