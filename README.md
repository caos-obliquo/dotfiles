# dotfiles

Monochrome Hyprland rice on Arch Linux.

## What's included

- **hyprland** - Window manager
- **waybar** - Status bar
- **wofi** - Application launcher
- **kitty** - Terminal emulator
- **dunst** - Notification daemon
- **nvim** - Text editor
- **cava** - Audio visualizer
- **thunar** - File manager

## Installation

### 1. Install dependencies
```bash
sudo pacman -S hyprland waybar wofi kitty dunst neovim cava thunar stow
2. Clone repository
git clone https://github.com/caos-obliquo/dotfiles.git ~/dotfiles
3. Backup your current configs (optional)
mkdir -p ~/.config_backup
cp -r ~/.config/{hypr,waybar,kitty,wofi,dunst,nvim} ~/.config_backup/
4. Deploy configs with stow
cd ~/dotfiles/stow_home
stow */
This creates symlinks from ~/dotfiles/stow_home/* to your ~/.config/.
5. Reload Hyprland
Press SUPER + SHIFT + R or logout and login again.
Keybindings
Applications

SUPER + RETURN - Terminal (kitty)
SUPER + D - Launcher (wofi)
SUPER + X - Firefox
SUPER + Z - Zen Browser
SUPER + S - Spotify
SUPER + E - File Manager
SUPER + L - Lock screen
Wofi Scripts

SUPER + F - Repository selector
SUPER + B - Bookmarks
SUPER + T - Sessions
Window Management

SUPER + C - Kill active window
SUPER + M - Exit Hyprland
SUPER + SPACE - Toggle floating
SUPER + SHIFT + F - Fullscreen
SUPER + Arrows - Move focus
SUPER + SHIFT + Arrows - Move window
SUPER + 1-9 - Switch workspace
SUPER + SHIFT + 1-9 - Move to workspace
SUPER + TAB - Toggle special workspace
SUPER + Mouse Left - Move window
SUPER + Mouse Right - Resize window
Updating
cd ~/dotfiles
git pull
cd stow_home
stow -R */
Uninstall
cd ~/dotfiles/stow_home
stow -D */
Notes

Configs are managed with GNU Stow
Edit files in ~/dotfiles/stow_home/
Changes reflect immediately via symlinks
Hyprland config uses ABNT2 keyboard layout
