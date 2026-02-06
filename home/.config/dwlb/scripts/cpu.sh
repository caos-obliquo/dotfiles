#!/bin/bash
# CPU usage block (dwlb format)

cpu=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{printf "%.0f", 100 - $1}')
echo "^fg(ff5555)󰻠 $cpu%^fg()"
