#!/bin/bash
# Memory usage block (dwlb format)

mem=$(free -h | awk '/^Mem:/ {print $3}')
echo "^fg(f1fa8c)󰍛 $mem^fg()"
