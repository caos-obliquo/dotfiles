#
# ~/.bashrc
#

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

alias qcy-connect="~/bt-qcy-connect.sh"
alias qcy-disconnect="~/bt-qcy-disconnect.sh"

alias ls='ls --color=auto'
alias grep='grep --color=auto'
PS1='[\u@\h \W]\$ '
export DBUS_SESSION_BUS_ADDRESS="unix:path=$XDG_RUNTIME_DIR/bus"

# export GTK_THEME=Arc-Dark
export  QT_QPA_PLATFORMTHEME=gtk2
# Punpun welcome message

 Display Punpun on terminal startup
if [[ $- == *i* ]] && [[ "$TERM" == *"kitty"* ]] && [ -n "$KITTY_WINDOW_ID" ]; then
    clear
    cat << 'PUNPUN'
⠂⡅⠠⠖⡐⢆⠴⣀⠅⠰⢌⡠⠄⢦⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣴⠒⡠⡅⠦⠡⠢⢅⣂⠒⡜⠘⠦⢡⠎⠄⡚⣨⠢
⠑⡄⡤⢐⡐⠠⠂⡁⠌⡂⣌⠐⢉⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣐⡈⡔⢡⠃⢂⡰⡈⢢⢉⢢⠃⠜⡡⢑⠠⣂
⠘⡀⠆⠦⡘⢉⠔⠔⢉⣠⣴⡿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣔⠸⠡⣐⠌⠆⠩⠔⡈⠵⡈⢄⠟⠤
⢀⠓⡘⢢⠌⢃⡌⠰⣾⠟⡁⡒⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡝⢻⣿⣧⡢⢐⡌⡍⠢⡑⠌⡬⠱⢎⠬⠐
⠈⡖⠡⠣⢐⠅⡊⣼⡯⢉⠐⢼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⡁⡼⢿⣷⡄⡐⡄⠣⣈⠝⡰⡉⢆⢃⡁
⢨⡐⢁⠑⠌⡂⣼⠟⢠⠒⡈⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣶⡬⣻⣷⣉⠴⡉⢆⢣⠱⢐⢊⢔⠨
⢂⡁⠄⡑⡈⢴⡿⠒⢄⢥⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠜⣿⣧⢎⡡⠣⠄⡭⠃⢅⠊⡡
⠠⠎⡐⠠⠌⣿⠇⢡⢹⣆⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡏⠬⣿⣦⠱⣈⡍⠲⣘⠨⣓⠰
⠘⡁⠌⣀⢍⣿⠄⡅⠾⣿⣿⣿⣿⣿⣿⣿⣧⢻⣿⡏⣷⠈⣿⣶⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣏⠰⣸⣿⣐⠆⡱⢉⠔⡑⡐⡰
⢡⠜⢂⡂⡜⢿⣈⡔⡢⣿⣽⣿⣿⣿⣿⣿⣿⣾⣿⣷⣿⣆⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡷⢢⣹⣿⠆⠜⡙⡔⡊⡄⢍⣂
⠠⡌⢸⡐⢂⠾⣇⢰⠡⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣾⣿⣸⣿⡿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⡱⢺⣿⢆⠳⣘⡤⡉⢎⡰⢌
⡑⠶⣐⡌⠣⡘⢿⣄⢋⡴⢩⢹⣿⣿⣿⣿⣿⣿⡿⢿⣿⣿⣿⣿⣷⣼⣿⣿⡿⢽⣿⡿⣻⣏⣷⣎⣱⣿⣿⡚⣑⣃⠆⡋⢦⢚⠬
⢒⣡⠢⠆⣣⠒⣉⢻⣷⣼⣿⣿⣿⡏⠉⠙⠛⠛⠿⠿⠿⠿⠿⠛⠛⠋⠉⣁⠀⠀⣷⡝⣿⣿⣿⣿⣿⣿⣿⣖⡬⢎⡗⡹⢰⢣⠜
⠱⡂⠝⡚⡔⢫⢠⠘⢬⣿⣿⣿⣿⣿⠀⠀⢀⣶⡄⠀⠀⠀⠀⠀⠀⠀⢸⣿⣧⠀⣯⡽⣾⣿⣿⣿⣿⣿⣿⣿⡜⢇⡺⢃⣯⠒⡏
⣁⡉⣶⠱⡈⢇⠱⠎⣿⣏⣿⣿⣿⣿⡀⠀⠀⣿⣿⠀⠀⠀⠀⠀⠀⠀⠈⢿⡿⠀⣿⡾⣹⣿⣿⣿⣿⣿⣿⣿⡸⢇⡸⡉⡆⢏⢱
⢦⠑⣪⢳⢹⡊⢯⠌⣿⣿⣿⣿⣿⣿⡇⠀⠀⠈⠋⠀⠀⠀⠀⠀⠀⠀⠀⠈⠀⠀⣯⣞⣽⣿⣿⣿⣿⣿⢿⠖⣭⢳⡏⡝⡞⣭⢚
⢎⣫⠥⢣⣳⢜⣢⢛⡜⡟⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⣠⣄⠀⠀⠀⠀⠀⠀⠀⣷⢧⢾⣿⣿⣿⡟⢷⣋⢾⢌⡳⣏⠾⡱⢆⣩
⢓⡎⢗⡫⣜⡊⠴⣊⠟⣸⣑⢻⣛⣋⣿⡀⠀⠀⠀⠀⠀⠙⣿⡆⠀⢸⡇⠀⠀⠀⣯⣛⣼⣿⣻⢥⣛⡶⣭⣙⡎⣷⡩⣚⣭⢞⢦
⡽⣜⡫⣵⢮⣹⢫⢵⣫⠝⣎⣷⣮⣷⣾⠀⠀⠀⠀⠀⠀⠀⢹⡇⠀⣿⠁⠀⠀⠀⠸⣭⣶⡿⣿⣾⣽⣾⣥⣿⣼⣲⣏⣞⡼⢏⡾
⢞⡵⣛⡶⣻⢆⠷⣋⣖⣟⣽⣷⡎⣿⣿⣷⣶⣤⣤⣄⣀⣀⣸⡗⠀⣿⣀⣠⣤⣶⣿⣿⣧⡷⣝⣾⣿⣻⢟⠛⠛⣿⣿⣿⣿⣯⡽
⡽⣺⣵⣳⡭⢞⡿⠉⠉⠀⠀⠹⣷⣿⣿⣿⣶⣾⣷⡆⠈⠉⢹⡇⢸⣿⠛⣿⢻⣼⣿⣿⣿⣞⣳⣾⣾⢼⣻⠀⠀⣾⣿⣿⣿⣿⣿
⣵⣿⣷⣿⡇⣿⣿⡆⠀⠀⠀⠀⠙⣿⣿⣿⣿⣿⣿⣧⠀⠀⠈⣧⢸⡟⠀⢸⣧⣿⣿⣿⣿⣯⣞⣿⢮⡟⡝⠀⠀⣿⣏⣿⣿⣿⣿
⣿⣿⣿⣿⣷⡏⣿⣿⠀⠀⠀⠀⠀⠈⢿⣿⣿⣿⣿⣿⠀⠀⠰⣿⣟⠇⠀⢸⣿⣼⣿⣿⣿⣿⣮⣿⢯⡼⡇⠀⠀⣿⣼⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⢨⣿⣿⣿⣿⢽⡆⠀⠸⢹⣿⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣯⢷⣹⠆⢠⡀⣿⣿⣿⢿⢿⣿
⣿⣿⣿⣿⣿⣟⣿⣿⣷⠀⠀⠀⠀⣰⣿⢸⣿⣿⣿⣿⣇⠀⠀⠆⠀⠀⠀⠀⣿⢿⣿⣿⣿⣿⡽⣿⣿⣾⣧⣄⡸⣿⣿⣿⣼⣿⣿
⠛⣩⣿⣿⣿⣿⣿⣿⣿⡄⠀⢠⣾⣟⣿⣦⣬⣿⣿⣿⣿⠀⠀⠀⠀⣤⣶⣶⣿⣿⡟⣣⢿⣿⣽⣳⣯⣻⡿⣿⢿⣿⣿⣿⣿⣿⣿
⣾⣿⣿⣿⣿⣿⣿⣿⣿⣏⣴⣟⡳⣾⡵⣿⣿⡏⠋⠿⠿⠶⢖⣶⡶⠛⠛⠿⠣⠿⠘⡽⣺⣿⣯⢷⣏⣷⣻⣟⣿⣼⣯⣿⢿⣿⣻
⣿⣿⣿⣿⣿⣿⣿⣟⣿⣟⣯⢷⣹⣳⡟⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⢽⣿⣟⣯⡿⣾⢿⣼⣷⣿⣶⣯⣿⣟⣿
⣿⣿⣿⡿⣿⣿⣿⢿⣟⣾⣹⢟⡾⣽⣻⣽⣿⣿⠻⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣯⣻⣿⣿⣳⣿⡻⣟⣷⣾⣳⣷⣻⣾⡽⢾
PUNPUN
    echo
    ~/.local/bin/punpun-quotes.sh
    echo
fi
unset FF_LOGO_TYPE FF_LOGO_SOURCE FF_CONFIG_PATH 2>/dev/null
unset FF_LOGO_TYPE FF_LOGO_SOURCE FF_CONFIG_PATH 2>/dev/null


# Gaming optimizations
export RADV_PERFTEST=aco,sam
export AMD_VULKAN_ICD=RADV
export __GL_SHADER_DISK_CACHE=1
export __GL_SHADER_DISK_CACHE_PATH="$HOME/.cache/gl_shader_cache"
export PATH="$HOME/.local/bin:$PATH"
export PS1="[\u@\h]\$ "
alias zen='zen-safe'
alias screenshot='screenshot-safe'
alias spotify="PULSE_SERVER=\"\" ALSA_PCM_DEVICE=0 ALSA_PCM_CARD=1 /usr/bin/spotify"
alias docker-desktop-start="systemctl --user stop docker-desktop.service; pkill -f docker-desktop; WAYLAND_DISPLAY=\$WAYLAND_DISPLAY XDG_RUNTIME_DIR=\$XDG_RUNTIME_DIR /opt/docker-desktop/bin/docker-desktop &"
alias docker-desktop="/usr/local/bin/docker-desktop-launch"
alias docker-desktop="systemctl --user stop docker-desktop.service 2>/dev/null; pkill -9 -f docker-desktop 2>/dev/null; WAYLAND_DISPLAY=\$WAYLAND_DISPLAY XDG_RUNTIME_DIR=\$XDG_RUNTIME_DIR /opt/docker-desktop/bin/docker-desktop"
alias config='/usr/bin/git --git-dir=/home/caos-obliquo/.cfg/ --work-tree=/home/caos-obliquo'
alias dotfiles='/usr/bin/git --git-dir=$HOME/.cfg/ --work-tree=$HOME'
