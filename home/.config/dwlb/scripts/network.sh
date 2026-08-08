#!/bin/bash
# network block with dynamic wifi signal icons (dwlb format)

if grep -q "^\s*w" /proc/net/wireless 2>/dev/null; then
    wifi=$(grep "^\s*w" /proc/net/wireless | awk '{print int($3 * 100 / 70)}')
    
    # choose icon based on signal strength
    if [ "$wifi" -ge 80 ]; then
        icon="󰤨"
    elif [ "$wifi" -ge 60 ]; then
        icon="󰤥"
    elif [ "$wifi" -ge 40 ]; then
        icon="󰤢"
    elif [ "$wifi" -ge 20 ]; then
        icon="󰤟"
    else
        icon="󰤯"
    fi
    
    echo "^fg(50fa7b)$icon $wifi%^fg()"
else
    # no wifi - show disconnected icon
    echo "^fg(6272a4)󰤮^fg()"
fi
