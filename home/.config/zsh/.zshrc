# ============================================
# ZSH CONFIG - zsh-vi-mode plugin
# ============================================

# ZSH-VI-MODE PLUGIN
source ~/.zsh-vi-mode/zsh-vi-mode.plugin.zsh

# Configure cursors
ZVM_INSERT_MODE_CURSOR=$ZVM_CURSOR_BEAM
ZVM_NORMAL_MODE_CURSOR=$ZVM_CURSOR_BLOCK
ZVM_VISUAL_MODE_CURSOR=$ZVM_CURSOR_UNDERLINE
ZVM_VISUAL_LINE_MODE_CURSOR=$ZVM_CURSOR_UNDERLINE

# Enable colors
autoload -U colors && colors

# Dracula-themed prompt (two-line)
# Line 1: user@host path$
# Line 2: >
PS1=$'%B%F{magenta}%n%f%F{white}@%f%F{blue}%M %F{magenta}%~%f%b%F{white}$%f\n%F{212}>%f '

# History
HISTSIZE=10000000
SAVEHIST=10000000
HISTFILE="${XDG_CACHE_HOME:-$HOME/.cache}/zsh/history"

# Load external configs
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/shell/shortcutrc" ] && source "${XDG_CONFIG_HOME:-$HOME/.config}/shell/shortcutrc"
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/shell/aliasrc" ] && source "${XDG_CONFIG_HOME:-$HOME/.config}/shell/aliasrc"
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/shell/zshnameddirrc" ] && source "${XDG_CONFIG_HOME:-$HOME/.config}/shell/zshnameddirrc"

# Completion
autoload -U compinit
zstyle ':completion:*' menu select
zstyle ':completion:*' list-colors ${(s.:.)LS_COLORS}
zmodload zsh/complist
compinit
_comp_options+=(globdots)

# Menu select with vim keys
bindkey -M menuselect 'h' vi-backward-char
bindkey -M menuselect 'k' vi-up-line-or-history
bindkey -M menuselect 'l' vi-forward-char
bindkey -M menuselect 'j' vi-down-line-or-history

# LF FILE MANAGER (Ctrl+O)
lfcd () {
    tmp="$(mktemp -uq)"
    trap 'rm -f $tmp >/dev/null 2>&1 && trap - HUP INT QUIT TERM PWR EXIT' HUP INT QUIT TERM PWR EXIT
    lf -last-dir-path="$tmp" "$@"
    if [ -f "$tmp" ]; then
        dir="$(cat "$tmp")"
        [ -d "$dir" ] && [ "$dir" != "$(pwd)" ] && cd "$dir"
    fi
}
bindkey -s '^o' '^ulfcd\n'

# EDIT IN VIM (Ctrl+E)
autoload edit-command-line; zle -N edit-command-line
bindkey '^e' edit-command-line

# =============================================================================
# ENVIRONMENT & PATH
# =============================================================================

export PATH="$HOME/.local/bin:$PATH"
export PATH="$HOME/.cargo/bin:$PATH"
export PATH=~/.npm-global/bin:$PATH

# Brazilian keyboard
export XKB_DEFAULT_LAYOUT=br
export XKB_DEFAULT_VARIANT=abnt2

# =============================================================================
# COLORED OUTPUT (Native Tools - No Replacements)
# =============================================================================

# Remove old aliases
unalias journalctl 2>/dev/null
unalias dmesg 2>/dev/null

# ccze Dracula functions
journalctl() {
    command journalctl --no-pager "$@" | ccze -A
}

# dmesg + ccze colors
dmesg() {
    dmesg "$@" | ccze -A
}

# Native colors for others
alias ls='ls --color=auto'
alias grep='grep --color=auto'
alias diff='diff --color=auto'
alias ip='ip -color=auto'

# Use less with native color support (less 551+)
export MANPAGER="less -R --use-color -Dd+208 -Du+c -Ds+y"
export MANROFFOPT="-c"

# Color mapping:
# -Dd+o = bold (headers like NAME, OVERVIEW) -> ORANGE (was red)
# -Du+c = underline (links like zsh, zshroadmap) -> CYAN
# -Ds+y = standout (search/alert) -> YELLOW
#
# Add TERM for pacman colored output
export TERM=xterm-256color

# Enable colors for systemctl/journalctl (red=error, white=notice, grey=debug)
export SYSTEMD_COLORS=true

# Long listing with colors
alias ll='ls -la'

# Editor aliases
alias vim='nvim'
alias nano='nvim'

# =============================================================================
# PLUGINS
# =============================================================================

# Syntax highlighting (green=valid, red=invalid, yellow=strings, etc.)
if [ -f /usr/share/zsh/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh ]; then
    source /usr/share/zsh/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh
    # Dracula colors for syntax highlighting
    ZSH_HIGHLIGHT_STYLES[command]='fg=#50fa7b'        # Green
    ZSH_HIGHLIGHT_STYLES[alias]='fg=#50fa7b'          # Green
    ZSH_HIGHLIGHT_STYLES[builtin]='fg=#8be9fd'        # Cyan
    ZSH_HIGHLIGHT_STYLES[function]='fg=#ffb86c'       # Orange
    ZSH_HIGHLIGHT_STYLES[path]='fg=#8be9fd'           # Cyan
    ZSH_HIGHLIGHT_STYLES[string]='fg=#f1fa8c'         # Yellow
    ZSH_HIGHLIGHT_STYLES[unknown-token]='fg=#ff5555'  # Red
fi

# Autosuggestions (grey)
if [ -f /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh ]; then
    source /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh
    ZSH_AUTOSUGGEST_HIGHLIGHT_STYLE='fg=#6272a4'      # Dracula comment color
fi

# ZSH-Z (jump to frequent directories)
if [ -f ~/.local/share/zsh/plugins/zsh-z/zsh-z.plugin.zsh ]; then
    source ~/.local/share/zsh/plugins/zsh-z/zsh-z.plugin.zsh
    ZSHZ_DATA="${XDG_CACHE_HOME:-$HOME/.cache}/zsh/z-data"
fi

# Syntax-highlighting cat using highlight
cat() {
    if command -v highlight >/dev/null 2>&1; then
        highlight -O ansi "$@" 2>/dev/null || command cat "$@"
    else
        command cat "$@"
    fi
}

# =============================================================================
# ALIASES
# =============================================================================

alias update="source ~/.config/zsh/.zshrc"
