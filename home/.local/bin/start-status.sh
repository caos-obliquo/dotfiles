#!/bin/bash
pkill -9 -f dwlb-status
~/.local/bin/dwlb-status.sh | dwlb -status-stdin all &
