#!/usr/bin/env python3
"""Gives the dark style's map icons the colours of the light style's, keeping their dark halo.

The dark icons are the light ones with every colour darkened, which makes them look muted on the
map. Each colour of a dark icon becomes the light icon's colour at the same place, except where
white turns (near) black: that is the halo around the icon, which has to stay dark on a dark map.
Everything else of the dark icon, such as the halo's opacity, stays. An icon whose light and dark
files don't have the same number of colours stays as it is.

Usage: brighten_dark_symbols.py <light symbols dir> <dark symbols dir>
"""
import os
import re
import sys

COLOUR = re.compile(r'#[0-9A-Fa-f]{6}\b|#[0-9A-Fa-f]{3}\b')
WHITE = {'#fff', '#ffffff'}
HALO_DARK = {'#000', '#000000', '#080808', '#111', '#111111'}


def brighten(light, dark):
    light_colours = COLOUR.findall(light)
    dark_colours = COLOUR.findall(dark)
    if len(light_colours) != len(dark_colours):
        return None
    colours = iter([d if l.lower() in WHITE and d.lower() in HALO_DARK else l
                    for l, d in zip(light_colours, dark_colours)])
    return COLOUR.sub(lambda _: next(colours), dark)


def main(light_dir, dark_dir):
    replaced = kept = 0
    for name in sorted(os.listdir(dark_dir)):
        light_path = os.path.join(light_dir, name)
        dark_path = os.path.join(dark_dir, name)
        if not name.endswith('.svg') or not os.path.isfile(light_path):
            continue
        with open(light_path, encoding='utf-8') as f:
            light = f.read()
        with open(dark_path, encoding='utf-8') as f:
            dark = f.read()
        result = brighten(light, dark)
        if result is None:
            kept += 1
            continue
        if result != dark:
            with open(dark_path, 'w', encoding='utf-8') as f:
                f.write(result)
            replaced += 1
    print(f'Dark icons with light colours: {replaced}, kept: {kept}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
