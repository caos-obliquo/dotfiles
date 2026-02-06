#!/bin/bash

# Kill everything
pkill -9 dwlb
pkill -9 swaybg
pkill -9 -f dwlb-status

# Start dwl with JUST dwlb and wallpaper (no status)
exec dbus-launch --exit-with-session dwl -s 'sh -c "swaybg -i ~/walls/wall3.jpg -m fill & dwlb -font \"JetBrainsMono Nerd Font:size=16\" -no-ipc -no-hidden"'
