#!/usr/bin/env bash
# Optimized clipboard picker for wmenu-caos
# Requires: kapc, cclip, wmenu-caos

THUMB_DIR="/tmp/kt"
mkdir -p "$THUMB_DIR"

# Stream text then images directly into wmenu
selected=$({
    # 1. Text First
    kapc search "" -L

    # 2. Images Second
    img_count=0
    while IFS=$'\t' read -r id mime _; do
        [[ "$mime" == *image* ]] || continue
        [ "$img_count" -ge 10 ] && break
        thumb="$THUMB_DIR/$id.png"
        [ -f "$thumb" ] && echo "[img:$thumb] $id" && ((img_count++))
    done < <(cclip list)
} | wmenu -c -l 15 -p "clip:" \
    -N 282a36 -n f8f8f2 -S bd93f9 -s f8f8f2 \
    -f "JetBrainsMono Nerd Font:size=13")

[ -z "$selected" ] && exit 0

if [[ "$selected" =~ ^[0-9]+$ ]]; then
    cclip copy "$selected" && wl-paste --type image/png | wl-copy --type image/png
else
    echo "$selected" | awk '{print $1}' | xargs -I{} kapc copy -i {}
fi
