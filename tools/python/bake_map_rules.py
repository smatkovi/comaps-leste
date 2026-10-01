#!/usr/bin/env python3
"""Applies Maps' own per-zoom line widths and place labels to compiled drawing rules.

A class (e.g. "highway-secondary-bridge") takes the first entry whose pattern matches its whole name.
Widths are interpolated between the keyed zooms and keep the nearest key's value outside them; labels take
the nearest key at or below the zoom (the lowest key below all keys).

Usage: PYTHONPATH=<dir with drules_struct_pb2> bake_map_rules.py <drules_proto_*.bin>...
"""
import re
import sys

import drules_struct_pb2

SCALE_COUNT = 21  # scales::UPPER_STYLE_SCALE + 1

# Class pattern: {zoom: line width factor}.
LINE_WIDTHS = {
    r'highway-(motorway|motorway_link)(-.*)?': {12: 0.5, 15: 1.0},
    r'highway-(trunk|trunk_link)(-.*)?': {12: 0.5, 15: 1.0},
    r'highway-(primary|primary_link)(-.*)?': {12: 0.75, 15: 1.0},
    r'highway-(secondary|secondary_link)(-.*)?': {12: 0.7, 15: 1.0},
}

# Class pattern: {zoom: caption shown}.
PLACE_LABELS = {
    r'place-suburb(-.*)?': {11: False, 12: True},
    r'place-quarter(-.*)?': {11: False, 12: True},
    r'place-neighbourhood(-.*)?': {11: False, 12: True},
}


def interpolate(keys, zoom):
    zooms = sorted(keys)
    if zoom <= zooms[0]:
        return keys[zooms[0]]
    if zoom >= zooms[-1]:
        return keys[zooms[-1]]
    upper = next(z for z in zooms if z >= zoom)
    lower = max(z for z in zooms if z < zoom)
    t = (zoom - lower) / (upper - lower)
    return keys[lower] + (keys[upper] - keys[lower]) * t


def step(keys, zoom):
    below = [z for z in keys if z <= zoom]
    return keys[max(below)] if below else keys[min(keys)]


def find(table, name):
    for pattern, keys in table:
        if pattern.fullmatch(name):
            return keys
    return None


def bake(path, widths, labels):
    cont = drules_struct_pb2.ContainerProto()
    with open(path, 'rb') as f:
        cont.ParseFromString(f.read())

    for ce in cont.cont:
        width_keys = find(widths, ce.name)
        label_keys = find(labels, ce.name)
        if width_keys is None and label_keys is None:
            continue
        for de in ce.element:
            if not 0 <= de.scale < SCALE_COUNT:
                continue
            if width_keys is not None:
                factor = interpolate(width_keys, de.scale)
                if factor != 1.0:
                    for line in de.lines:
                        line.width *= factor
            if label_keys is not None and not step(label_keys, de.scale):
                de.ClearField('caption')

    with open(path, 'wb') as f:
        f.write(cont.SerializeToString())


def main(paths):
    widths = [(re.compile(p), keys) for p, keys in LINE_WIDTHS.items()]
    labels = [(re.compile(p), keys) for p, keys in PLACE_LABELS.items()]
    for path in paths:
        bake(path, widths, labels)


if __name__ == '__main__':
    main(sys.argv[1:])
