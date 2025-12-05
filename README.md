# dwall - Dynamic Wallpaper for Linux

A suckless-style dynamic wallpaper daemon that changes your wallpaper based on the time of day, with smooth fade transitions. Inspired by macOS dynamic wallpapers.

## Features

- **Sun-based timing** - Wallpaper changes based on actual sunrise/sunset for your location
- **Smooth fade transitions** - Pre-generated blend frames for seamless transitions
- **Multi-monitor support** - Different themes per monitor
- **Video wallpaper support** - Use animated wallpapers via mpvpaper
- **Battery throttling** - Automatically reduces effects when on battery
- **Suckless configuration** - All settings in config.h, rebuild to apply

## Dependencies

- feh (wallpaper setter)
- ImageMagick (transition frame generation)
- Optional: mpvpaper or xwinwrap (video wallpapers)
- Optional: timewall (HEIC extraction)

## Installation

```bash
# Edit config.h with your settings
vim config.h

# Build
make

# Install (optional)
sudo make install
```

## Usage

```bash
# Set wallpaper once based on current time
dwall

# Run as daemon (recommended)
dwall -d

# Test mode - cycle through all stages
dwall -t

# Show status
dwall -s

# Show version
dwall -v
```

## Configuration

Edit `config.h` before building:

```c
/* Your location for sun calculations */
static const double latitude  = 30.0;   /* degrees N */
static const double longitude = 31.0;   /* degrees E */

/* Monitor configurations */
static struct monitor_config monitors[] = {
    { "DP-1",    "catalina", NULL },        /* static theme */
    { "DP-2",    "monterey", NULL },        /* different theme */
    { "DVI-I-1", "catalina", "~/vid.mp4" }, /* video wallpaper */
};
```

## Setting Up Themes

1. **Extract from macOS HEIC file:**
   ```bash
   timewall unpack "macOS Monterey.heic" /tmp/extract
   ```

2. **Create theme directory:**
   ```bash
   mkdir -p ~/.config/dynamic-wallpaper/themes/mytheme
   ```

3. **Map frames to stages** (01_night.jpg through 10_early_night.jpg):
   ```bash
   magick /tmp/extract/1.png -quality 90 ~/.config/dynamic-wallpaper/themes/mytheme/01_night.jpg
   # ... map remaining frames based on HEIC properties.xml timestamps
   ```

4. **Generate transition frames:**
   ```bash
   ./gen-transitions.sh mytheme
   ```

5. **Update config.h and rebuild**

## Directory Structure

```
~/.config/dynamic-wallpaper/
├── themes/
│   ├── catalina/
│   │   ├── 01_night.jpg
│   │   ├── 02_late_night.jpg
│   │   └── ...
│   └── monterey/
│       └── ...
└── cache/
    ├── catalina/       # transition frames
    └── monterey/
```

## Stages

| Stage | Time Period |
|-------|-------------|
| night | 00:00 - 02:00 |
| late_night | 02:00 - 04:00 |
| dawn | 04:00 - sunrise |
| sunrise | sunrise - sunrise+30m |
| morning | sunrise+30m - noon-1h |
| noon | noon-1h - noon+1h |
| afternoon | noon+1h - sunset-30m |
| sunset | sunset-30m - sunset+30m |
| dusk | sunset+30m - sunset+90m |
| early_night | sunset+90m - 00:00 |

## Autostart

Add to your `~/.xinitrc` or window manager autostart:

```bash
dwall -d &
```

## License

MIT License - See LICENSE file
