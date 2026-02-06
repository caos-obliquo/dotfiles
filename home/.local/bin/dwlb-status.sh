#!/bin/bash
while true; do
    # Volume (pink) - dynamic icon based on level
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
        vol_display="^fg(ff79c6)$vol_icon $vol^fg()"
    else
        vol_display=""
    fi
    
    # CPU (red)
    cpu=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{printf "%.0f", 100 - $1}')
    
    # Memory (yellow)
    mem=$(free -h | awk '/^Mem:/ {print $3}')
    
    # Temperature (orange)
    if command -v sensors &> /dev/null; then
        temp=$(sensors 2>/dev/null | grep -m1 'Package id 0:\|Core 0:' | awk '{print $3}' | tr -d '+°C')
        [ -z "$temp" ] && temp=$(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null | awk '{printf "%.0f", $1/1000}')
        temp_display="^fg(ffb86c)󰔏 ${temp}°^fg()"
    else
        temp_display=""
    fi
    
    # Network (green) - dynamic WiFi icon based on signal strength
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
        net_display="^fg(50fa7b)$wifi_icon $wifi%^fg()"
    else
        net_display="^fg(6272a4)󰤮^fg()"
    fi
    
    # Battery (cyan/green) - dynamic icon based on level
    if [ -d /sys/class/power_supply/BAT0 ]; then
        bat=$(cat /sys/class/power_supply/BAT0/capacity)
        status=$(cat /sys/class/power_supply/BAT0/status)
        
        # Choose icon based on battery level
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
        
        # Color: green if charging, cyan if discharging
        if [ "$status" = "Charging" ]; then
            bat_display="^fg(50fa7b)󰂄 $bat%^fg()"
        else
            bat_display="^fg(8be9fd)$bat_icon $bat%^fg()"
        fi
    else
        bat_display=""
    fi
    
    # Time (purple) - no icon
    time=$(date '+%H:%M')
    
    # Build status
    echo "$vol_display | ^fg(ff5555)󰻠 $cpu%^fg() | ^fg(f1fa8c)󰍛 $mem^fg() | $temp_display | $net_display | $bat_display | ^fg(bd93f9)$time^fg()"
    
    sleep 2
done
