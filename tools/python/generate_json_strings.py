#!/usr/bin/env python3
"""
Flatten data/translations/*/<lang>.json/localize.json into data/*/<lang>.json/localize.json,
keeping only the defaultMessage of every key. Symlinked languages stay symlinks.
"""
import json
import os
import sys


def main():
    data_dir = sys.argv[1]
    translations = os.path.join(data_dir, 'translations')
    for root, _, files in os.walk(translations):
        for name in files:
            if name != 'localize.json':
                continue
            src = os.path.join(root, name)
            dst = os.path.join(data_dir, os.path.relpath(src, translations))
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            if os.path.lexists(dst):
                os.remove(dst)
            if os.path.islink(src):
                os.symlink(os.readlink(src), dst)
                continue
            with open(src, encoding='utf-8') as f:
                strings = json.load(f)
            with open(dst, 'w', encoding='utf-8') as f:
                json.dump({k: v.get('defaultMessage') for k, v in strings.items()}, f,
                          ensure_ascii=False, separators=(',', ':'))
                f.write('\n')


if __name__ == '__main__':
    main()
