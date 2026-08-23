# bottomstack

Two extra tiling layouts: `bstack` (master on top, stack columns below, "TTT")
and `bstackhoriz` (master on top, stack rows below, "===").

- Source: https://codeberg.org/dwl/dwl-patches/patches/bottomstack (bottomstack-0.8.patch)
- Applied: clean, 0.8-dev fork compatible (binds adapted: upstream Mod+o
  collides with our setopacity, so bottomstack = Mod+u, horizontal = Mod+Shift+U)
- Binds: `Mod+u` bstack, `Mod+Shift+U` bstackhoriz, `Mod+Space` cycles layouts
- Deps: none
