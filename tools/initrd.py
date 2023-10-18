"""Initrd filesystem image generator"""

import sys
import struct
import glob
from pathlib import Path

OUTPUT_FILE='initrd.img'
PATH_LENGTH=60
FILES = [str(v.as_posix()) for v in Path(sys.argv[1]).glob('**/*.*')]

class Header:
    def __init__(self, filepath, length, offset):
        self.filepath = filepath
        self.length = length
        self.offset = offset
    def pack(self, base_offset: int):
        return struct.pack(f'={PATH_LENGTH}sLL', bytes(self.filepath[:PATH_LENGTH-1], encoding='ascii'), self.length, self.offset + base_offset)

with open(OUTPUT_FILE, 'wb') as out:
    headers = []
    offset = 0
    data = b''
    for p in FILES:
        if len(p) > PATH_LENGTH:
            print(f'{p}: maximum path length (60) is exceeded, skipping')
            continue
        with open(p, 'rb') as f:
            contents = f.read()
            data += contents
            p = '/' + p[len('initrd/'):]
            headers.append(Header(p, len(contents), offset))
            print(p)
            offset += len(contents)
    print(f'Total amount (in bytes): {len(data)}')
    out.write(struct.pack('H', len(headers)))
    # base_offset = len(headers) * (PATH_LENGTH + 2 + 2) + 2
    for h in headers:
        # print(h.pack())
        out.write(h.pack(0))

    out.write(data)
