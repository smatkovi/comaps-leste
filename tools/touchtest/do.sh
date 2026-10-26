#!/bin/bash
# do.sh GESTURE TAG: screenshot, gesture, screenshot (shot-TAG-a/b.png), and
# the touch log lines of the gesture (touch-TAG.log)
T=$(cd "$(dirname "$0")" && pwd)
export XAUTHORITY=$T/xauth DISPLAY=:7
xwd -root -silent | magick xwd:- -crop 420x820+0+0 -resize 50% "$T/shot-$2-a.png"
n0=$(wc -l < "$T/comaps.log")
python3 -I "$T/gesture.py" "$1" > "$T/fifo"
sleep 3
xwd -root -silent | magick xwd:- -crop 420x820+0+0 -resize 50% "$T/shot-$2-b.png"
tail -n +$((n0 + 1)) "$T/comaps.log" | grep -E "touch qt|drape" > "$T/touch-$2.log"
magick "$T/shot-$2-a.png" "$T/shot-$2-b.png" +append "$T/shot-$2.png"
wc -l < "$T/touch-$2.log"
