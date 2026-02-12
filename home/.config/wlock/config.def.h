static Clr colorname[4] = {
	[INIT]      = { 0x19192bff, 0x19192bff, 0x19192bff }, /* Dark purple background */
	[INPUT]     = { 0x24243bff, 0x24243bff, 0x24243bff }, /* Subtle purple input */
	[INPUT_ALT] = { 0x2f2f4bff, 0x2f2f4bff, 0x2f2f4bff }, /* Slightly lighter */
	[FAILED]    = { 0x5c1c1cff, 0x5c1c1cff, 0x5c1c1cff }, /* Dark red */
};

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;
