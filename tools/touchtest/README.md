# Two-finger test bench

Real multi-touch on the PC, through the same path as on a phone running X11:
a virtual touchscreen (uinput, `vtouch.py`) is the only input of an X server
of its own on `:7` (dummy video driver, xf86-input-libinput), which delivers
XInput 2.2 touch events to the app.

Needs root through `sudo -n` (uinput, Xorg), python-evdev, xf86-video-dummy,
xf86-input-libinput, xdpyinfo, xinput, xwd and ImageMagick.

    ./xstart.sh                       # X server :7 with the virtual touchscreen
    COMAPS_BUILD=/path/to/build ./run-comaps.sh > comaps.log 2>&1 &
    ./do.sh staggered s1              # gesture, screenshots shot-s1.png, log touch-s1.log
    ./xstop.sh

Gestures (`gesture.py`): `staggered` (one finger, the second 300 ms later and
only it moves, then it lifts and the first one pans), `together`, `zoomout`,
`drag`, `tap`. `run-comaps.sh` sets `COMAPS_TOUCH_DEBUG=1`: every Qt touch
event and what drape gets is in the log.
