static Clr colorname[4] = {
	[INIT]      = { 0x282a36ff, 0x282a36ff, 0x282a36ff }, /* Dracula background */
	[INPUT]     = { 0x343746ff, 0x343746ff, 0x343746ff }, /* Subtle step up from bg */
	[INPUT_ALT] = { 0x44475aff, 0x44475aff, 0x44475aff }, /* Dracula current line */
	[FAILED]    = { 0xff5555ff, 0xff5555ff, 0xff5555ff }, /* Dracula red (wrong password) */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;
