/* Taken from https://github.com/djpohly/dwl/issues/466 */
#define COLOR(hex)                                                            \
  { ((hex >> 24) & 0xFF) / 255.0f, ((hex >> 16) & 0xFF) / 255.0f,             \
    ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f }

/* appearance */
static const int sloppyfocus = 1; /* focus follows mouse */
static const int bypass_surface_visibility = 0;
static const int smartgaps = 0; /* 1 means no outer gap when there is only one window */
static int gaps = 0; /* 1 means gaps between windows are added */
static const unsigned int gappx = 6; /* gap pixel between windows */
static const unsigned int borderpx = 0; /* window border width */
static const unsigned int systrayspacing = 2; /* systray spacing */
static const int showsystray = 0; /* 0 means no systray */

static const int showbar = 1; /* 0 means no bar */
static const int topbar = 1; /* 0 means bottom bar */

/* Dracula theme — bar colors: fg, bg, border (uint32_t ARGB, alpha baked in) */
static const float rootcolor[] = COLOR (0x222222ff);
static const float fullscreen_bg[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static int enableautoswallow = 1; /* enables autoswallowing newly spawned clients */
static float swallowborder = 1.0f; /* add this multiplied by borderpx to border when a client is swallowed */
static const float default_opacity = 1.0f;
static const char *fonts[] = { "JetBrainsMono Nerd Font:size=16" };
static uint32_t colors[][3] = {
  /*               fg          bg          border    */
  [SchemeNorm] = { 0xeeeeeeff, 0x222222ee, 0x444444ff }, /* bg 93% alpha */
  [SchemeSel]  = { 0x1e1e2eff, 0xbd93f9dd, 0xbd93f9dd }, /* Dracula purple 87% alpha */
  [SchemeUrg]  = { 0xff0000ff, 0xff0000cc, 0xff0000ff },
};

/* tagging */
static char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static int log_level = WLR_ERROR;

/* appicons - tag bar icons; set to 0 to use defaults */
static char outer_separator_beg = '[';
static char outer_separator_end = ']';
static char inner_separator = ' ';
static unsigned truncate_icons_after = 2;
static char truncate_symbol[] = "...";

/* window rules: appid, title, tags, isfloating, opacity, isterm, noswallow, monitor, appicon */
static const Rule rules[] = {
  { NULL, NULL, 0, 0, 1.0f, 0, 0, -1, "󱂚" }, /* generic fallback icon for unmatched apps */
  { "foot", NULL, 0, 0, 1.0f, 1, 1, -1, "󰆍" },
  { "wmenu-center", NULL, 0, 1, 0.85f, 0, 0, -1, NULL },
  { "steam", NULL, 0, 0, 1.0f, 0, 0, -1, "" },
  { "youtui", NULL, 0, 0, 1.0f, 0, 0, -1, "󰑈" },
  { NULL, "pipemixer", 0, 0, 1.0f, 0, 0, -1, "󱄠" },
  { NULL, "bluetoothctl", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "nvim", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "vim", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "hx", 0, 0, 1.0f, 0, 0, -1, "󰔤" },
  { NULL, "emacs", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "nano", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "htop", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "btop", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "top", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "glances", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "ranger", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "lf", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "yazi", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "newsboat", 0, 0, 1.0f, 0, 0, -1, "󰭹" },
  { NULL, "lazygit", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "git", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "tig", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "gitui", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "python", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "python3", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "node", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "npm", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "yarn", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "deno", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "go", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "rustc", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "cargo", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "rustup", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "gcc", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "g++", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "clang", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "julia", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "R", 0, 0, 1.0f, 0, 0, -1, "ﳒ" },
  { NULL, "ruby", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "php", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "perl", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "java", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "csharp", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "swift", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "lua", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "kotlin", 0, 0, 1.0f, 0, 0, -1, "󰬟" },
  { NULL, "sqlite", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "psql", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "mysql", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "redis", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "docker", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "kubectl", 0, 0, 1.0f, 0, 0, -1, "󱃾" },
  { NULL, "k9s", 0, 0, 1.0f, 0, 0, -1, "󱃾" },
  { NULL, "helm", 0, 0, 1.0f, 0, 0, -1, "󱃾" },
  { NULL, "pacman", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "paru", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "yay", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "apt", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "nala", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "dnf", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "yum", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "systemctl", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "gdb", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "valgrind", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "curl", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "wget", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "rsync", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "ping", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "ssh", 0, 0, 1.0f, 0, 0, -1, "󰣀" },
  { NULL, "scp", 0, 0, 1.0f, 0, 0, -1, "󰣀" },
  { NULL, "tmux", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "make", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "cmake", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "gh", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "sudo", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "ccze", 0, 0, 1.0f, 0, 0, -1, "󰆍" },
  { NULL, "pass", 0, 0, 1.0f, 0, 0, -1, "󰟃" },
  { NULL, "raw-cli", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "wclipmenu", 0, 0, 1.0f, 0, 0, -1, "󰆍" },

  { "imv", NULL, 0, 0, 1.0f, 0, 0, -1, "󰑿" },
  { "thunar", NULL, 0, 0, 1.0f, 0, 0, -1, "" },
  { "file-roller", NULL, 0, 0, 1.0f, 0, 0, -1, "󰹬" },
  { "pavucontrol", NULL, 0, 0, 1.0f, 0, 0, -1, "" },
  { "gamescope", NULL, 0, 0, 1.0f, 0, 0, -1, "" },
  { "wlock", NULL, 0, 0, 1.0f, 0, 0, -1, "" },
  { "zathura", NULL, 0, 0, 1.0f, 0, 0, -1, "󰈙" },
  { "mpv", NULL, 0, 0, 1.0f, 0, 0, -1, "󰗋" },
  { NULL, "procs", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "btm", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "bottom", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "fastfetch", 0, 0, 1.0f, 0, 0, -1, "󰲳" },
  { NULL, "bat", 0, 0, 1.0f, 0, 0, -1, "󰭟" },
  { NULL, "eza", 0, 0, 1.0f, 0, 0, -1, "󰈹" },
  { NULL, "fd", 0, 0, 1.0f, 0, 0, -1, "󰐤" },
  { NULL, "rg", 0, 0, 1.0f, 0, 0, -1, "󰐤" },
  { NULL, "fzf", 0, 0, 1.0f, 0, 0, -1, "󰒄" },
  { NULL, "tree", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "atuin", 0, 0, 1.0f, 0, 0, -1, "󰾯" },
  { NULL, "zoxide", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "tldr", 0, 0, 1.0f, 0, 0, -1, "󰔤" },
  { NULL, "nmap", 0, 0, 1.0f, 0, 0, -1, "󰏂" },
  { NULL, "mtr", 0, 0, 1.0f, 0, 0, -1, "󰏂" },
  { NULL, "ncat", 0, 0, 1.0f, 0, 0, -1, "󰭟" },
  { NULL, "nc", 0, 0, 1.0f, 0, 0, -1, "󰭟" },
  { NULL, "netcat", 0, 0, 1.0f, 0, 0, -1, "󰭟" },
  { NULL, "nethogs", 0, 0, 1.0f, 0, 0, -1, "󰭟" },
  { NULL, "arp-scan", 0, 0, 1.0f, 0, 0, -1, "󰏂" },
  { NULL, "whois", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "dog", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "speedtest-cli", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "tailscale", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "yt-dlp", 0, 0, 1.0f, 0, 0, -1, "󰗋" },
  { NULL, "w3m", 0, 0, 1.0f, 0, 0, -1, "󰩟" },
  { NULL, "dive", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "terraform", 0, 0, 1.0f, 0, 0, -1, "ﲽ" },
  { NULL, "magick", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "convert", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "brightnessctl", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "pamixer", 0, 0, 1.0f, 0, 0, -1, "󱄠" },
  { NULL, "cppcheck", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "luacheck", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "luarocks", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "meson", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "ninja", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "kapc", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "kapd", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "kapg", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "pi", 0, 0, 1.0f, 0, 0, -1, "π" },
  { NULL, "opencode", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "tiny-agents", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "litellm", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "hf", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "huggingface-cli", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "uv", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "uvx", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "rtk", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "me3", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "lite", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "distro", 0, 0, 1.0f, 0, 0, -1, "" },
  { NULL, "dotenv", 0, 0, 1.0f, 0, 0, -1, "󰺩" },
  { NULL, "env", 0, 0, 1.0f, 0, 0, -1, "󰺩" },

  /* browsers last: id-match always wins over any page-title match */
  { "waterfox", NULL, 0, 0, 1.0f, 0, 0, -1, "󰈹" },
  { "chromium", NULL, 0, 0, 1.0f, 0, 0, -1, "󰊯" },
  { "firefox", NULL, 0, 0, 1.0f, 0, 0, -1, "" },
};

/* layouts: tile, floating, monocle */
static const Layout layouts[] = {
  { "[]=", tile },
  { "><>", NULL },
  { "[M]", monocle },
  { "TTT", bstack },
  { "===", bstackhoriz },
};

/* monitor defaults - works with multiple monitors */
static const MonitorRule monrules[] = {
  { NULL, 0.55f, 1, 1, &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL, -1, -1 },
};

/* keyboard - Brazilian ABNT2 */
static const int repeat_rate = 35;
static const int repeat_delay = 200;

static const struct xkb_rule_names xkb_rules = {
  .rules = NULL,
  .model = "abnt2",
  .layout = "br",
  .variant = NULL,
  .options = NULL, /* caps2esc handled at input level via interception-caps2esc */
};

/* trackpad */
static const int natural_scrolling = 0;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int middle_button_emulation = 0;
static const enum libinput_config_scroll_method scroll_method
    = LIBINPUT_CONFIG_SCROLL_2FG;
static const enum libinput_config_click_method click_method
    = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
static const enum libinput_config_accel_profile accel_profile
    = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;
static const enum libinput_config_tap_button_map button_map
    = LIBINPUT_CONFIG_TAP_MAP_LRM;

static const int cursor_timeout = 5; /* hide cursor after N seconds idle */

#define MODKEY WLR_MODIFIER_LOGO /* Super/Windows key */

#define TAGKEYS(KEY, SKEY, TAG)                                               \
  { MODKEY, KEY, view, { .ui = 1 << TAG } },                                  \
      { MODKEY | WLR_MODIFIER_CTRL, KEY, toggleview, { .ui = 1 << TAG } },    \
      { MODKEY | WLR_MODIFIER_SHIFT, SKEY, tag, { .ui = 1 << TAG } },         \
  {                                                                           \
    MODKEY | WLR_MODIFIER_CTRL | WLR_MODIFIER_SHIFT, SKEY, toggletag,         \
    {                                                                         \
      .ui = 1 << TAG                                                          \
    }                                                                         \
  }

#define SHCMD(cmd)                                                            \
  {                                                                           \
    .v = (const char *[]) { "/bin/sh", "-c", cmd, NULL }                      \
  }

/* idle to use with lock */
static const char *const autostart[]
    = { "widle", "-t", "300000", "wlock", NULL, NULL };

/* commands */
static const char *termcmd[] = { "foot", NULL };
static const char *menucmd[] = { "wmenu-run", "-t", NULL };
static const char *dmenucmd[] = { "wmenu", NULL };
static const char *browsercmd[] = { "waterfox", "Waterfox", NULL };
static const char *dismisscmd[] = { "makoctl", "dismiss", NULL };
static const char *passcmd[] = { "passmenu", NULL }; /* requires passmenu (pass) */
static const char *lockcmd[] = { "wlock", NULL };
static const char *clipcmd[] = { "wclipmenu", NULL };
static const char *clipimgcmd[] = { "wclipmenu", "image", NULL };

/* media keys only - FN+F keys handled by laptop firmware */
static const char *brightnessup[] = { "brightnessctl", "set", "+5%", NULL };
static const char *brightnessdown[] = { "brightnessctl", "set", "5%-", NULL };
static const char *volumemute[]
    = { "wpctl", "set-mute", "@DEFAULT_AUDIO_SINK@", "toggle", NULL };
static const char *volumedown[]
    = { "wpctl", "set-volume", "@DEFAULT_AUDIO_SINK@", "5%-", NULL };
static const char *volumeup[]
    = { "wpctl", "set-volume", "@DEFAULT_AUDIO_SINK@", "5%+", NULL };
static const char *audioplay[] = { "playerctl", "-p", "youtui", "play-pause", NULL };
static const char *audionext[] = { "playerctl", "-p", "youtui", "next", NULL };
static const char *audioprev[] = { "playerctl", "-p", "youtui", "previous", NULL };

/* screenshots */
static const char *screenshot_full[] = { "dwl-screenshot", "full", NULL };
static const char *screenshot_area[] = { "dwl-screenshot", "area", NULL };
static const char *screenshot_window[] = { "dwl-screenshot", "window", NULL };
static const char *screenshot_monitor[] = {
  "sh", "-c",
  "grim -l 0 -o $(swaymsg -t get_outputs | jq -r '.[] | select(.focused) | "
  ".name') - | wl-copy",
  NULL
};

static const Key keys[] = {
  /* modifier                  key                 function        argument */

  /* launchers */
  { MODKEY, XKB_KEY_d, spawn, { .v = menucmd } },
  { MODKEY, XKB_KEY_b, spawnorfocus, { .v = browsercmd } },
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_X, togglebar, { 0 } }, /* toggle bar (Shift+X: plain Cmd+X is Mac cut via PiKVM) */
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_D, spawn, { .v = dismisscmd } },
  { MODKEY, XKB_KEY_g, spawn, { .v = passcmd } },
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_Return, spawn, { .v = termcmd } },
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_L, spawn, { .v = lockcmd } },
	{ MODKEY, XKB_KEY_p, spawn, { .v = clipcmd } },
	{ MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_P, spawn, { .v = clipimgcmd } },

  /* window navigation - same monitor */
  { MODKEY, XKB_KEY_j, focusstack, { .i = +1 } }, /* next window */
  { MODKEY, XKB_KEY_k, focusstack, { .i = -1 } }, /* prev window */
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_j, movestack, { .i = +1 } }, /* move window down stack */
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_k, movestack, { .i = -1 } }, /* move window up stack */
  { MODKEY, XKB_KEY_Return, zoom, { 0 } },        /* promote to master */

  /* master area control */
  { MODKEY | WLR_MODIFIER_SHIFT,
    XKB_KEY_I,
    incnmaster,
    { .i = +1 } }, /* more masters */
  { MODKEY | WLR_MODIFIER_SHIFT,
    XKB_KEY_Y,
    incnmaster,
    { .i = -1 } },                                  /* fewer masters */
  { MODKEY, XKB_KEY_h, setmfact, { .f = -0.05f } }, /* shrink master */
  { MODKEY, XKB_KEY_l, setmfact, { .f = +0.05f } }, /* expand master */
  { MODKEY, XKB_KEY_o, setopacity, { .f = +0.1f } },
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_O, setopacity, { .f = -0.1f } },

  /* layouts */
  { MODKEY, XKB_KEY_Tab, view, { 0 } },                   /* last tag */
  { MODKEY, XKB_KEY_t, setlayout, { .v = &layouts[0] } }, /* tile */
  { MODKEY, XKB_KEY_f, setlayout, { .v = &layouts[1] } }, /* float */
  { MODKEY, XKB_KEY_m, setlayout, { .v = &layouts[2] } }, /* monocle */
  { MODKEY, XKB_KEY_u, setlayout, { .v = &layouts[3] } }, /* bottomstack */
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_U, setlayout, { .v = &layouts[4] } }, /* bottomstack horizontal */
  { MODKEY, XKB_KEY_space, setlayout, { 0 } },            /* toggle layout */
  { MODKEY | WLR_MODIFIER_SHIFT,
    XKB_KEY_space,
    togglefloating,
    { 0 } },                                      /* toggle float */
  { MODKEY, XKB_KEY_e, togglefullscreen, { 0 } }, /* fullscreen */
{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_G, togglegaps, { 0 } }, /* toggle gaps */

  /* window kill */
  { MODKEY, XKB_KEY_q, killclient, { 0 } },

  /* tags */
  { MODKEY, XKB_KEY_0, view, { .ui = ~0 } }, /* all tags */
  { MODKEY | WLR_MODIFIER_SHIFT,
    XKB_KEY_parenright,
    tag,
    { .ui = ~0 } }, /* tag all */

  /* monitor management - multi-monitor support */
  { MODKEY,
    XKB_KEY_comma,
    focusmon,
    { .i = WLR_DIRECTION_LEFT } }, /* focus left monitor */
  { MODKEY,
    XKB_KEY_period,
    focusmon,
    { .i = WLR_DIRECTION_RIGHT } }, /* focus right monitor */
  { MODKEY | WLR_MODIFIER_SHIFT,
    XKB_KEY_less,
    tagmon,
    { .i = WLR_DIRECTION_LEFT } }, /* window to left monitor */
  { MODKEY | WLR_MODIFIER_SHIFT,
    XKB_KEY_greater,
    tagmon,
    { .i = WLR_DIRECTION_RIGHT } }, /* window to right monitor */

  /* brightness - FN+F3/F4 (media keys only, no plain F-keys) */
  { 0, XKB_KEY_XF86MonBrightnessDown, spawn, { .v = brightnessdown } },
  { 0, XKB_KEY_XF86MonBrightnessUp, spawn, { .v = brightnessup } },

  /* volume - F6 mute / F7 down / F8 up + media keys */
  { 0, XKB_KEY_F6, spawn, { .v = volumemute } },
  { 0, XKB_KEY_F7, spawn, { .v = volumedown } },
  { 0, XKB_KEY_F8, spawn, { .v = volumeup } },
  { 0, XKB_KEY_XF86AudioMute, spawn, { .v = volumemute } },
  { 0, XKB_KEY_XF86AudioLowerVolume, spawn, { .v = volumedown } },
  { 0, XKB_KEY_XF86AudioRaiseVolume, spawn, { .v = volumeup } },

  /* media control - youtui */
  { 0, XKB_KEY_XF86AudioPlay, spawn, { .v = audioplay } },
  { 0, XKB_KEY_XF86AudioNext, spawn, { .v = audionext } },
  { 0, XKB_KEY_XF86AudioPrev, spawn, { .v = audioprev } },

  /* screenshots */
  { MODKEY, XKB_KEY_s, spawn, { .v = screenshot_area } },
  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_S, spawn, { .v = screenshot_full } },
  { MODKEY | WLR_MODIFIER_CTRL, XKB_KEY_s, spawn, { .v = screenshot_window } },
  { MODKEY | WLR_MODIFIER_ALT,
    XKB_KEY_s,
    spawn,
    { .v = screenshot_monitor } }, /* current monitor */

  TAGKEYS (XKB_KEY_1, XKB_KEY_exclam, 0),
  TAGKEYS (XKB_KEY_2, XKB_KEY_at, 1),
  TAGKEYS (XKB_KEY_3, XKB_KEY_numbersign, 2),
  TAGKEYS (XKB_KEY_4, XKB_KEY_dollar, 3),
  TAGKEYS (XKB_KEY_5, XKB_KEY_percent, 4),
  TAGKEYS (XKB_KEY_6, XKB_KEY_asciicircum, 5),
  TAGKEYS (XKB_KEY_7, XKB_KEY_ampersand, 6),
  TAGKEYS (XKB_KEY_8, XKB_KEY_asterisk, 7),
  TAGKEYS (XKB_KEY_9, XKB_KEY_parenleft, 8),

  { MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_Q, quit, { 0 } },

  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT,
    XKB_KEY_Terminate_Server,
    quit,
    { 0 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F1, chvt, { .ui = 1 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F2, chvt, { .ui = 2 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F3, chvt, { .ui = 3 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F4, chvt, { .ui = 4 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F5, chvt, { .ui = 5 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F6, chvt, { .ui = 6 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F7, chvt, { .ui = 7 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F8, chvt, { .ui = 8 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F9, chvt, { .ui = 9 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F10, chvt, { .ui = 10 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F11, chvt, { .ui = 11 } },
  { WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_F12, chvt, { .ui = 12 } },
};

static const Button buttons[] = {
  { ClkLtSymbol, 0, BTN_LEFT, setlayout, { .v = &layouts[0] } },
  { ClkLtSymbol, 0, BTN_RIGHT, setlayout, { .v = &layouts[2] } },
  { ClkTitle, 0, BTN_MIDDLE, zoom, { 0 } },
  { ClkStatus, 0, BTN_MIDDLE, spawn, { .v = termcmd } },
  { ClkClient, MODKEY, BTN_LEFT, moveresize, { .ui = CurMove } },
  { ClkClient, MODKEY, BTN_MIDDLE, togglefloating, { 0 } },
  { ClkClient, MODKEY, BTN_RIGHT, moveresize, { .ui = CurResize } },
  { ClkTagBar, 0, BTN_LEFT, view, { 0 } },
  { ClkTagBar, 0, BTN_RIGHT, toggleview, { 0 } },
  { ClkTagBar, MODKEY, BTN_LEFT, tag, { 0 } },
  { ClkTagBar, MODKEY, BTN_RIGHT, toggletag, { 0 } },
  { ClkTray, 0, BTN_LEFT, trayactivate, { 0 } },
  { ClkTray, 0, BTN_RIGHT, traymenu, { 0 } },
};
