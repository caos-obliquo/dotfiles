#!/usr/bin/env bash
#
# autorice-deploy-vm.sh
# VM edition — dwl DraculaWL Rice Deployment
#
# Skipped vs bare-metal:
#   - AMD GPU drivers + early KMS  (no amdgpu in VM)
#   - TLP power management         (no battery)
#   - Gaming/Vulkan env vars        (irrelevant)
#
# Added for VM:
#   - spice-vdagent                 (clipboard + resolution sync)
#   - WLR_RENDERER=pixman           (software rendering fallback)
#   - WLR_NO_HARDWARE_CURSORS=1     (prevents cursor crash)
#   - Virtual-1 monitor in dwl config (not eDP-1)
#
# Usage: ./autorice-deploy-vm.sh
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
WMENU_REPO="https://github.com/caos-obliquo/wmenu-dwlb.git"

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

    VIRT=$(systemd-detect-virt 2>/dev/null || echo "unknown")
    success "VM detected: $VIRT"
}

# ============================================
# Pacman
# ============================================

setup_pacman() {
    section "Configuring Pacman"

    sudo cp /etc/pacman.conf /etc/pacman.conf.backup

    # Remove deprecated repos that break extra sync (removed March 2025)
    for repo in community community-testing testing testing-debug staging staging-debug; do
        sudo sed -i "/^\[$repo\]/,/^$/d" /etc/pacman.conf
    done

    # Ensure [extra] is present and uncommented
    if grep -q '^#\[extra\]' /etc/pacman.conf; then
        sudo sed -i 's/^#\[extra\]/[extra]/' /etc/pacman.conf
        sudo sed -i '/^\[extra\]/{n;s/^#Include/Include/}' /etc/pacman.conf
        log "[extra] repository uncommented"
    elif ! grep -q '^\[extra\]' /etc/pacman.conf; then
        printf '\n[extra]\nInclude = /etc/pacman.d/mirrorlist\n' | sudo tee -a /etc/pacman.conf >/dev/null
        log "[extra] repository appended"
    fi

    sudo sed -i 's/^#ParallelDownloads/ParallelDownloads/' /etc/pacman.conf
    sudo sed -i 's/^#Color/Color/' /etc/pacman.conf
    grep -q "ILoveCandy" /etc/pacman.conf || sudo sed -i '/^Color/a ILoveCandy' /etc/pacman.conf

    # Update mirrors
    sudo pacman -S --needed --noconfirm reflector
    sudo reflector --country Brazil --age 12 --protocol https --sort rate \
        --save /etc/pacman.d/mirrorlist

    # Full sync + upgrade to avoid partial upgrade issues
    sudo pacman -Syu --noconfirm

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
# VM Guest Utilities
# ============================================

setup_vm_guest() {
    section "Installing VM Guest Utilities"

    sudo pacman -S --needed --noconfirm \
        spice-vdagent \
        xf86-video-qxl \
        mesa \
        libva-mesa-driver || warn "Some VM guest packages had issues"

    sudo systemctl enable spice-vdagentd.service
    sudo systemctl start spice-vdagentd.service 2>/dev/null || true

    success "VM guest utilities configured"
}

# ============================================
# Wayland Stack
# ============================================

install_wayland_stack() {
    section "Installing Wayland Stack"

    # Ensure pacman db is fresh before checking wlroots0.18
    sudo pacman -Sy --noconfirm

    # wlroots0.18 is in [extra] — install it with all required deps explicitly
    # deps: seatd, libdisplay-info, libliftoff, vulkan-icd-loader, lcms2
    if sudo pacman -S --needed --noconfirm \
        wayland wayland-protocols \
        wlroots0.18 \
        seatd \
        libdisplay-info \
        libliftoff \
        vulkan-icd-loader \
        lcms2 \
        libinput \
        libxkbcommon \
        xorg-xwayland \
        xcb-util-wm \
        xcb-util-errors \
        xcb-util-renderutil \
        pixman \
        pkgconf \
        fcft \
        glslang \
        vulkan-headers \
        meson ninja; then
        success "Wayland stack installed from [extra]"
    else
        warn "wlroots0.18 from pacman failed — building from source (v0.18.3)"
        build_wlroots_from_source
    fi
}

# ============================================
# Build wlroots from source (fallback)
# ============================================

build_wlroots_from_source() {
    section "Building wlroots 0.18 from source"

    # Install all build deps first
    sudo pacman -S --needed --noconfirm \
        wayland wayland-protocols \
        seatd libdisplay-info libliftoff \
        vulkan-icd-loader vulkan-headers \
        lcms2 libinput libxkbcommon \
        xorg-xwayland xcb-util-wm \
        xcb-util-errors xcb-util-renderutil \
        pixman pkgconf glslang \
        meson ninja git || error "wlroots build deps failed"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "wlroots" ]; then
        git -C wlroots fetch --all
    else
        git clone https://gitlab.freedesktop.org/wlroots/wlroots.git
    fi

    cd wlroots
    git checkout 0.18.3 2>/dev/null || git checkout $(git tag | grep "^0\.18" | sort -V | tail -1)

    rm -rf build
    meson setup build --buildtype=release
    ninja -C build
    sudo ninja -C build install
    sudo ldconfig

    success "wlroots 0.18 built and installed from source"
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
# Build dwl (sevz fork) — patched for Virtual-1
# ============================================

build_dwl() {
    section "Building dwl (sevz fork — VM/Virtual-1)"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "dwl" ]; then
        git -C dwl pull || true
    else
        git clone "$DWL_REPO" dwl
    fi

    cd dwl

    if [ -f "$DOTFILES_DIR/builds/dwl/config.h" ]; then
        cp "$DOTFILES_DIR/builds/dwl/config.h" .
        sed -i 's/"eDP-1"/"Virtual-1"/g' config.h
        success "config.h from dotfiles, patched for Virtual-1"
    else
        warn "No config.h in dotfiles, using config.def.h"
        cp config.def.h config.h
        sed -i 's/"eDP-1"/"Virtual-1"/g' config.h
    fi

    make clean
    make || error "dwl build failed"
    sudo make install

    success "dwl installed (Virtual-1)"
}

# ============================================
# Build dwlb-geometry
# ============================================

build_dwlb() {
    section "Building dwlb-geometry"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "dwlb-geometry" ]; then
        git -C dwlb-geometry pull || true
    else
        git clone "$DWLB_REPO" dwlb-geometry
    fi

    cd dwlb-geometry
    cp "$DOTFILES_DIR/builds/dwlb-geometry/config.h" . 2>/dev/null ||
        {
            warn "No config.h for dwlb, using default"
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
        git -C wmenu-dwlb pull || true
    else
        git clone "$WMENU_REPO" wmenu-dwlb
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
# Build widle (Codeberg — sewn/widle)
# ============================================

build_widle() {
    section "Building widle"

    mkdir -p "$BUILDS_DIR"
    cd "$BUILDS_DIR"

    if [ -d "widle" ]; then
        git -C widle pull || true
    else
        git clone https://codeberg.org/sewn/widle.git
    fi

    cd widle

    if [ -f meson.build ]; then
        rm -rf build
        meson setup build
        ninja -C build
        sudo ninja -C build install
    else
        make || error "widle build failed"
        sudo make install
    fi

    success "widle installed"
}

# ============================================
# Patch start-dwl.sh for VM
# ============================================

patch_startup_for_vm() {
    section "Patching start-dwl.sh for VM"

    STARTUP="$LOCAL_BIN/start-dwl.sh"

    if [ ! -f "$STARTUP" ]; then
        warn "start-dwl.sh not found at $LOCAL_BIN — skipping patch"
        return
    fi

    if ! grep -q "WLR_RENDERER" "$STARTUP"; then
        sed -i '/^export XDG_SESSION_TYPE/a export WLR_RENDERER=pixman\nexport WLR_NO_HARDWARE_CURSORS=1' "$STARTUP"
        success "WLR_RENDERER=pixman injected into start-dwl.sh"
    else
        success "VM env vars already present in start-dwl.sh"
    fi
}

# ============================================
# zsh plugins
# ============================================

setup_zsh_plugins() {
    section "Setting Up zsh Plugins"

    if [ ! -d "$HOME/.zsh-vi-mode" ]; then
        git clone https://github.com/jeffreytse/zsh-vi-mode "$HOME/.zsh-vi-mode"
    else
        git -C "$HOME/.zsh-vi-mode" pull
    fi

    mkdir -p "$HOME/.local/share/zsh/plugins"
    if [ ! -d "$HOME/.local/share/zsh/plugins/zsh-z" ]; then
        git clone https://github.com/agkozak/zsh-z \
            "$HOME/.local/share/zsh/plugins/zsh-z"
    else
        git -C "$HOME/.local/share/zsh/plugins/zsh-z" pull
    fi

    if [ "$SHELL" != "$(which zsh)" ]; then
        chsh -s "$(which zsh)"
        success "Shell changed to zsh"
    else
        success "Already using zsh"
    fi

    success "zsh plugins ready"
}

# ============================================
# tmux plugins
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
# Walls
# ============================================

setup_walls() {
    section "Setting Up Wallpapers Directory"
    mkdir -p "$WALLS_DIR"
    warn "Place wallpapers in $WALLS_DIR — start-dwl.sh expects wall3.jpg"
    success "Walls dir ready"
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
        git clone https://github.com/caos-obliquo/nvim.git "$CONFIG_DIR/nvim"
    fi

    log "Installing system linter dependencies..."
    sudo pacman -S --needed --noconfirm \
        nodejs npm \
        luarocks luacheck \
        cppcheck \
        jdk21-openjdk || warn "Some nvim system deps failed"

    sudo archlinux-java set java-21-openjdk 2>/dev/null || warn "Could not set java-21-openjdk"

    log "Configuring npm global prefix..."
    mkdir -p "$HOME/.npm-global"
    npm config set prefix "$HOME/.npm-global"
    npm install -g neovim 2>/dev/null || warn "npm neovim provider failed"

    log "Installing pynvim..."
    pip install pynvim --break-system-packages 2>/dev/null || warn "pynvim install failed"

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
    echo -e "${GREEN}║         DraculaWL — VM Edition Ready                  ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${CYAN}VM-specific changes applied:${NC}"
    echo -e "  ${YELLOW}•${NC} AMD GPU drivers skipped"
    echo -e "  ${YELLOW}•${NC} TLP skipped"
    echo -e "  ${YELLOW}•${NC} spice-vdagent installed (clipboard + resolution)"
    echo -e "  ${YELLOW}•${NC} dwl monitor rule patched to Virtual-1"
    echo -e "  ${YELLOW}•${NC} WLR_RENDERER=pixman + WLR_NO_HARDWARE_CURSORS=1"
    echo -e "  ${YELLOW}•${NC} wlroots0.18 with all deps explicit"
    echo ""
    echo -e "${CYAN}Next steps:${NC}"
    echo -e "  ${BLUE}1.${NC} Add wallpaper: ${YELLOW}~/walls/wall3.jpg${NC}"
    echo -e "  ${BLUE}2.${NC} Reboot: ${YELLOW}sudo reboot${NC}"
    echo -e "  ${BLUE}3.${NC} Login to TTY1 — dwl starts via .zprofile"
    echo -e "  ${BLUE}4.${NC} Inside tmux: ${YELLOW}prefix+I${NC} to install plugins"
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
║     dwl DraculaWL Autorice — VM Edition                  ║
║     sevz/dwl · dwlb-geometry · wmenu-dwlb                ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
BANNER
    echo -e "${NC}"

    prompt "Deploy DraculaWL rice (VM edition)? [y/N] "
    read -r response
    [[ ! "$response" =~ ^[Yy]$ ]] && error "Cancelled"

    check_system
    setup_pacman
    install_base_system
    setup_vm_guest
    install_wayland_stack
    install_applications
    install_portals
    setup_network
    clone_dotfiles
    build_dwl
    build_dwlb
    build_wmenu
    patch_startup_for_vm
    build_widle
    setup_zsh_plugins
    setup_tmux_plugins
    setup_walls
    setup_neovim
    print_summary
}

main "$@"
