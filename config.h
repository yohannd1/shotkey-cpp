/* the main mod mask. Used */
#define Mod Mod4Mask

/* the shell used to spawn commands */
char shell[] = "/bin/sh";

/* the possible modes to be used */
enum {
	Music,
	Bright,

	// Declare modes above this
	MODE_SIZE,
};

// Define mode key bindings here
// NOTE: "10" here is the maximum number of key bindings for each mode
Key modes[MODE_SIZE][10] = {
	[Music] = {
		{ 0, XK_m,   CMD("notify-send inside_music") },
	},
	[Bright] = {
		{ 0, XK_m,   CMD("notify-send inside_bright") },
		{ 0, XK_n,   CMD("notify-send inside_bright_1") },
	},
};

// Define normal mode key bindings here
Key keys[] = {
	{ Mod|ShiftMask, XK_y,   CMD("notify-send hello") },
	{ Mod|ShiftMask, XK_z,   MODE(Music, False) },
	{ Mod|ShiftMask, XK_x,   MODE(Bright, True) },
};

/* useful metadata */
ModeProperties mode_properties[MODE_SIZE] = {
	[Music] = { "Music player" },
	[Bright] = { "Brightness" },
};

/* shell script called on mode change */
char* on_mode_change = "notify-send \"kadj [$SHOTKEY_MODE_ID] $SHOTKEY_MODE_LABEL\"";
