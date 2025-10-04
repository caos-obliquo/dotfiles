#!/bin/bash

echo "This will modify system files. Review changes before proceeding."
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

# WirePlumber configs (safe to copy)
sudo mkdir -p /etc/wireplumber/bluetooth.lua.d
sudo mkdir -p /etc/wireplumber/main.lua.d
sudo cp system-configs/51-bluez-config.lua /etc/wireplumber/bluetooth.lua.d/
sudo cp system-configs/51-bluetooth-priority.lua /etc/wireplumber/main.lua.d/

echo "System configs installed. See system-configs/*.changes for manual edits needed."
