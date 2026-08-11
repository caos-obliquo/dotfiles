# zsh config - zsh-vi-mode plugin

# zsh-vi-mode plugin
source ~/.zsh-vi-mode/zsh-vi-mode.plugin.zsh

# configure cursors
ZVM_INSERT_MODE_CURSOR=$ZVM_CURSOR_BEAM
ZVM_NORMAL_MODE_CURSOR=$ZVM_CURSOR_BLOCK
ZVM_VISUAL_MODE_CURSOR=$ZVM_CURSOR_UNDERLINE
ZVM_VISUAL_LINE_MODE_CURSOR=$ZVM_CURSOR_UNDERLINE

# enable colors
autoload -U colors && colors

# dracula prompt: user@host path$ / > (❯)
PS1=$'%B%F{magenta}%n%f%F{white}@%f%F{blue}%M %F{magenta}%~%f%b%F{white}$%f\n%B%F{212}❯%f%b '

# history
HISTSIZE=10000000
SAVEHIST=10000000
HISTFILE="${XDG_CACHE_HOME:-$HOME/.cache}/zsh/history"

# load external configs
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/shell/shortcutrc" ] && source "${XDG_CONFIG_HOME:-$HOME/.config}/shell/shortcutrc"
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/shell/aliasrc" ] && source "${XDG_CONFIG_HOME:-$HOME/.config}/shell/aliasrc"
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/shell/zshnameddirrc" ] && source "${XDG_CONFIG_HOME:-$HOME/.config}/shell/zshnameddirrc"

# completion
autoload -U compinit
zstyle ':completion:*' menu select
zstyle ':completion:*' list-colors ${(s.:.)LS_COLORS}
zmodload zsh/complist
compinit
_comp_options+=(globdots)

# menu select with vim keys
bindkey -M menuselect 'h' vi-backward-char
bindkey -M menuselect 'k' vi-up-line-or-history
bindkey -M menuselect 'l' vi-forward-char
bindkey -M menuselect 'j' vi-down-line-or-history

# lf file manager (ctrl+o)
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

# edit command line in vim (ctrl+e)
autoload edit-command-line; zle -N edit-command-line
bindkey '^e' edit-command-line

# environment & path

export PATH="$HOME/.local/bin:$PATH"
export PATH="$HOME/.cargo/bin:$PATH"
export PATH=~/.npm-global/bin:$PATH

export XKB_DEFAULT_LAYOUT=br
export XKB_DEFAULT_VARIANT=abnt2

# ccze - dracula semantic scheme
# tier 1 signal - errors and warnings
# tier 2 structure - repeated tokens (timestamp, host, pid)
# tier 3 data - process names, ips, paths

unalias journalctl 2>/dev/null
unalias dmesg 2>/dev/null

ccze_dracula() {
    ccze -F ~/.config/ccze/cczerc -A -o nolookups "$@"
}

# system logs
journalctl() { command journalctl --no-pager "$@" | ccze_dracula -A; }
dmesg()      { command dmesg "$@" | ccze_dracula -A; }

# systemd
systemctl()  { command systemctl --no-pager "$@" | ccze_dracula -A; }

# networking
ping()       { command ping "$@" 2>&1 | ccze_dracula -A; }
traceroute() { command traceroute "$@" 2>&1 | ccze_dracula -A; }
netstat()    { command netstat "$@" 2>&1 | ccze_dracula -A; }

# processes
ps()         { command ps "$@" | ccze_dracula -A; }

# users and permissions
who()        { command who "$@" | ccze_dracula -A; }
w()          { command w "$@" | ccze_dracula -A; }
last()       { command last "$@" | ccze_dracula -A; }

# disk
df()         { command df "$@" | ccze_dracula -A; }
du()         { command du "$@" | ccze_dracula -A; }
mount()      { command mount "$@" | ccze_dracula -A; }

# environment
env()        { command env "$@" | ccze_dracula -A; }
printenv()   { command printenv "$@" | ccze_dracula -A; }

# docker / k8s
docker()         { command docker "$@" 2>&1 | ccze_dracula -A; }
docker-compose() { command docker-compose "$@" 2>&1 | ccze_dracula -A; }
kubectl()        { command kubectl "$@" 2>&1 | ccze_dracula -A; }
helm()           { command helm "$@" 2>&1 | ccze_dracula -A; }

# file browsing
tree()       { command tree "$@" | ccze_dracula -A; }
find()       { command find "$@" | ccze_dracula -A; }

uptime()     { command uptime "$@" | ccze_dracula -A; }
# date and cat intentionally not wrapped

# native colors

alias ls='ls --color=auto'
alias ll='ls -la'
alias grep='grep --color=auto'
alias diff='diff --color=auto'
alias ip='ip -color=auto'

alias vim='nvim'
alias nano='nvim'

# dracula man pages

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

# plugins

# syntax highlighting
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

# autosuggestions - comment gray = ghost text, clearly not yet typed
if [ -f /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh ]; then
    source /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh
    ZSH_AUTOSUGGEST_HIGHLIGHT_STYLE='fg=#6272a4'
fi

# zsh-z
if [ -f ~/.local/share/zsh/plugins/zsh-z/zsh-z.plugin.zsh ]; then
    source ~/.local/share/zsh/plugins/zsh-z/zsh-z.plugin.zsh
    ZSHZ_DATA="${XDG_CACHE_HOME:-$HOME/.cache}/zsh/z-data"
fi

# atuin

eval "$(atuin init zsh --disable-up-arrow --disable-ctrl-r)"

# bind ctrl+up to atuin search
bindkey '^[[1;5A' atuin-search

# vi mode: bind 'k' in normal mode to atuin search
bindkey -M vicmd 'k' atuin-search

# aliases

alias update="source ~/.config/zsh/.zshrc"

. "$HOME/.local/share/../bin/env"

# API keys - keep empty in the repo; fill locally (or via your secret store)
export OLLAMA_API_KEY=""
export GEMINI_API_KEY=""
export CEREBRAS_API_KEY=""

# auto-start litellm proxy
if ! pgrep -f "litellm.*4000" > /dev/null 2>&1; then
  nohup litellm --config ~/.pi/litellm-config.yaml --port 4000 \
    > ~/.pi/litellm.log 2>&1 &
fi
export LIBVIRT_DEFAULT_URI="qemu:///system"
export PATH="$HOME/.config/emacs/bin:$PATH"

# opencode
export PATH="$HOME/.opencode/bin:$PATH"
export SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt
export YTMAPI_COOKIE="$HOME/.config/terraform-ytmusic/cookies.txt"
export PATH="$HOME/.cargo/bin:$PATH"

# bun completions
[ -s "$HOME/.bun/_bun" ] && source "$HOME/.bun/_bun"
