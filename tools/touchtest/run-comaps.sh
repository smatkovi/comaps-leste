#!/bin/bash
# CoMaps from the native build on the test X server, with an isolated
# config/data dir and its touch log on stderr
T=$(cd "$(dirname "$0")" && pwd)
cd "${COMAPS_BUILD:?set COMAPS_BUILD to the build directory with the comaps binary}"
exec env XAUTHORITY=$T/xauth DISPLAY=:7 QT_QPA_PLATFORM=xcb \
	XDG_CONFIG_HOME=$T/home/config XDG_DATA_HOME=$T/home/data XDG_CACHE_HOME=$T/home/cache \
	COMAPS_TOUCH_DEBUG=1 COMAPS_GPSD=${COMAPS_GPSD:-off} QT_FORCE_STDERR_LOGGING=1 ./comaps "$@"
