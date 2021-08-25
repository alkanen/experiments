OUTPUT="${OUTPUT:-images/som.mp4}"
GLOB="${GLOB:-images/*.png}"
# Make sure both width and height are divisible by 2
DIMENSIONS="width=ceil(iw/2)*2:height=ceil(ih/2)*2"
# Pad the ending with a couple of seconds of the last frame so it doesn't just end
END_PADDING=stop_mode=clone:stop_duration=2
FPS="${FPS:-30}"
FRAMERATE="${FRAMREATE:-$FPS}"

ffmpeg -y -framerate $FRAMERATE -pattern_type glob  -i "$GLOB" -vf pad="$DIMENSIONS",tpad="$END_PADDING" -c:v libx264 -r $FPS -pix_fmt yuv420p "$OUTPUT"
