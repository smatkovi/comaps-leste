#!/usr/bin/env python3
"""A virtual multi-touch screen (uinput) for testing two-finger gestures in an
X server of its own. Creates the device, writes its event node to argv[2],
then reads gesture commands from the FIFO argv[1], one per line:

  pinch CX CY D0 D1 STEPS MS   two fingers at CX+-D0, moving to CX+-D1 (horizontal)
  drag X0 Y0 X1 Y1 STEPS MS    one finger
  quit
"""
import os
import sys
import time

from evdev import AbsInfo, UInput, ecodes as e

W, H = 1920, 1080
cap = {
    e.EV_KEY: [e.BTN_TOUCH],
    e.EV_ABS: [
        (e.ABS_X, AbsInfo(0, 0, W - 1, 0, 0, 10)),
        (e.ABS_Y, AbsInfo(0, 0, H - 1, 0, 0, 10)),
        (e.ABS_MT_SLOT, AbsInfo(0, 0, 9, 0, 0, 0)),
        (e.ABS_MT_TRACKING_ID, AbsInfo(0, 0, 65535, 0, 0, 0)),
        (e.ABS_MT_POSITION_X, AbsInfo(0, 0, W - 1, 0, 0, 10)),
        (e.ABS_MT_POSITION_Y, AbsInfo(0, 0, H - 1, 0, 0, 10)),
    ],
}
ui = UInput(cap, name='vtouch test touchscreen', input_props=[e.INPUT_PROP_DIRECT])
tid = 100


def finger(slot, x, y, down=None):
    global tid
    ui.write(e.EV_ABS, e.ABS_MT_SLOT, slot)
    if down is True:
        tid += 1
        ui.write(e.EV_ABS, e.ABS_MT_TRACKING_ID, tid)
    if down is False:
        ui.write(e.EV_ABS, e.ABS_MT_TRACKING_ID, -1)
        return
    ui.write(e.EV_ABS, e.ABS_MT_POSITION_X, int(x))
    ui.write(e.EV_ABS, e.ABS_MT_POSITION_Y, int(y))


def frame(x, y, touching=True, key=None):
    if key is not None:
        ui.write(e.EV_KEY, e.BTN_TOUCH, key)
    ui.write(e.EV_ABS, e.ABS_X, int(x))
    ui.write(e.EV_ABS, e.ABS_Y, int(y))
    ui.syn()


def pinch(cx, cy, d0, d1, steps, ms):
    finger(0, cx - d0, cy, True)
    finger(1, cx + d0, cy, True)
    frame(cx - d0, cy, key=1)
    for i in range(1, steps + 1):
        d = d0 + (d1 - d0) * i / steps
        time.sleep(ms / 1000)
        finger(0, cx - d, cy)
        finger(1, cx + d, cy)
        frame(cx - d, cy)
    time.sleep(ms / 1000)
    finger(0, 0, 0, False)
    finger(1, 0, 0, False)
    frame(cx, cy, key=0)


def drag(x0, y0, x1, y1, steps, ms):
    finger(0, x0, y0, True)
    frame(x0, y0, key=1)
    for i in range(1, steps + 1):
        time.sleep(ms / 1000)
        x = x0 + (x1 - x0) * i / steps
        y = y0 + (y1 - y0) * i / steps
        finger(0, x, y)
        frame(x, y)
    time.sleep(ms / 1000)
    finger(0, 0, 0, False)
    frame(x1, y1, key=0)


fifo, node_file = sys.argv[1], sys.argv[2]
time.sleep(0.5)
with open(node_file, 'w') as f:
    f.write(ui.device.path + '\n')
os.chmod(node_file, 0o644)
while True:
    with open(fifo) as f:
        for line in f:
            w = line.split()
            if not w:
                continue
            if w[0] == 'quit':
                ui.close()
                sys.exit(0)
            args = [float(a) for a in w[1:]]
            # single steps: down SLOT X Y / move SLOT X Y / up SLOT, then
            # sync (one evdev frame) and sleep MS
            if w[0] == 'down':
                finger(int(args[0]), args[1], args[2], True)
                continue
            if w[0] == 'move':
                finger(int(args[0]), args[1], args[2])
                continue
            if w[0] == 'up':
                finger(int(args[0]), 0, 0, False)
                continue
            if w[0] == 'sync':
                ui.syn()
                continue
            if w[0] == 'sleep':
                time.sleep(args[0] / 1000)
                continue
            if w[0] == 'pinch':
                pinch(*args[:4], int(args[4]), args[5])
            elif w[0] == 'drag':
                drag(*args[:4], int(args[4]), args[5])
            print('done', line.strip(), flush=True)
