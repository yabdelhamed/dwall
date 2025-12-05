#!/bin/sh
# Generate transition frames between wallpapers for smooth fading
# Usage: gen-transitions.sh [theme]
# If no theme specified, generates for all themes

BASEDIR="$HOME/.config/dynamic-wallpaper"
THEMEDIR="$BASEDIR/themes"
CACHEDIR="$BASEDIR/cache"
STEPS=10  # frames per transition

WALLS="01_night 02_late_night 03_dawn 04_sunrise 05_morning 06_noon 07_afternoon 08_sunset 09_dusk 10_early_night"

# Find file extension
find_wall() {
    dir="$1"
    name="$2"
    for ext in jpg jpeg png webp; do
        [ -f "$dir/$name.$ext" ] && echo "$dir/$name.$ext" && return
    done
}

# Generate transitions for a theme
gen_theme() {
    theme="$1"
    imgdir="$THEMEDIR/$theme"
    cachedir="$CACHEDIR/$theme"

    if [ ! -d "$imgdir" ]; then
        echo "Theme not found: $theme"
        return 1
    fi

    echo "=== Generating transitions for theme: $theme ==="
    mkdir -p "$cachedir"

    prev=""
    prevname=""
    for name in $WALLS; do
        curr=$(find_wall "$imgdir" "$name")
        if [ -n "$prev" ] && [ -n "$curr" ]; then
            echo "  $prevname -> $name"
            for i in $(seq 1 $STEPS); do
                pct=$((i * 100 / STEPS))
                out="$cachedir/${prevname}_to_${name}_$(printf '%02d' $i).jpg"
                if [ ! -f "$out" ]; then
                    magick "$prev" "$curr" -gravity center -compose blend \
                        -define compose:args=$pct -composite -quality 85 "$out"
                fi
            done
        fi
        prev="$curr"
        prevname="$name"
    done

    # Wrap-around transition (last -> first)
    first=$(find_wall "$imgdir" "01_night")
    if [ -n "$prev" ] && [ -n "$first" ]; then
        echo "  $prevname -> 01_night"
        for i in $(seq 1 $STEPS); do
            pct=$((i * 100 / STEPS))
            out="$cachedir/${prevname}_to_01_night_$(printf '%02d' $i).jpg"
            if [ ! -f "$out" ]; then
                magick "$prev" "$first" -gravity center -compose blend \
                    -define compose:args=$pct -composite -quality 85 "$out"
            fi
        done
    fi

    echo "  Done: $(ls -1 "$cachedir" 2>/dev/null | wc -l) frames"
}

# Main
if [ -n "$1" ]; then
    # Generate for specific theme
    gen_theme "$1"
else
    # Generate for all themes
    if [ ! -d "$THEMEDIR" ]; then
        echo "No themes found in $THEMEDIR"
        exit 1
    fi

    for dir in "$THEMEDIR"/*/; do
        theme=$(basename "$dir")
        gen_theme "$theme"
    done
fi

echo ""
echo "All transitions cached in: $CACHEDIR"
