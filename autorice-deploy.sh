#!/usr/bin/env bash
#
# autorice-deploy.sh
# EliteBook 645 G11 — dwl DraculaWL Rice Deployment
#
# Stack: dwl (sevz fork) + dwlb-geometry + wmenu-dwlb + foot + tmux + nvim
#        zsh + atuin + ccze + mako + zathura + cliphist + widle + wlock
#
# Assumes: Fresh Arch install, user account created, sudo working
# Usage:   ./autorice-deploy.sh
#

set -euo pipefail

# ============================================
# Configuration
# ============================================

DOTFILES_REPO="https://github.com/caos-obliquo/dotfiles"
DOTFILES_DIR="$HOME/dotfiles"
CONFIG_DIR="$HOME/.config"
LOCAL_BIN="$HOME/.local/bin"
SRC_DIR="$HOME/.local/src"
BUILDS_DIR="$HOME/builds"
WALLS_DIR="$HOME/walls"

DWL_REPO="https://codeberg.org/sevz/dwl.git"
DWLB_REPO="https://github.com/caos-obliquo/dwlb-geometry.git"
WMENU_REPO="git@github.com:caos-obliquo/wmenu-dwlb.git"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# ============================================
# Helpers
# ============================================

log() { echo -e "${BLUE}[$(date +'%H:%M:%S')]${NC} $*"; }
success() { echo -e "${GREEN}[✓]${NC} $*"; }
warn() { echo -e "${YELLOW}[!]${NC} $*"; }
error() {
    echo -e "${RED}[✗]${NC} $*"
    exit 1
}
prompt() { echo -e "${CYAN}[?]${NC} $*"; }

section() {
    echo ""
    echo -e "${CYAN}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC} $(printf '%-54s' "$*") ${CYAN}║${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

# ============================================
# System Checks
# ============================================

check_system() {
    section "System Checks"

    [ ! -f /etc/arch-release ] && error "Arch Linux only"
    success "Running Arch Linux"

    ping -c 1 archlinux.org &>/dev/null || error "No internet connection"
    success "Internet connection active"

    [ "$EUID" -eq 0 ] && error "Don't run as root"
    success "Running as user: $USER"

    sudo -v || error "Sudo privileges required"
    success "Sudo confirmed"
}

# ============================================
# Pacman
# ============================================

setup_pacman() {
    section "Configuring Pacman"

    sudo cp /etc/pacman.conf /etc/pacman.conf.backup

    sudo sed -i 's/^#ParallelDownloads/ParallelDownloads/' /etc/pacman.conf
    sudo sed -i 's/^#Color/Color/' /etc/pacman.conf
    grep -q "ILoveCandy" /etc/pacman.conf || sudo sed -i '/^Color/a ILoveCandy' /etc/pacman.conf

    sudo pacman -S --needed --noconfirm reflector
    sudo reflector --country Brazil --age 12 --protocol https --sort rate \
        --save /etc/pacman.d/mirrorlist

    sudo pacman -Syy
    success "Pacman configured"
}

# ============================================
# Base System
# ============================================

install_base_system() {
    section "Installing Base System Packages"

    sudo pacman -S --needed --noconfirm \
        base-devel git curl wget stow \
        man-db man-pages \
        zsh zsh-completions \
        zsh-syntax-highlighting zsh-autosuggestions \
        htop btop \
        neovim \
        ripgrep fd fzf \
        tree unzip zip rsync \
        tmux \
        lf \
        pass \
        atuin \
        ccze || error "Failed to install base packages"

    success "Base packages installed"
}

# ============================================
# AMD GPU
# ============================================

setup_amd_gpu() {
    section "Configuring AMD GPU (Radeon 660M)"

    sudo pacman -S --needed --noconfirm \
        amd-ucode \
        mesa lib32-mesa \
        vulkan-radeon lib32-vulkan-radeon \
        libva-mesa-driver lib32-libva-mesa-driver \
        mesa-vdpau lib32-mesa-vdpau \
        xf86-video-amdgpu || warn "AMD driver issues"

    if ! grep -q "^MODULES=.*amdgpu" /etc/mkinitcpio.conf; then
        sudo sed -i 's/^MODULES=(\(.*\))/MODULES=(\1 amdgpu)/' /etc/mkinitcpio.conf
        sudo mkinitcpio -P
        success "Early KMS enabled for amdgpu"
    else
        success "Early KMS already configured"
    fi

    success "AMD GPU configured"
}

# ============================================
# Wayland Stack
# ============================================

install_wayland_stack() {
    section "Installing Wayland Stack"

    sudo pacman -S --needed --noconfirm \
        wayland wayland-protocols \
        wlroots0.18 \
        libinput \
        libxkbcommon \
        xorg-xwayland \
        xcb-util-wm \
        pixman \
        pkgconf \
        fcft \
        meson ninja || error "Wayland stack failed"

    success "Wayland stack installed"
}

# ============================================
# Applications
# ============================================

install_applications() {
    section "Installing Applications"

    sudo pacman -S --needed --noconfirm \
        foot \
        wl-clipboard \
        cliphist \
        grim slurp \
        swaybg \
        mako \
        widle \
        pamixer \
        playerctl || error "Application install failed"

    sudo pacman -S --needed --noconfirm \
        ttf-jetbrains-mono-nerd \
        noto-fonts noto-fonts-emoji \
        ttf-font-awesome || warn "Font issues"

    sudo pacman -S --needed --noconfirm \
        firefox \
        mpv \
        imv \
        zathura zathura-pdf-mupdf || warn "Media app issues"

    sudo pacman -S --needed --noconfirm \
        ranger \
        thunar thunar-archive-plugin \
        file-roller || warn "File manager issues"

    success "Applications installed"
}

# ============================================
# XDG Portals
# ============================================

install_portals() {
    section "Installing XDG Portals"

    sudo pacman -S --needed --noconfirm \
        xdg-desktop-portal \
        xdg-desktop-portal-wlr \
        xdg-desktop-portal-gtk || warn "Portal issues"

    success "XDG Portals installed"
}

# ============================================
# Power Management
# ============================================

setup_power_management() {
    section "Configuring Power Management (TLP)"

    sudo pacman -S --needed --noconfirm tlp tlp-rdw
    sudo systemctl enable tlp.service
    sudo systemctl start tlp.service
    sudo systemctl mask systemd-rfkill.service
    sudo systemctl mask systemd-rfkill.socket

    success "TLP configured"
}

# ============================================
# Network
# ============================================

setup_network() {
    section "Configuring Network"

    sudo pacman -S --needed --noconfirm networkmanager
    sudo systemctl enable NetworkManager
    sudo systemctl start NetworkManager

    success "NetworkManager configured"
}

# ============================================
# Clone Dotfiles
# ============================================

clone_dotfiles() {
    section "Cloning Dotfiles"

    if [ -d "$DOTFILES_DIR" ]; then
        log "Dotfiles exist, pulling..."
        git -C "$DOTFILES_DIR" pull
    else
        git clone "$DOTFILES_REPO" "$DOTFILES_DIR"
    fi

    log "Copying home/ to ~/..."
    cp -r "$DOTFILES_DIR/home/." "$HOME/"
    chmod +x "$LOCAL_BIN"/*.sh 2>/dev/null || true

    success "Dotfiles deployed"
}

# ============================================
# Build dwl (sevz fork)
# ============================================

build_dwl() {
    section "Building dwl (sevz fork)"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "dwl" ]; then
        log "dwl exists, updating..."
        git -C dwl pull || true
    else
        git clone "$DWL_REPO" dwl
    fi

    cd dwl
    cp "$DOTFILES_DIR/builds/dwl/config.h" . 2>/dev/null ||
        {
            warn "No config.h in dotfiles, using config.def.h"
            cp config.def.h config.h
        }

    make clean
    make || error "dwl build failed"
    sudo make install

    success "dwl installed"
}

# ============================================
# Build dwlb-geometry
# ============================================

build_dwlb() {
    section "Building dwlb-geometry"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "dwlb-geometry" ]; then
        log "dwlb-geometry exists, updating..."
        git -C dwlb-geometry pull || true
    else
        git clone "$DWLB_REPO" dwlb-geometry
    fi

    cd dwlb-geometry
    cp "$DOTFILES_DIR/builds/dwlb-geometry/config.h" . 2>/dev/null ||
        {
            warn "No config.h in dotfiles, using config.def.h"
            cp config.def.h config.h
        }

    make clean
    make || error "dwlb-geometry build failed"
    sudo make install

    success "dwlb-geometry installed"
}

# ============================================
# Build wmenu-dwlb (meson)
# ============================================

build_wmenu() {
    section "Building wmenu-dwlb"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "wmenu-dwlb" ]; then
        log "wmenu-dwlb exists, updating..."
        git -C wmenu-dwlb pull || true
    else
        # SSH key required — falls back to https if not set up
        git clone "$WMENU_REPO" wmenu-dwlb 2>/dev/null ||
            git clone "https://github.com/caos-obliquo/wmenu-dwlb.git" wmenu-dwlb
    fi

    cd wmenu-dwlb
    cp "$DOTFILES_DIR/builds/wmenu-dwlb/config.h" . 2>/dev/null || warn "No config.h for wmenu-dwlb"
    cp "$DOTFILES_DIR/builds/wmenu-dwlb/menu.c" . 2>/dev/null || warn "No menu.c for wmenu-dwlb"

    rm -rf build
    meson setup build
    ninja -C build
    sudo ninja -C build install

    success "wmenu-dwlb installed"
}

# ============================================
# zsh plugins (not in pacman)
# ============================================

setup_zsh_plugins() {
    section "Setting Up zsh Plugins"

    # zsh-vi-mode
    if [ ! -d "$HOME/.zsh-vi-mode" ]; then
        git clone https://github.com/jeffreytse/zsh-vi-mode "$HOME/.zsh-vi-mode"
    else
        git -C "$HOME/.zsh-vi-mode" pull
    fi

    # zsh-z
    mkdir -p "$HOME/.local/share/zsh/plugins"
    if [ ! -d "$HOME/.local/share/zsh/plugins/zsh-z" ]; then
        git clone https://github.com/agkozak/zsh-z \
            "$HOME/.local/share/zsh/plugins/zsh-z"
    else
        git -C "$HOME/.local/share/zsh/plugins/zsh-z" pull
    fi

    # Change shell
    if [ "$SHELL" != "$(which zsh)" ]; then
        chsh -s "$(which zsh)"
        success "Shell changed to zsh"
    else
        success "Already using zsh"
    fi

    success "zsh plugins ready"
}

# ============================================
# tmux plugins (TPM)
# ============================================

setup_tmux_plugins() {
    section "Setting Up tmux Plugins (TPM)"

    mkdir -p "$HOME/.local/share/tmux/plugins"
    if [ ! -d "$HOME/.local/share/tmux/plugins/tpm" ]; then
        git clone https://github.com/tmux-plugins/tpm \
            "$HOME/.local/share/tmux/plugins/tpm"
    else
        git -C "$HOME/.local/share/tmux/plugins/tpm" pull
    fi

    success "TPM installed — run prefix+I inside tmux to install plugins"
}

# ============================================
# Walls dir placeholder
# ============================================

setup_walls() {
    section "Setting Up Wallpapers Directory"

    mkdir -p "$WALLS_DIR"
    warn "Place your wallpapers in $WALLS_DIR — start-dwl.sh expects wall3.jpg"

    success "Walls dir ready"
}

# ============================================
# atuin
# ============================================

setup_atuin() {
    section "Configuring atuin"

    # atuin is already in pacman, just init the db
    atuin gen-completions --shell zsh >/dev/null 2>&1 || true
    success "atuin ready"
}

# ============================================
# Setup Neovim (caos.nvim)
# ============================================

setup_neovim() {
    section "Setting Up Neovim (caos.nvim)"

    if [ -d "$CONFIG_DIR/nvim" ]; then
        log "nvim config exists, pulling..."
        git -C "$CONFIG_DIR/nvim" pull
    else
        git clone git@github.com:caos-obliquo/nvim.git "$CONFIG_DIR/nvim" 2>/dev/null ||
            git clone https://github.com/caos-obliquo/nvim.git "$CONFIG_DIR/nvim"
    fi

    log "Configuring npm global prefix..."
    mkdir -p "$HOME/.npm-global"
    npm config set prefix "$HOME/.npm-global"
    npm install -g neovim || warn "npm neovim provider failed"

    log "Installing pynvim..."
    pip install pynvim --break-system-packages || warn "pynvim install failed"

    log "Installing system linter dependencies..."
    sudo pacman -S --needed --noconfirm \
        luarocks \
        luacheck \
        cppcheck \
        nodejs \
        npm || warn "Some nvim system deps failed"

    log "Installing Java (jdtls dependency)..."
    sudo pacman -S --needed --noconfirm jdk21-openjdk || warn "Java install failed"
    sudo archlinux-java set java-21-openjdk 2>/dev/null || warn "Could not set java-21-openjdk"

    if command -v yay &>/dev/null; then
        yay -S --needed --noconfirm pmd || warn "pmd AUR install failed"
    else
        warn "pmd not installed — no AUR helper. Install manually or via yay."
    fi

    success "Neovim deployed — open nvim, run :Lazy sync then :Mason"
    warn "Mason packages: :MasonInstall yamllint jsonlint hadolint tflint shellcheck eslint_d htmlhint stylelint ruff cpplint golangci-lint markdownlint luacheck shfmt prettier stylua google-java-format goimports"
}

# ============================================
# Summary
# ============================================

print_summary() {
    section "Installation Complete!"

    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║        EliteBook 645 G11 — DraculaWL Ready            ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${CYAN}Next steps:${NC}"
    echo -e "  ${BLUE}1.${NC} Add wallpaper to ${YELLOW}~/walls/wall3.jpg${NC}"
    echo -e "  ${BLUE}2.${NC} Reboot: ${YELLOW}sudo reboot${NC}"
    echo -e "  ${BLUE}3.${NC} Login to TTY1 — dwl starts via .zprofile"
    echo -e "  ${BLUE}4.${NC} Inside tmux: ${YELLOW}prefix+I${NC} to install plugins"
    echo ""
    echo -e "${CYAN}Key bindings:${NC}"
    echo -e "  ${BLUE}Super+Return${NC}    Terminal (foot)"
    echo -e "  ${BLUE}Super+D${NC}         App launcher (wmenu in bar)"
    echo -e "  ${BLUE}Super+P${NC}         Clipboard history"
    echo -e "  ${BLUE}Super+S${NC}         Screenshot area"
    echo -e "  ${BLUE}Ctrl+Up${NC}         Atuin history search"
    echo -e "  ${BLUE}Super+Shift+Q${NC}   Quit dwl"
    echo ""
}

# ============================================
# Main
# ============================================

main() {
    clear
    echo -e "${CYAN}"
    cat <<"BANNER"
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║     EliteBook 645 G11 — dwl DraculaWL Autorice           ║
║     sevz/dwl · dwlb-geometry · wmenu-dwlb                ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
BANNER
    echo -e "${NC}"

    prompt "Deploy DraculaWL rice? [y/N] "
    read -r response
    [[ ! "$response" =~ ^[Yy]$ ]] && error "Cancelled"

    check_system
    setup_pacman
    install_base_system
    setup_amd_gpu
    install_wayland_stack
    install_applications
    install_portals
    setup_power_management
    setup_network
    clone_dotfiles
    build_dwl
    build_dwlb
    build_wmenu
    setup_zsh_plugins
    setup_tmux_plugins
    setup_walls
    setup_atuin
    setup_neovim
    print_summary
}

main "$@"
