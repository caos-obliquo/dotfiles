# swallow

Terminal swallow: when a terminal spawns a client, the terminal window is
hidden and the client takes over its workspace slot. Closing the client
restores the terminal.

- Source: https://codeberg.org/dwl/dwl-patches/patches/swallow (swallow-0.8.patch)
- Applied: hand-merged to 0.8-dev fork (9-field Rule struct collision,
  `typedef struct Client Client;` forward decl, `BORDERPX()` macro, dwl-ipc
  bar_geometry and opacity fork additions)
- Binds: `Mod+a` toggle swallow focused into next client, `Mod+Shift+a` toggle
  auto-swallow (autoswallow = on by default, `enableautoswallow`)
- Rule fields: foot is `isterm=1, noswallow=1` (auto-swallow host); other
  rules `noswallow=0` so child apps get swallowed
- Deps: none (uses `/proc/<pid>/stat` parent walk, no extra libs)
