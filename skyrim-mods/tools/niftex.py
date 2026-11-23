"""Minimal Skyrim SE NIF (20.2.0.7, BSVER 100) reader/writer for texture paths.

Reads the header (block types, block sizes, string table), finds every
BSShaderTextureSet / BSEffectShaderProperty texture path, links shapes to their
shader + texture set, and can rewrite BSShaderTextureSet paths (block sizes in
the header are updated, other blocks are copied byte for byte).
"""

import struct

TEXTURE_SLOTS = ["diffuse", "normal", "glow/skin", "height/parallax", "env", "envmask", "inner/multilayer", "backlight/specular", "slot8"]
SHAPE_TYPES = {"BSTriShape", "BSDynamicTriShape", "BSSubIndexTriShape", "BSMeshLODTriShape", "NiTriShape", "NiTriStrips"}


class Reader:
    def __init__(self, data, pos=0):
        self.d, self.p = data, pos

    def u8(self):
        v = self.d[self.p]; self.p += 1; return v

    def u16(self):
        v, = struct.unpack_from("<H", self.d, self.p); self.p += 2; return v

    def u32(self):
        v, = struct.unpack_from("<I", self.d, self.p); self.p += 4; return v

    def i32(self):
        v, = struct.unpack_from("<i", self.d, self.p); self.p += 4; return v

    def sized(self):
        n = self.u32(); s = self.d[self.p:self.p + n]; self.p += n
        return s.decode("cp1252", "replace")

    def short(self):
        n = self.u8(); s = self.d[self.p:self.p + n]; self.p += n
        return s.rstrip(b"\0").decode("cp1252", "replace")

    def skip(self, n):
        self.p += n


def sized_bytes(s):
    b = s.encode("cp1252")
    return struct.pack("<I", len(b)) + b


class Nif:
    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            data = f.read()
        nl = data.index(b"\n")
        self.header_line = data[:nl + 1]
        r = Reader(data, nl + 1)
        self.version = r.u32()
        if self.version != 0x14020007:
            raise ValueError(f"unsupported NIF version 0x{self.version:08X}")
        r.u8()  # endian
        self.user_version = r.u32()
        nblocks = r.u32()
        self.bsver = r.u32()
        r.short(); r.short(); r.short()  # author, process script, export script
        if self.bsver >= 103:
            r.short()
        ntypes = r.u16()
        self.block_types = [r.sized() for _ in range(ntypes)]
        self.type_index = [r.u16() & 0x7FFF for _ in range(nblocks)]
        self.sizes_pos = r.p
        self.sizes = [r.u32() for _ in range(nblocks)]
        nstr = r.u32(); r.u32()
        self.strings = [r.sized() for _ in range(nstr)]
        ngroups = r.u32()
        r.skip(4 * ngroups)
        self.prefix = data[:self.sizes_pos]
        self.after_sizes = data[self.sizes_pos + 4 * nblocks:r.p]
        self.blocks = []
        p = r.p
        for s in self.sizes:
            self.blocks.append(bytearray(data[p:p + s])); p += s
        self.footer = data[p:]

    def type_of(self, i):
        return self.block_types[self.type_index[i]]

    def string(self, idx):
        return self.strings[idx] if 0 <= idx < len(self.strings) else None

    # --- texture sets -------------------------------------------------------
    def texture_set(self, i):
        r = Reader(self.blocks[i])
        return [r.sized() for _ in range(r.i32())]

    def set_texture_set(self, i, paths):
        out = struct.pack("<i", len(paths)) + b"".join(sized_bytes(p) for p in paths)
        self.blocks[i] = bytearray(out)
        self.sizes[i] = len(out)

    def effect_textures(self, i):
        """BSEffectShaderProperty (SSE): source + greyscale texture."""
        r = Reader(self.blocks[i])
        r.skip(4)  # name
        r.skip(4 * r.u32())  # extra data refs
        r.skip(4)  # controller
        r.skip(4 + 4 + 8 + 8)  # flags1, flags2, uv offset, uv scale
        src = r.sized()
        r.skip(4 + 16 + 16 + 4 + 4)  # clamp/lighting/lod/unused, falloff, emissive color, mult, soft depth
        grey = r.sized()
        return {"source": src, "greyscale": grey}

    # --- links --------------------------------------------------------------
    def _net(self, r, has_shader_type=False):
        if has_shader_type:
            r.skip(4)
        name = self.string(r.i32())
        r.skip(4 * r.u32())
        r.skip(4)
        return name

    def shape_info(self, i):
        t = self.type_of(i)
        r = Reader(self.blocks[i])
        name = self._net(r)
        if t in ("BSTriShape", "BSDynamicTriShape", "BSSubIndexTriShape", "BSMeshLODTriShape"):
            r.skip(4 + 12 + 36 + 4 + 4)  # flags, translation, rotation, scale, collision
            r.skip(16 + 4)  # bounding sphere, skin
            shader = r.i32()
            return name, shader
        return name, None

    def shader_texture_set(self, i):
        if self.type_of(i) != "BSLightingShaderProperty":
            return None
        r = Reader(self.blocks[i])
        self._net(r, has_shader_type=True)
        r.skip(4 + 4 + 8 + 8)
        ref = r.i32()
        if 0 <= ref < len(self.blocks) and self.type_of(ref) == "BSShaderTextureSet":
            return ref
        return None

    def report(self):
        """Yield (shape_name, kind, slot, path, block_index)."""
        seen = set()
        for i, _ in enumerate(self.blocks):
            if self.type_of(i) in SHAPE_TYPES:
                name, shader = self.shape_info(i)
                if shader is None or not (0 <= shader < len(self.blocks)):
                    continue
                st = self.type_of(shader)
                if st == "BSLightingShaderProperty":
                    ts = self.shader_texture_set(shader)
                    if ts is not None:
                        seen.add(ts)
                        for slot, p in enumerate(self.texture_set(ts)):
                            if p:
                                yield name, "lighting", TEXTURE_SLOTS[slot] if slot < len(TEXTURE_SLOTS) else str(slot), p, ts
                elif st == "BSEffectShaderProperty":
                    seen.add(shader)
                    for slot, p in self.effect_textures(shader).items():
                        if p:
                            yield name, "effect", slot, p, shader
        for i, _ in enumerate(self.blocks):
            if i in seen:
                continue
            t = self.type_of(i)
            if t == "BSShaderTextureSet":
                for slot, p in enumerate(self.texture_set(i)):
                    if p:
                        yield None, "lighting", TEXTURE_SLOTS[slot] if slot < len(TEXTURE_SLOTS) else str(slot), p, i
            elif t == "BSEffectShaderProperty":
                for slot, p in self.effect_textures(i).items():
                    if p:
                        yield None, "effect", slot, p, i

    def shape_names(self):
        return [self.shape_info(i)[0] for i in range(len(self.blocks)) if self.type_of(i) in SHAPE_TYPES]

    def save(self, path):
        with open(path, "wb") as f:
            f.write(self.prefix)
            f.write(b"".join(struct.pack("<I", s) for s in self.sizes))
            f.write(self.after_sizes)
            for b in self.blocks:
                f.write(b)
            f.write(self.footer)
