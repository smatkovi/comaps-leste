#!/usr/bin/env python3
"""Gesture scripts for vtouch.py's FIFO (one command per line).

  staggered   first finger down, the second 300 ms later, only the second
              moves outwards, then it lifts first and the first finger pans
  together    both down in one frame, both move outwards, both lift
  zoomout     both down, both move inwards
"""
import sys

name = sys.argv[1]
cx, cy = 200, 450
out = []
step = lambda *cmds: out.extend(list(cmds) + ['sync', 'sleep 16'])
if name == 'staggered':
    step('down 0 150 %d' % cy)
    out.append('sleep 300')
    step('down 1 250 %d' % cy)
    for i in range(1, 21):
        step('move 1 %d %d' % (250 + 6 * i, cy))
    step('up 1')
    for i in range(1, 11):
        step('move 0 %d %d' % (150, cy - 10 * i))
    step('up 0')
elif name == 'together':
    step('down 0 %d %d' % (cx - 50, cy), 'down 1 %d %d' % (cx + 50, cy))
    for i in range(1, 21):
        step('move 0 %d %d' % (cx - 50 - 4 * i, cy), 'move 1 %d %d' % (cx + 50 + 4 * i, cy))
    step('up 0', 'up 1')
elif name == 'zoomout':
    step('down 0 %d %d' % (cx - 130, cy), 'down 1 %d %d' % (cx + 130, cy))
    for i in range(1, 21):
        step('move 0 %d %d' % (cx - 130 + 4 * i, cy), 'move 1 %d %d' % (cx + 130 - 4 * i, cy))
    step('up 0', 'up 1')
if name == 'drag':
    step('down 0 100 600')
    for i in range(1, 21):
        step('move 0 %d %d' % (100 + 10 * i, 600 - 10 * i))
    step('up 0')
elif name == 'tap':
    step('down 0 200 300')
    step('up 0')
print('\n'.join(out))
