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

# Dracula prompt: user@host path$ / >
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

export XKB_DEFAULT_LAYOUT=br
export XKB_DEFAULT_VARIANT=abnt2

# =============================================================================
# CCZE - DRACULA SEMANTIC SCHEME
# =============================================================================
# Tier 1 SIGNAL    — error/warning: must scream
# Tier 2 STRUCTURE — timestamp/host/pid: recede, always repeated
# Tier 3 DATA      — process/ip/path: readable, meaningful

unalias journalctl 2>/dev/null
unalias dmesg 2>/dev/null

ccze_dracula() {
    ccze -F ~/.config/ccze/cczerc -A -o nolookups "$@"
}

# SYSTEM LOGS
journalctl() { command journalctl --no-pager "$@" | ccze_dracula -A; }
dmesg()      { command dmesg "$@" | ccze_dracula -A; }

# SYSTEMD
systemctl()  { command systemctl --no-pager "$@" | ccze_dracula -A; }

# NETWORKING
ping()       { command ping "$@" 2>&1 | ccze_dracula -A; }
traceroute() { command traceroute "$@" 2>&1 | ccze_dracula -A; }
netstat()    { command netstat "$@" 2>&1 | ccze_dracula -A; }

# PROCESSES
ps()         { command ps "$@" | ccze_dracula -A; }

# USERS & PERMISSIONS
who()        { command who "$@" | ccze_dracula -A; }
w()          { command w "$@" | ccze_dracula -A; }
last()       { command last "$@" | ccze_dracula -A; }

# DISK
df()         { command df "$@" | ccze_dracula -A; }
du()         { command du "$@" | ccze_dracula -A; }
mount()      { command mount "$@" | ccze_dracula -A; }

# ENVIRONMENT
env()        { command env "$@" | ccze_dracula -A; }
printenv()   { command printenv "$@" | ccze_dracula -A; }

# DOCKER / K8S
docker()         { command docker "$@" 2>&1 | ccze_dracula -A; }
docker-compose() { command docker-compose "$@" 2>&1 | ccze_dracula -A; }
kubectl()        { command kubectl "$@" 2>&1 | ccze_dracula -A; }
helm()           { command helm "$@" 2>&1 | ccze_dracula -A; }

# FILE BROWSING
tree()       { command tree "$@" | ccze_dracula -A; }
find()       { command find "$@" | ccze_dracula -A; }

uptime()     { command uptime "$@" | ccze_dracula -A; }
# date() and cat() intentionally NOT wrapped

# =============================================================================
# NATIVE COLORS
# =============================================================================

alias ls='ls --color=auto'
alias ll='ls -la'
alias grep='grep --color=auto'
alias diff='diff --color=auto'
alias ip='ip -color=auto'

alias vim='nvim'
alias nano='nvim'

# =============================================================================
# DRACULA MAN PAGES
# =============================================================================

export MANPAGER="/usr/bin/less -s -M +Gg"
export LESS_TERMCAP_mb=$'\e[1;31m'
export LESS_TERMCAP_md=$'\e[1;34m'
export LESS_TERMCAP_so=$'\e[01;45;37m'
export LESS_TERMCAP_us=$'\e[01;36m'
export LESS_TERMCAP_me=$'\e[0m'
export LESS_TERMCAP_se=$'\e[0m'
export LESS_TERMCAP_ue=$'\e[0m'
export GROFF_NO_SGR=1

export TERM=xterm-256color
export SYSTEMD_COLORS=true

# =============================================================================
# PLUGINS
# =============================================================================

# Syntax highlighting
# green=valid, cyan=builtin/path, orange=function, yellow=string, red=unknown
if [ -f /usr/share/zsh/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh ]; then
    source /usr/share/zsh/plugins/zsh-syntax-highlighting/zsh-syntax-highlighting.zsh
    ZSH_HIGHLIGHT_STYLES[command]='fg=#50fa7b'
    ZSH_HIGHLIGHT_STYLES[alias]='fg=#50fa7b'
    ZSH_HIGHLIGHT_STYLES[builtin]='fg=#8be9fd'
    ZSH_HIGHLIGHT_STYLES[function]='fg=#ffb86c'
    ZSH_HIGHLIGHT_STYLES[path]='fg=#8be9fd'
    ZSH_HIGHLIGHT_STYLES[string]='fg=#f1fa8c'
    ZSH_HIGHLIGHT_STYLES[unknown-token]='fg=#ff5555'
fi

# Autosuggestions — comment gray = ghost text, clearly not yet typed
if [ -f /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh ]; then
    source /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh
    ZSH_AUTOSUGGEST_HIGHLIGHT_STYLE='fg=#6272a4'
fi

# ZSH-Z
if [ -f ~/.local/share/zsh/plugins/zsh-z/zsh-z.plugin.zsh ]; then
    source ~/.local/share/zsh/plugins/zsh-z/zsh-z.plugin.zsh
    ZSHZ_DATA="${XDG_CACHE_HOME:-$HOME/.cache}/zsh/z-data"
fi

# =============================================================================
# ALIASES
# =============================================================================

alias update="source ~/.config/zsh/.zshrc"
