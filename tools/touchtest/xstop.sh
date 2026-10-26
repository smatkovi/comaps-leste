#!/bin/bash
# Stops what xstart.sh started: the X server on :7 (its pid file) and vtouch.py.
T=$(cd "$(dirname "$0")" && pwd)
[ -p "$T/fifo" ] && timeout 2 sh -c "echo quit > '$T/fifo'" 2>/dev/null
if [ -f /tmp/.X7-lock ]; then
	pid=$(tr -d ' ' < /tmp/.X7-lock)
	[ -n "$pid" ] && ps -p "$pid" -o comm= | grep -qx Xorg && sudo -n kill "$pid"
fi
sleep 1
pgrep -a -u root -f "$T/vtouch.py" || true
