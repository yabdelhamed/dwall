/* See LICENSE file for copyright and license details. */
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <wordexp.h>

#include "config.h"

enum { Night, LateNight, Dawn, Sunrise, Morning, Noon, Afternoon, Sunset, Dusk, EarlyNight };

static volatile sig_atomic_t running = 1;
static int laststage = -1;
static pid_t video_pids[16] = {0};

static void
sighandler(int sig)
{
	(void)sig;
	running = 0;
}

static void
die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

static int
on_battery(void)
{
	FILE *f;
	char status[32] = {0};

	f = fopen("/sys/class/power_supply/BAT0/status", "r");
	if (!f)
		f = fopen("/sys/class/power_supply/BAT1/status", "r");
	if (!f)
		return 0;

	if (fgets(status, sizeof(status), f))
		status[strcspn(status, "\n")] = 0;
	fclose(f);

	return strcmp(status, "Discharging") == 0;
}

static int
get_fade_frames(void)
{
	if (battery_throttle && on_battery())
		return battery_fade_frames;
	return fade_frames;
}

static void
suntimes(int *rise_h, int *rise_m, int *set_h, int *set_m)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int doy = t->tm_yday + 1;

	double decl = -23.45 * cos((M_PI / 180.0) * (360.0 / 365.0) * (doy + 10));
	double lat_r = latitude * M_PI / 180.0;
	double decl_r = decl * M_PI / 180.0;

	double cos_ha = -tan(lat_r) * tan(decl_r);
	if (cos_ha > 1.0) cos_ha = 1.0;
	if (cos_ha < -1.0) cos_ha = -1.0;

	double ha = acos(cos_ha) * 180.0 / M_PI;
	double noon_time = 12.0 - (longitude / 15.0) + timezone_offset;
	double rise = noon_time - ha / 15.0;
	double set = noon_time + ha / 15.0;

	*rise_h = (int)rise;
	*rise_m = (int)((rise - *rise_h) * 60);
	*set_h = (int)set;
	*set_m = (int)((set - *set_h) * 60);
}

static int
getstage(void)
{
	int rise_h, rise_m, set_h, set_m;
	suntimes(&rise_h, &rise_m, &set_h, &set_m);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int cur = t->tm_hour * 60 + t->tm_min;

	int rise = rise_h * 60 + rise_m;
	int set = set_h * 60 + set_m;
	int noon = (rise + set) / 2;

	if (cur < 2 * 60)           return Night;
	if (cur < 4 * 60)           return LateNight;
	if (cur < rise)             return Dawn;
	if (cur < rise + 30)        return Sunrise;
	if (cur < noon - 60)        return Morning;
	if (cur < noon + 60)        return Noon;
	if (cur < set - 30)         return Afternoon;
	if (cur < set + 30)         return Sunset;
	if (cur < set + 90)         return Dusk;
	return EarlyNight;
}

static char *
expandpath(const char *path)
{
	wordexp_t exp;
	char *result;

	if (wordexp(path, &exp, 0) != 0)
		return strdup(path);

	result = strdup(exp.we_wordv[0]);
	wordfree(&exp);
	return result;
}

static char *
get_theme_image(const char *theme, int stage)
{
	char path[1024];
	snprintf(path, sizeof(path),
		"~/.config/dynamic-wallpaper/themes/%s/%s",
		theme, stagefiles[stage]);
	return expandpath(path);
}

static char *
get_cache_frame(const char *theme, int from_stage, int to_stage, int frame)
{
	char path[1024];
	char frombase[64], tobase[64];
	char *dot;

	snprintf(frombase, sizeof(frombase), "%s", stagefiles[from_stage]);
	if ((dot = strrchr(frombase, '.'))) *dot = 0;
	snprintf(tobase, sizeof(tobase), "%s", stagefiles[to_stage]);
	if ((dot = strrchr(tobase, '.'))) *dot = 0;

	snprintf(path, sizeof(path),
		"~/.config/dynamic-wallpaper/cache/%s/%s_to_%s_%02d.jpg",
		theme, frombase, tobase, frame);
	return expandpath(path);
}

static void
msleep(int ms)
{
	usleep(ms * 1000);
}

static void
kill_videos(void)
{
	for (size_t i = 0; i < NUM_MONITORS; i++) {
		if (video_pids[i] > 0) {
			kill(video_pids[i], SIGTERM);
			waitpid(video_pids[i], NULL, WNOHANG);
			video_pids[i] = 0;
		}
	}
}

static void
start_video(size_t mon_idx, const char *video_path, const char *output)
{
	pid_t pid;

	if ((pid = fork()) == 0) {
		char *path = expandpath(video_path);
		execlp("mpvpaper", "mpvpaper", "-o",
			"no-audio loop", output, path, NULL);
		/* fallback to xwinwrap + mpv */
		execlp("xwinwrap", "xwinwrap", "-g", "1920x1080+0+0", "-ov", "-ni", "-s",
			"-nf", "--", "mpv", "-wid", "WID", "--no-audio", "--loop",
			path, NULL);
		_exit(1);
	} else if (pid > 0) {
		video_pids[mon_idx] = pid;
	}
}

/* Set wallpapers using feh - one path per monitor */
static void
setwall_feh(char **paths, size_t count)
{
	pid_t pid;

	if ((pid = fork()) == 0) {
		char **argv = malloc((2 + count + 1) * sizeof(char *));
		argv[0] = "feh";
		argv[1] = "--bg-fill";
		for (size_t i = 0; i < count; i++)
			argv[2 + i] = paths[i];
		argv[2 + count] = NULL;

		setsid();
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		execvp(argv[0], argv);
		_exit(1);
	} else if (pid > 0) {
		waitpid(pid, NULL, 0);
	}
}

static void
fade_transition(int from_stage, int to_stage)
{
	int frames = get_fade_frames();
	char *paths[NUM_MONITORS];

	for (int f = 1; f <= frames; f++) {
		for (size_t i = 0; i < NUM_MONITORS; i++) {
			paths[i] = get_cache_frame(monitors[i].theme,
				from_stage, to_stage, f);
		}
		setwall_feh(paths, NUM_MONITORS);
		for (size_t i = 0; i < NUM_MONITORS; i++)
			free(paths[i]);
		msleep(fade_frame_ms);
	}
}

static void
setwallpaper(int stage)
{
	char *paths[NUM_MONITORS];
	int has_video = 0;

	if (stage < 0 || stage >= (int)NUM_STAGES)
		return;

	/* Check for video monitors */
	for (size_t i = 0; i < NUM_MONITORS; i++) {
		if (monitors[i].video) {
			has_video = 1;
			break;
		}
	}

	/* Handle video wallpapers */
	if (has_video) {
		kill_videos();
		for (size_t i = 0; i < NUM_MONITORS; i++) {
			if (monitors[i].video) {
				start_video(i, monitors[i].video, monitors[i].name);
			}
		}
	}

	/* Do fade transition if enabled and stage changed */
	if (fade_enabled && laststage >= 0 && laststage != stage) {
		fade_transition(laststage, stage);
	}

	/* Set final wallpapers */
	for (size_t i = 0; i < NUM_MONITORS; i++) {
		if (!monitors[i].video) {
			paths[i] = get_theme_image(monitors[i].theme, stage);
		} else {
			paths[i] = strdup("/tmp/.dwall-placeholder.jpg");
		}
	}

	setwall_feh(paths, NUM_MONITORS);

	for (size_t i = 0; i < NUM_MONITORS; i++)
		free(paths[i]);

	laststage = stage;
}

static void
printstatus(void)
{
	int rise_h, rise_m, set_h, set_m;
	suntimes(&rise_h, &rise_m, &set_h, &set_m);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int stage = getstage();

	printf("dwall - dynamic wallpaper v2.0\n");
	printf("time:     %02d:%02d\n", t->tm_hour, t->tm_min);
	printf("stage:    %s\n", stagenames[stage]);
	printf("sunrise:  %02d:%02d\n", rise_h, rise_m);
	printf("sunset:   %02d:%02d\n", set_h, set_m);
	printf("location: %.2f°N, %.2f°E\n", latitude, longitude);
	printf("battery:  %s\n", on_battery() ? "yes (throttled)" : "no");
	printf("\nmonitors:\n");
	for (size_t i = 0; i < NUM_MONITORS; i++) {
		printf("  %s: theme=%s%s\n",
			monitors[i].name,
			monitors[i].theme,
			monitors[i].video ? " [video]" : "");
	}
}

static void
testmode(void)
{
	int stage = 0;
	struct sigaction sa;

	setbuf(stdout, NULL);

	sa.sa_handler = sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	printf("Test mode: cycling through all stages (%us each)\n", test_interval);
	printf("Press Ctrl+C to stop\n\n");

	while (running) {
		printf("[%d/%d] %s\n", stage + 1, (int)NUM_STAGES, stagenames[stage]);
		fflush(stdout);
		setwallpaper(stage);
		sleep(test_interval);
		stage = (stage + 1) % NUM_STAGES;
	}
	printf("\nTest complete.\n");
	kill_videos();
}

static void
usage(void)
{
	die("usage: dwall [-dhstv]\n");
}

int
main(int argc, char *argv[])
{
	int daemonize = 0;
	int status = 0;
	int test = 0;
	int i;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			usage();
		switch (argv[i][1]) {
		case 'd':
			daemonize = 1;
			break;
		case 's':
			status = 1;
			break;
		case 't':
			test = 1;
			break;
		case 'v':
			die("dwall-2.0\n");
			break;
		case 'h':
		default:
			usage();
		}
	}

	if (status) {
		printstatus();
		return 0;
	}

	if (test) {
		testmode();
		return 0;
	}

	if (daemonize) {
		struct sigaction sa;

		sa.sa_handler = sighandler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);

		while (running) {
			int stage = getstage();
			if (stage != laststage) {
				time_t now = time(NULL);
				struct tm *t = localtime(&now);
				printf("[%02d:%02d] %s%s\n",
					t->tm_hour, t->tm_min, stagenames[stage],
					on_battery() ? " (battery)" : "");
				setwallpaper(stage);
			}
			sleep(interval);
		}
		kill_videos();
	} else {
		int stage = getstage();
		printf("%s\n", stagenames[stage]);
		setwallpaper(stage);
	}

	return 0;
}
