#!/bin/bash
# Battery block with dynamic icons based on level (dwlb format)

if [ -d /sys/class/power_supply/BAT0 ]; then
    bat=$(cat /sys/class/power_supply/BAT0/capacity)
    status=$(cat /sys/class/power_supply/BAT0/status)
    
    # Choose icon based on battery level
    if [ "$bat" -ge 90 ]; then
        icon="󰁹"
    elif [ "$bat" -ge 80 ]; then
        icon="󰂂"
    elif [ "$bat" -ge 70 ]; then
        icon="󰂁"
    elif [ "$bat" -ge 60 ]; then
        icon="󰂀"
    elif [ "$bat" -ge 50 ]; then
        icon="󰁿"
    elif [ "$bat" -ge 40 ]; then
        icon="󰁾"
    elif [ "$bat" -ge 30 ]; then
        icon="󰁽"
    elif [ "$bat" -ge 20 ]; then
        icon="󰁼"
    else
        icon="󰁺"
    fi
    
    # Color: green if charging, cyan if discharging
    if [ "$status" = "Charging" ]; then
        echo "^fg(50fa7b)󰂄 $bat%^fg()"
    else
        echo "^fg(8be9fd)$icon $bat%^fg()"
    fi
else
    echo ""
fi
