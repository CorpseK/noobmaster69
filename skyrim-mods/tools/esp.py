"""Minimal TES4/ESP reader: masters, flags and records (handles compressed records)."""
import struct, zlib
def subrecords(data):
    p=0; big=None
    while p+6<=len(data):
        t=data[p:p+4].decode('latin1'); n,=struct.unpack_from('<H',data,p+4); p+=6
        if t=='XXXX': big,=struct.unpack_from('<I',data,p); p+=4; continue
        if big is not None: n=big; big=None
        yield t,data[p:p+n]; p+=n
def records(data):
    p=0
    while p<len(data):
        t=data[p:p+4].decode('latin1'); size,=struct.unpack_from('<I',data,p+4)
        if t=='GRUP':
            yield from records(data[p+24:p+size]); p+=size; continue
        flags,fid=struct.unpack_from('<II',data,p+8); body=data[p+24:p+24+size]
        if flags & 0x40000: body=zlib.decompress(body[4:])
        yield t,fid,flags,body; p+=24+size
def load(path):
    data=open(path,'rb').read()
    hsize,=struct.unpack_from('<I',data,4)
    head=data[24:24+hsize]
    masters=[s[:-1].decode('cp1252') for t,s in subrecords(head) if t=='MAST']
    flags,=struct.unpack_from('<I',data,8)
    return masters, flags, list(records(data[24+hsize:]))
