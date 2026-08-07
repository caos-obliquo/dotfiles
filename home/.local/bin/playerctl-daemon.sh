#!/bin/bash
STATE_FILE="/tmp/youtui-state"

cleanup() {
    echo "|" >"$STATE_FILE"
    exit 0
}
trap cleanup INT TERM EXIT

echo "|" >"$STATE_FILE"

playerctl -p youtui --follow metadata --format \
    '{{status}}|{{artist}}|{{title}}' 2>/dev/null | while IFS='|' read -r s a t; do
    case "$s" in
    Playing) echo "|$a|$t" >"$STATE_FILE" ;;
    Paused) echo "⏸|$a|$t" >"$STATE_FILE" ;;
    *) echo "⏹|" >"$STATE_FILE" ;;
    esac
done

echo "|" >"$STATE_FILE"
