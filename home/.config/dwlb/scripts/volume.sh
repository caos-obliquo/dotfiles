#!/bin/bash
# Volume block with dynamic icons (dwlb format)

if command -v pamixer &> /dev/null; then
    vol=$(pamixer --get-volume)
    
    # Choose icon based on mute status and volume level
    if [ "$(pamixer --get-mute)" = "true" ]; then
        icon="󰖁"
    elif [ "$vol" -ge 70 ]; then
        icon="󰕾"
    elif [ "$vol" -ge 30 ]; then
        icon="󰖀"
    else
        icon="󰕿"
    fi
    
    echo "^fg(ff79c6)$icon $vol%^fg()"
else
    echo ""
fi
