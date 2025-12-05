/* See LICENSE file for copyright and license details. */

/* Location for sunrise/sunset calculation */
static const double latitude  = 30.0;   /* degrees N */
static const double longitude = 31.0;   /* degrees E */

/* Update interval in seconds (daemon mode) */
static const unsigned int interval = 300;

/* Test mode interval in seconds */
static const unsigned int test_interval = 6;

/* Fade transition settings (uses pre-generated frames) */
static const int fade_enabled = 1;       /* 0 = disabled, 1 = enabled */
static const int fade_frames = 10;       /* number of frames per transition */
static const int fade_frame_ms = 50;     /* ms between frames (10*50ms = 0.5s fade) */

/* Battery throttling - reduce effects when on battery */
static const int battery_throttle = 1;
static const int battery_fade_frames = 5;

/* Monitor configurations
 * name:  xrandr output name
 * theme: folder under ~/.config/dynamic-wallpaper/themes/
 * video: video file path (NULL = use time-based images)
 *
 * Order must match xrandr left-to-right for feh.
 */
struct monitor_config {
	const char *name;
	const char *theme;
	const char *video;
};

static struct monitor_config monitors[] = {
	{ "DP-1",    "catalina", NULL },
	{ "DP-2",    "monterey", NULL },
	{ "DVI-I-1", "catalina", NULL },
};

#define NUM_MONITORS (sizeof(monitors) / sizeof(monitors[0]))

/* Stage image filenames (inside each theme folder) */
static const char *stagefiles[] = {
	"01_night.jpg",
	"02_late_night.jpg",
	"03_dawn.jpg",
	"04_sunrise.jpg",
	"05_morning.jpg",
	"06_noon.jpg",
	"07_afternoon.jpg",
	"08_sunset.jpg",
	"09_dusk.jpg",
	"10_early_night.jpg",
};

/* Stage names for display */
static const char *stagenames[] = {
	"night",
	"late_night",
	"dawn",
	"sunrise",
	"morning",
	"noon",
	"afternoon",
	"sunset",
	"dusk",
	"early_night",
};

#define NUM_STAGES (sizeof(stagefiles) / sizeof(stagefiles[0]))
