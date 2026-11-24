"""Minimal Skyrim SE BSA (v105) lister/extractor. Usage:
  py -I bsa.py list <bsa> <regex>
  py -I bsa.py extract <bsa> <regex> <outdir>
"""
import re
import struct
import sys
import os


def lz4_block(src, out):
    i = 0
    n = len(src)
    while i < n:
        tok = src[i]; i += 1
        lit = tok >> 4
        if lit == 15:
            while True:
                b = src[i]; i += 1
                lit += b
                if b != 255:
                    break
        out += src[i:i + lit]; i += lit
        if i >= n:
            break
        off = src[i] | (src[i + 1] << 8); i += 2
        ml = tok & 15
        if ml == 15:
            while True:
                b = src[i]; i += 1
                ml += b
                if b != 255:
                    break
        ml += 4
        start = len(out) - off
        for k in range(ml):
            out.append(out[start + k])
    return out


def lz4_frame(data):
    assert struct.unpack_from('<I', data, 0)[0] == 0x184D2204, 'not lz4 frame'
    flg = data[4]
    i = 6  # magic, FLG, BD
    if flg & 0x08:
        i += 8  # content size
    if flg & 0x01:
        i += 4  # dict id
    i += 1  # header checksum
    out = bytearray()
    while True:
        bs = struct.unpack_from('<I', data, i)[0]; i += 4
        if bs == 0:
            break
        size = bs & 0x7FFFFFFF
        blk = data[i:i + size]; i += size
        if bs & 0x80000000:
            out += blk
        else:
            lz4_block(blk, out)
        if flg & 0x10:
            i += 4
    return bytes(out)


def read_bsa(path):
    f = open(path, 'rb')
    hdr = f.read(36)
    magic, ver, off, flags, nfold, nfile, lfold, lfile, ftype = struct.unpack('<4sIIIIIIII', hdr)
    assert magic == b'BSA\0' and ver == 105, (magic, ver)
    compressed = bool(flags & 0x4)
    embed = bool(flags & 0x100)
    folders = [struct.unpack('<QIIQ', f.read(24)) for _ in range(nfold)]
    entries = []
    for _h, count, _p, _o in folders:
        ln = f.read(1)[0]
        fname = f.read(ln)[:-1].decode('cp1252')
        for _ in range(count):
            _fh, size, foff = struct.unpack('<QII', f.read(16))
            entries.append([fname, None, size, foff])
    names = f.read(lfile).split(b'\0')
    for e, nm in zip(entries, names):
        e[1] = nm.decode('cp1252')
    return f, compressed, embed, entries


def get(f, compressed, embed, e):
    folder, name, size, off = e
    comp = compressed ^ bool(size & 0x40000000)
    size &= 0x3FFFFFFF
    f.seek(off)
    if embed:
        ln = f.read(1)[0]
        f.read(ln)
        size -= ln + 1
    if comp:
        f.read(4)
        return lz4_frame(f.read(size - 4))
    return f.read(size)


def main():
    cmd, path, pat = sys.argv[1:4]
    rx = re.compile(pat, re.I)
    f, c, em, entries = read_bsa(path)
    for e in entries:
        full = e[0] + '\\' + e[1]
        if not rx.search(full):
            continue
        if cmd == 'list':
            print(full, e[2] & 0x3FFFFFFF)
        else:
            dst = os.path.join(sys.argv[4], *full.split('\\'))
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            with open(dst, 'wb') as o:
                o.write(get(f, c, em, e))
            print('extracted', full)


main()
