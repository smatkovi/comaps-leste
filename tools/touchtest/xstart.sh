#!/bin/bash
# An X server of its own on :7 (dummy video, 1920x1080) whose only input is
# the virtual touchscreen of vtouch.py, through libinput as on the phones.
# No other device is added (AutoAddDevices off, an empty config dir), and it
# does not switch VTs. Stop: xstop.sh
set -euo pipefail
T=$(cd "$(dirname "$0")" && pwd)
D=:7
rm -f "$T/fifo" "$T/node"
mkfifo "$T/fifo"
chmod 600 "$T/fifo"
sudo -n python3 -I "$T/vtouch.py" "$T/fifo" "$T/node" > "$T/vtouch.log" 2>&1 &
for i in $(seq 1 20); do [ -s "$T/node" ] && break; sleep 0.25; done
NODE=$(cat "$T/node")
echo "touch device: $NODE"
mkdir -p "$T/empty-confdir"
cat > "$T/xorg.conf" <<EOF
Section "ServerFlags"
	Option "AutoAddDevices" "false"
	Option "AutoEnableDevices" "false"
	Option "DontVTSwitch" "true"
	Option "AllowEmptyInput" "true"
EndSection
Section "Device"
	Identifier "dummy"
	Driver "dummy"
	VideoRam 256000
EndSection
Section "Monitor"
	Identifier "monitor"
	HorizSync 5.0-1000.0
	VertRefresh 5.0-200.0
	Modeline "1920x1080" 148.50 1920 2008 2052 2200 1080 1084 1089 1125 +hsync +vsync
EndSection
Section "Screen"
	Identifier "screen"
	Device "dummy"
	Monitor "monitor"
	DefaultDepth 24
	SubSection "Display"
		Depth 24
		Modes "1920x1080"
	EndSubSection
EndSection
Section "InputDevice"
	Identifier "touch"
	Driver "libinput"
	Option "Device" "$NODE"
EndSection
Section "ServerLayout"
	Identifier "layout"
	Screen "screen"
	InputDevice "touch"
EndSection
EOF
rm -f "$T/xauth"
touch "$T/xauth"
xauth -f "$T/xauth" add $D . "$(mcookie)"
sudo -n Xorg $D -config "$T/xorg.conf" -configdir "$T/empty-confdir" \
	-logfile "$T/Xorg.log" -auth "$T/xauth" -nolisten tcp -noreset \
	-sharevts -novtswitch > "$T/Xorg.out" 2>&1 &
for i in $(seq 1 40); do
	XAUTHORITY="$T/xauth" DISPLAY=$D xdpyinfo > /dev/null 2>&1 && break
	sleep 0.25
done
XAUTHORITY="$T/xauth" DISPLAY=$D xinput list
