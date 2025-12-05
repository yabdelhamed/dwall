/* See LICENSE file for copyright and license details. */

/* Location for sunrise/sunset calculation */
static const double latitude  = 30.0;   /* degrees N */
static const double longitude = 31.0;   /* degrees E */

/* Wallpaper setter command */
static const char *setter[] = { "feh", "--bg-fill", NULL };

/* Update interval in seconds (daemon mode) */
static const unsigned int interval = 300;

/* Fade transition settings (uses pre-generated frames from gen-transitions.sh) */
static const int fade_enabled = 1;       /* 0 = disabled, 1 = enabled */
static const int fade_frames = 10;       /* number of frames per transition */
static const int fade_frame_ms = 50;     /* ms between frames (10*50ms = 0.5s fade) */

/* Wallpaper images for each stage
 * Order: night, late_night, dawn, sunrise, morning,
 *        noon, afternoon, sunset, dusk, early_night
 */
static const char *wallpapers[] = {
	[0] = "~/.config/dynamic-wallpaper/images/01_night.jpg",
	[1] = "~/.config/dynamic-wallpaper/images/02_late_night.jpg",
	[2] = "~/.config/dynamic-wallpaper/images/03_dawn.jpg",
	[3] = "~/.config/dynamic-wallpaper/images/04_sunrise.jpg",
	[4] = "~/.config/dynamic-wallpaper/images/05_morning.jpg",
	[5] = "~/.config/dynamic-wallpaper/images/06_noon.jpg",
	[6] = "~/.config/dynamic-wallpaper/images/07_afternoon.jpg",
	[7] = "~/.config/dynamic-wallpaper/images/08_sunset.jpg",
	[8] = "~/.config/dynamic-wallpaper/images/09_dusk.jpg",
	[9] = "~/.config/dynamic-wallpaper/images/10_early_night.jpg",
};
