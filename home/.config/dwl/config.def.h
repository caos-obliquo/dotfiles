/* Taken from https://github.com/djpohly/dwl/issues/466 */
#define COLOR(hex)    { ((hex >> 24) & 0xFF) / 255.0f, \
                        ((hex >> 16) & 0xFF) / 255.0f, \
                        ((hex >> 8) & 0xFF) / 255.0f, \
                        (hex & 0xFF) / 255.0f }

/* appearance */
static const int sloppyfocus               = 1;  /* focus follows mouse */
static const int bypass_surface_visibility = 0;
static const unsigned int borderpx         = 1;  /* window border width */

/* Dracula theme */
static const float rootcolor[]             = COLOR(0x222222ff);
static const float bordercolor[]           = COLOR(0x444444ff);
static const float focuscolor[]            = COLOR(0xbd93f9ff);  /* purple */
static const float urgentcolor[]           = COLOR(0xff0000ff);

static const float fullscreen_bg[]         = {0.0f, 0.0f, 0.0f, 1.0f};

#define TAGCOUNT (9)

static int log_level = WLR_ERROR;

/* window rules - gimp floating, firefox on tag 9 */
static const Rule rules[] = {
	{ "Gimp_EXAMPLE",     NULL,       0,            1,           -1 },
	{ "firefox_EXAMPLE",  NULL,       1 << 8,       0,           -1 },
};

/* layouts: tile, floating, monocle */
static const Layout layouts[] = {
	{ "[]=",      tile },
	{ "><>",      NULL },
	{ "[M]",      monocle },
};

/* monitor defaults - works with multiple monitors */
static const MonitorRule monrules[] = {
	{ NULL,       0.55f, 1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
};

/* keyboard - Brazilian ABNT2 */
static const int repeat_rate = 35;
static const int repeat_delay = 200;

static const struct xkb_rule_names xkb_rules = {
	.rules = NULL,
	.model = "abnt2",
	.layout = "br",
	.variant = NULL,
	.options = NULL,
};

/* trackpad */
static const int natural_scrolling = 0;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int middle_button_emulation = 0;
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

#define MODKEY WLR_MODIFIER_LOGO  /* Super/Windows key */

#define TAGKEYS(KEY,SKEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_SHIFT, SKEY,           tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* idle to use with lock */
static const char *const autostart[] = {
    "widle", "-t", "300000", "wlock", NULL,
    NULL
};

/* commands */
static const char *termcmd[] = { "foot", NULL };
static const char *menucmd[] = { "wmenu-run", "-t", NULL };
static const char *lockcmd[] = { "wlock", NULL };

/* media keys only - FN+F keys handled by laptop firmware */
static const char *brightnessup[]   = { "brightnessctl", "set", "+5%", NULL };
static const char *brightnessdown[] = { "brightnessctl", "set", "5%-", NULL };
static const char *volumemute[]   = { "wpctl", "set-mute", "@DEFAULT_AUDIO_SINK@", "toggle", NULL };
static const char *volumedown[]   = { "wpctl", "set-volume", "@DEFAULT_AUDIO_SINK@", "5%-", NULL };
static const char *volumeup[]     = { "wpctl", "set-volume", "@DEFAULT_AUDIO_SINK@", "5%+", NULL };

/* screenshots */
static const char *screenshot_full[]   = { "dwl-screenshot", "full", NULL };
static const char *screenshot_area[]   = { "dwl-screenshot", "area", NULL };
static const char *screenshot_window[] = { "dwl-screenshot", "window", NULL };
static const char *screenshot_monitor[] = { "sh", "-c", "grim -l 0 -o $(swaymsg -t get_outputs | jq -r '.[] | select(.focused) | .name') - | wl-copy", NULL };

static const Key keys[] = {
	/* modifier                  key                 function        argument */
	
	/* launchers */
	{ MODKEY,                    XKB_KEY_d,          spawn,          {.v = menucmd} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Return,     spawn,          {.v = termcmd} },
    { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_L,          spawn,          {.v = lockcmd} },
	
	/* window navigation - same monitor */
	{ MODKEY,                    XKB_KEY_j,          focusstack,     {.i = +1} },  /* next window */
	{ MODKEY,                    XKB_KEY_k,          focusstack,     {.i = -1} },  /* prev window */
	{ MODKEY,                    XKB_KEY_Return,     zoom,           {0} },        /* promote to master */
	
	/* master area control */
	{ MODKEY,                    XKB_KEY_i,          incnmaster,     {.i = +1} },  /* more masters */
	{ MODKEY,                    XKB_KEY_p,          incnmaster,     {.i = -1} },  /* fewer masters */
	{ MODKEY,                    XKB_KEY_h,          setmfact,       {.f = -0.05f} }, /* shrink master */
	{ MODKEY,                    XKB_KEY_l,          setmfact,       {.f = +0.05f} }, /* expand master */
	
	/* layouts */
	{ MODKEY,                    XKB_KEY_Tab,        view,           {0} },        /* last tag */
	{ MODKEY,                    XKB_KEY_t,          setlayout,      {.v = &layouts[0]} }, /* tile */
	{ MODKEY,                    XKB_KEY_f,          setlayout,      {.v = &layouts[1]} }, /* float */
	{ MODKEY,                    XKB_KEY_m,          setlayout,      {.v = &layouts[2]} }, /* monocle */
	{ MODKEY,                    XKB_KEY_space,      setlayout,      {0} },        /* toggle layout */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space,      togglefloating, {0} },        /* toggle float */
	{ MODKEY,                    XKB_KEY_e,          togglefullscreen, {0} },      /* fullscreen */
	
	/* window kill */
	{ MODKEY,                    XKB_KEY_q,          killclient,     {0} },
	
	/* tags */
	{ MODKEY,                    XKB_KEY_0,          view,           {.ui = ~0} }, /* all tags */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_parenright, tag,            {.ui = ~0} }, /* tag all */
	
	/* monitor management - multi-monitor support */
	{ MODKEY,                    XKB_KEY_comma,      focusmon,       {.i = WLR_DIRECTION_LEFT} },   /* focus left monitor */
	{ MODKEY,                    XKB_KEY_period,     focusmon,       {.i = WLR_DIRECTION_RIGHT} },  /* focus right monitor */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_less,       tagmon,         {.i = WLR_DIRECTION_LEFT} },   /* window to left monitor */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_greater,    tagmon,         {.i = WLR_DIRECTION_RIGHT} },  /* window to right monitor */
	
	/* brightness - FN+F3/F4 (media keys only, no plain F-keys) */
	{ 0,                         XKB_KEY_XF86MonBrightnessDown, spawn, {.v = brightnessdown} },
	{ 0,                         XKB_KEY_XF86MonBrightnessUp,   spawn, {.v = brightnessup} },
	
	/* volume - FN+F6/F7/F8 (media keys only, no plain F-keys) */
	{ 0,                         XKB_KEY_XF86AudioMute,        spawn, {.v = volumemute} },
	{ 0,                         XKB_KEY_XF86AudioLowerVolume, spawn, {.v = volumedown} },
	{ 0,                         XKB_KEY_XF86AudioRaiseVolume, spawn, {.v = volumeup} },
	
	/* screenshots */
	{ MODKEY,                    XKB_KEY_s,          spawn,          {.v = screenshot_area} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_S,          spawn,          {.v = screenshot_full} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_s,          spawn,          {.v = screenshot_window} },
	{ MODKEY|WLR_MODIFIER_ALT,   XKB_KEY_s,          spawn,          {.v = screenshot_monitor} }, /* current monitor */
	
	TAGKEYS(          XKB_KEY_1, XKB_KEY_exclam,                     0),
	TAGKEYS(          XKB_KEY_2, XKB_KEY_at,                         1),
	TAGKEYS(          XKB_KEY_3, XKB_KEY_numbersign,                 2),
	TAGKEYS(          XKB_KEY_4, XKB_KEY_dollar,                     3),
	TAGKEYS(          XKB_KEY_5, XKB_KEY_percent,                    4),
	TAGKEYS(          XKB_KEY_6, XKB_KEY_asciicircum,                5),
	TAGKEYS(          XKB_KEY_7, XKB_KEY_ampersand,                  6),
	TAGKEYS(          XKB_KEY_8, XKB_KEY_asterisk,                   7),
	TAGKEYS(          XKB_KEY_9, XKB_KEY_parenleft,                  8),
	
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Q,          quit,           {0} },

	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_Terminate_Server, quit, {0} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F1, chvt, {.ui = 1} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F2, chvt, {.ui = 2} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F3, chvt, {.ui = 3} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F4, chvt, {.ui = 4} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F5, chvt, {.ui = 5} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F6, chvt, {.ui = 6} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F7, chvt, {.ui = 7} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F8, chvt, {.ui = 8} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F9, chvt, {.ui = 9} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F10, chvt, {.ui = 10} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F11, chvt, {.ui = 11} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F12, chvt, {.ui = 12} },
};

static const Button buttons[] = {
	{ MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
	{ MODKEY, BTN_MIDDLE, togglefloating, {0} },
	{ MODKEY, BTN_RIGHT,  moveresize,     {.ui = CurResize} },
};
