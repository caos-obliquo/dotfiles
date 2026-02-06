#!/bin/bash
# Temperature block (dwlb format)

if command -v sensors &> /dev/null; then
    temp=$(sensors 2>/dev/null | grep -m1 'Package id 0:\|Core 0:' | awk '{print $3}' | tr -d '+°C')
    [ -z "$temp" ] && temp=$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null | awk '{printf "%.0f", $1/1000}')
    
    if [ -n "$temp" ]; then
        echo "^fg(ffb86c)󰔏 ${temp}°^fg()"
    else
        echo ""
    fi
else
    temp=$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null | awk '{printf "%.0f", $1/1000}')
    if [ -n "$temp" ]; then
        echo "^fg(ffb86c)󰔏 ${temp}°^fg()"
    else
        echo ""
    fi
fi
