#!/usr/bin/env python3

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent

def main() -> int:
    for i in ROOT.glob('*.ui'):
        print(f'Processing {i.name} ...')

        raw = i.read_text(encoding='utf-8')

        reg = re.compile(r'>\s*<normaloff>.*?</iconset>', re.MULTILINE)
        raw = reg.sub('/>', raw)

        i.write_text(raw, encoding='utf-8', newline='\n')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
