# dotfiles

Monochrome Hyprland rice on Arch Linux.

## What's included

hyprland | waybar | wofi | kitty | dunst | nvim | cava | thunar

## Installation

**1. Install dependencies**
sudo pacman -S hyprland waybar wofi kitty dunst neovim cava thunar stow

**2. Clone repository**
git clone https://github.com/caos-obliquo/dotfiles.git ~/dotfiles

**3. Backup your configs (optional)**
cp -r ~/.config/{hypr,waybar,kitty,wofi,dunst,nvim} ~/.config_backup/

**4. Deploy with stow**
cd ~/dotfiles/stow_home
stow */

**5. Reload Hyprland**

Press `SUPER + SHIFT + R` or relogin.

## Keybindings

**Applications**
- `SUPER + RETURN` - Terminal
- `SUPER + D` - Launcher
- `SUPER + X` - Firefox
- `SUPER + Z` - Zen Browser
- `SUPER + E` - File Manager
- `SUPER + L` - Lock

**Window Management**
- `SUPER + C` - Kill window
- `SUPER + SPACE` - Toggle floating
- `SUPER + Arrows` - Move focus
- `SUPER + 1-9` - Workspaces

**System**
- `Print` - Screenshot
- Media keys for volume/brightness

## Notes

Managed with GNU Stow. Edit files in `~/dotfiles/stow_home/`.
