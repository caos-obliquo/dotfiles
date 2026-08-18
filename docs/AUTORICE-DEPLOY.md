# autorice-deploy.sh — One-shot DraculaWL deployment

**Purpose:** turn a fresh Arch install into a running DraculaWL desktop in one
command. Clones every fork, applies per-fork build configs from `builds/`,
builds and installs them, copies `home/` → `~/`, enables user services.

```bash
./autorice-deploy.sh
```

---

## What it does (step by step)

| Phase | Function | What happens |
|---|---|---|
| 0 | `check_system` | Arch + internet + sudo + non-root |
| 1 | `setup_pacman` | ParallelDownloads, Color, ILoveCandy, Brazil mirrors |
| 2 | `install_base_system` | base-devel, git, zsh, nvim, tmux, fzf, ripgrep, fd, atuin, pass, lf |
| 3 | `setup_amd_gpu` | amd-ucode, mesa, vulkan-radeon, early KMS (`amdgpu` in initramfs) |
| 4 | `install_wayland_stack` | wlroots0.18, libinput, xwayland, fcft, meson, ninja, pkgconf |
| 5 | `install_applications` | foot, grim/slurp, mako, widle, pamixer, playerctl, fonts, firefox, mpv, imv, zathura, ranger, thunar |
| 6 | `install_portals` | xdg-desktop-portal-wlr + gtk |
| 7 | `setup_power_management` | TLP + systemd-rfkill masking |
| 8 | `setup_network` | NetworkManager |
| 9 | `clone_dotfiles` | `git clone https://github.com/caos-obliquo/dotfiles` → `~/dotfiles`, `cp -r home/. ~/` |
| 10 | `build_dwl` | Clone caos-obliquo/dwl → copy `builds/dwl/config.h` → `make && sudo make install` |
| 11 | `build_wmenu` | Clone caos-obliquo/wmenu-caos → copy `builds/wmenu-caos/config.h` + `menu.c` + `wayland.c` → meson+ninja install |
| 12 | `build_kaprica` | Clone caos-obliquo/kaprica → meson install to `~/.local` → enable `kaprica.service` (user) |
| 13 | `build_wclipmenu` | Clone caos-obliquo/wclipmenu → patch `KAPC_PATH` → make install to `~/.local` |
| 14 | `build_ccze` | Clone cornet/ccze → autotools build to `~/.local` |
| 15 | `setup_zsh_plugins` | zsh-vi-mode, zsh-z, `chsh -s zsh` |
| 16 | `setup_tmux_plugins` | TPM → `prefix+I` inside tmux to install plugins |
| 17 | `setup_walls` | `mkdir -p ~/walls` (expects `wall3.jpg`) |
| 18 | `setup_atuin` | Shell completions |
| 19 | `setup_neovim` | Clone caos.nvim, npm global prefix, pynvim, system linters, Java 21, Mason packages |

---

## Per-fork build configs (vendored in this repo)

Each fork has its compile-time config checked into `builds/<tool>/`:

```
builds/
├── dwl/
│   └── config.h              # patched dwl: 7 patches, keybinds, colors, opacity
├── dwlb-geometry/
│   └── config.h              # bar geometry, colors, status format
└── wmenu-caos/
    ├── config.h              # wmenu colors, fonts, height, launcher behavior
    └── menu.c                # menu item rendering (thumbnails for wclipmenu image)
```

The deploy script copies these into each source tree before building.

---

## Runtime services enabled

| Service | Type | Started by |
|---|---|---|
| `NetworkManager` | system | `systemctl enable --now` |
| `tlp` | system | `systemctl enable --now` |
| `kaprica.service` | user | `systemctl --user enable --now` (inside graphical session) |
| `dwl` | TTY login | `.zprofile` → `start-dwl.sh` |

---

## Post-deploy checklist

```
1. Add wallpaper: cp your_wallpaper.jpg ~/walls/wall3.jpg
2. Reboot: sudo reboot
3. Login TTY1 → dwl starts automatically
4. Inside tmux: prefix+I (install TPM plugins)
```

---

## Customization points

### Change git remotes (private forks)
Edit the `*_REPO` variables at the top of `autorice-deploy.sh`:

```bash
DWL_REPO="git@github.com:youruser/dwl.git"
DWLB_REPO="git@github.com:youruser/dwlb-geometry.git"
WMENU_REPO="git@github.com:youruser/wmenu-caos.git"
```

### Add/remove packages
Edit the `pacman -S` arrays in `install_base_system`, `install_applications`,
`install_wayland_stack`, etc.

### Skip a build step
Comment out the function call in `main()`:

```bash
# build_wmenu   # skip if you don't need wmenu-caos
```

### Run only specific phases
Source the script and call individual functions:

```bash
source ./autorice-deploy.sh
build_dwl
build_dwlb
```

---

## Assumptions & constraints

- **Arch Linux only** (pacman, systemd, makepkg)
- **Fresh install** — no config migration, no backup of existing dots
- **sudo required** — for system packages, `make install` to `/usr/local`
- **Internet** — clones from GitHub, reflector for mirrors
- **Brazil mirrors** — `reflector --country Brazil`; change in `setup_pacman`
- **EliteBook 645 G11** — AMD 680M, early KMS configured for amdgpu
- **SSH key** — wmenu-caos clone tries SSH first, falls back to HTTPS

---

## Troubleshooting

| Issue | Fix |
|---|---|
| `sudo` password prompt fails | Run `sudo -v` manually first, or configure `NOPASSWD` for your user |
| `meson: command not found` | `sudo pacman -S meson ninja` (installed in `install_wayland_stack`) |
| kaprica service won't enable | Must run inside graphical session; enable after first login: `systemctl --user enable --now kaprica` |
| wclipmenu build fails `KAPC_PATH` | Script patches `src/wclipmenu.h` — verify `$HOME` expansion worked |
| ccze build fails | Needs `ncurses pcre autoconf automake` (installed in `build_ccze`) |
| dwl/dwlb build fails | Check `config.h` exists in `builds/<tool>/`; copy from `config.def.h` if missing |

---

## Related docs

- [Start-dwl session entry](home/.local/bin/start-dwl.sh) — what runs at TTY login
- [dwl patches](home/.config/dwl/patches/README.md) — 7 patches applied
- [dwlb-geometry bar](home/.config/dwlb-geometry/README.md) — bar config
- [wmenu-caos](home/.config/wmenu-caos/README.md) — launcher/picker config
- [kaprica](home/.config/kaprica/README.md) — clipboard history daemon
- [wclipmenu](home/.config/wclipmenu/README.md) — clipboard picker
- [KEYBINDS.md](KEYBINDS.md) — complete keyboard reference