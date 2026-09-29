#!/usr/bin/env python3
"""Draws the dark style's map icons like the light style's, with a dark style halo.

The dark icons are darkened or redrawn versions of the light ones, which makes them look muted
on the map. Every dark icon that has a light counterpart becomes that light icon, except for
its halo: the white, translucent shapes filled or outlined behind the icon. The halo takes the
colour and opacity of the dark icon's halo, or black at 60% when the dark icon has none, so it
doesn't glow on a dark map. Icons that only the dark style has stay as they are.

Usage: brighten_dark_symbols.py <light symbols dir> <dark symbols dir>
"""
import os
import re
import sys

SHAPE = re.compile(r'<(?:path|rect|circle|ellipse|polygon|g)\b[^>]*>')
PAINT = re.compile(r'\b(fill|stroke)="([^"]*)"')
OPACITY = re.compile(r'\bopacity="([^"]*)"')
WHITE = {'#fff', '#ffffff', 'white'}
DARK = {'#000', '#000000', '#080808', '#111', '#111111', 'black'}
DEFAULT_HALO = ('#000', '.6')


def is_halo(tag, colours):
    """A translucent shape filled or outlined in one of these colours."""
    return OPACITY.search(tag) is not None and any(
        value.lower() in colours for _, value in PAINT.findall(tag))


def brighten(light, dark):
    halos = [m for m in SHAPE.finditer(light) if is_halo(m.group(0), WHITE)]
    if not halos:
        return light
    dark_halo = next((m.group(0) for m in SHAPE.finditer(dark) if is_halo(m.group(0), WHITE | DARK)), None)
    if dark_halo:
        colour = next(v for _, v in PAINT.findall(dark_halo) if v.lower() in WHITE | DARK)
        opacity = OPACITY.search(dark_halo).group(1)
    else:
        colour, opacity = DEFAULT_HALO

    def recolour(tag):
        tag = PAINT.sub(lambda p: f'{p.group(1)}="{colour}"' if p.group(2).lower() in WHITE else p.group(0), tag)
        return OPACITY.sub(f'opacity="{opacity}"', tag, count=1)

    out, last = [], 0
    for m in halos:
        out += [light[last:m.start()], recolour(m.group(0))]
        last = m.end()
    return ''.join(out) + light[last:]


def main(light_dir, dark_dir):
    replaced = dark_only = 0
    for name in sorted(os.listdir(dark_dir)):
        light_path = os.path.join(light_dir, name)
        dark_path = os.path.join(dark_dir, name)
        if not name.endswith('.svg'):
            continue
        if not os.path.isfile(light_path):
            dark_only += 1
            continue
        with open(light_path, encoding='utf-8') as f:
            light = f.read()
        with open(dark_path, encoding='utf-8') as f:
            dark = f.read()
        result = brighten(light, dark)
        if result != dark:
            with open(dark_path, 'w', encoding='utf-8') as f:
                f.write(result)
            replaced += 1
    print(f'Dark icons drawn like the light ones: {replaced}, only in the dark style: {dark_only}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
