# Lines configured by zsh-newuser-install
HISTFILE=~/.histfile
HISTSIZE=1000
SAVEHIST=1000
unsetopt beep
bindkey -v
# End of lines configured by zsh-newuser-install

# The following lines were added by compinstall
zstyle :compinstall filename '/home/beto/.zshrc'

autoload -Uz compinit
compinit
# End of lines added by compinstall

# Enable colors and prompt substitution
autoload -U colors && colors
setopt promptsubst  # Required for $variables in PS1

# Version Control System Information
autoload -Uz vcs_info

precmd_vcs_info() { vcs_info }
precmd_functions+=( precmd_vcs_info )

zstyle ':vcs_info:*' enable git
zstyle ':vcs_info:*' check-for-changes true
zstyle ':vcs_info:git*+set-message:*' hooks git-untracked git-st git-stash

# Git status format (modern %F{} syntax)
zstyle ':vcs_info:git*:*' formats "%F{white}on %F{blue} %b %F{red}%u%F{green}%c%m"

# Show ? for untracked files
function +vi-git-untracked(){
  if [[ $(git rev-parse --is-inside-work-tree 2> /dev/null) == 'true' ]] && \
    git status --porcelain | grep '??' &> /dev/null ; then
    hook_com[staged]+="%F{red}?"
  fi
}

# Show ahead/behind counts
function +vi-git-st() {
  local ahead behind
  local -a gitstatus

  # Exit early in case the worktree is on a detached HEAD
  git rev-parse ${hook_com[branch]}@{upstream} >/dev/null 2>&1 || return 0

  local -a ahead_and_behind=(
    $(git rev-list --left-right --count HEAD...${hook_com[branch]}@{upstream} 2>/dev/null)
  )

  ahead=${ahead_and_behind[1]}
  behind=${ahead_and_behind[2]}

  (( $ahead )) && gitstatus+=( "%F{green}⇡${ahead}" )
  (( $behind )) && gitstatus+=( "%F{red}⇣${behind}" )

  hook_com[misc]+=${(j:/:)gitstatus}
}

# Show stash count
function +vi-git-stash() {
  local -a stashes

  if [[ -s ${hook_com[base]}/.git/refs/stash ]] ; then
    stashes=$(git stash list 2>/dev/null | wc -l)
    hook_com[misc]+="%F{yellow}[${stashes} ]"
  fi
}

# Two-line prompt with git info and colored exit status
PS1='%F{cyan}%B%~%b $vcs_info_msg_0_ %f'
PS1+=$'\n%(?.%F{green}.%F{red})❯ %f'
eval "$(atuin init zsh)"

export PATH="$HOME/.cargo/bin:$PATH"
alias dotfiles='/usr/bin/git --git-dir=/home/beto/.dotfiles.git --work-tree=/home/beto'
