#!/usr/bin/env bash
kapc search "" -L | wmenu -c -l 15 | awk '{print $1}' | xargs -I{} kapc copy -i {}
