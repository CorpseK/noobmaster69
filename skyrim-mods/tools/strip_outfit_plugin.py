"""Strip an outfit/armor mod plugin down to "the items exist + can be tempered".

Keeps (FormIDs unchanged, so SPID/SkyPatcher lines keep working):
  - every record that is NEW in this plugin (ARMO, ARMA, OTFT, KYWD, TXST, LVLI, ...),
  - COBJ records whose workbench is a tempering bench (armor table, sharpening wheel).
Removes:
  - COBJ crafting recipes (forge, tanning rack, skyforge, smelter... anything but tempering),
  - everything placed in the world: the whole CELL and WRLD groups (REFR/ACHR, chests, mannequins),
  - QUST/DIAL/INFO/SCEN (start-up quests that hand out items or recipe books, dialogue),
  - every OVERRIDE of a record from another plugin (leveled-list injection, NPC/outfit edits,
    merchant chests, ...), EXCEPT ARMO/ARMA/TXST/OTFT overrides of a non-vanilla plugin
    (3BA conversion plugins edit the original mod's armor this way).
Not handled (delete these files from the mod folder yourself): SPID *_DISTR.ini, KID *_KID.ini,
SkyPatcher ini files, Container Distribution Framework files, BOS *_SWAP.ini.

  py -I strip_outfit_plugin.py "<mod>/Plugin.esp" "<out folder>/Plugin.esp"
  py -I strip_outfit_plugin.py --dry-run "<mod>/Plugin.esp"

Put the output in its own MO2 mod, loaded below the original (same file name wins).
"""
import argparse
import collections
import struct
import sys
import zlib

TEMPER_BENCHES = {  # Skyrim.esm keywords
    0x000ADB78: "CraftingSmithingArmorTable",
    0x00088108: "CraftingSmithingSharpeningWheel",
}
VANILLA = {"skyrim.esm", "update.esm", "dawnguard.esm", "hearthfires.esm", "dragonborn.esm"}
# Overrides of these types are kept when they target a non-vanilla plugin: that is how
# "3BA conversion" plugins point the original mod's armor at the new meshes.
KEEP_MOD_OVERRIDE_TYPES = {"ARMO", "ARMA", "TXST", "OTFT"}
DROP_TOP_GROUPS = {"CELL", "WRLD", "QUST", "DIAL", "SCEN"}
DROP_NEW_TYPES = {"QUST", "DIAL", "INFO", "SCEN", "CELL", "WRLD", "REFR", "ACHR", "NAVM", "LAND"}


def subrecords(data):
    p = 0
    big = None
    while p + 6 <= len(data):
        t = data[p:p + 4].decode("latin1")
        n, = struct.unpack_from("<H", data, p + 4)
        p += 6
        if t == "XXXX":
            big, = struct.unpack_from("<I", data, p)
            p += 4
            continue
        if big is not None:
            n, big = big, None
        yield t, data[p:p + n]
        p += n


class Plugin:
    def __init__(self, raw):
        self.raw = raw
        if raw[:4] != b"TES4":
            raise ValueError("not a Skyrim plugin (no TES4 header)")
        hsize, = struct.unpack_from("<I", raw, 4)
        self.header_end = 24 + hsize
        self.masters = [s[:-1].decode("cp1252") for t, s in subrecords(raw[24:self.header_end]) if t == "MAST"]
        self.self_index = len(self.masters)

    def is_new(self, formid):
        return (formid >> 24) >= self.self_index

    def master_of(self, formid):
        i = formid >> 24
        return self.masters[i] if i < len(self.masters) else "<this plugin>"


def record_body(raw, p):
    size, flags = struct.unpack_from("<II", raw, p + 4)
    body = raw[p + 24:p + 24 + size]
    if flags & 0x40000:
        body = zlib.decompress(body[4:])
    return body


def is_temper_recipe(plugin, body):
    for t, s in subrecords(body):
        if t == "BNAM" and len(s) >= 4:
            bench, = struct.unpack_from("<I", s)
            return plugin.master_of(bench) == "Skyrim.esm" and (bench & 0xFFFFFF) in TEMPER_BENCHES
    return False


def walk(plugin, raw, start, end, stats, top=None):
    """Return rebuilt bytes for raw[start:end] (a sequence of records/groups)."""
    out = bytearray()
    p = start
    while p < end:
        t = raw[p:p + 4].decode("latin1")
        size, = struct.unpack_from("<I", raw, p + 4)
        if t == "GRUP":
            label = raw[p + 8:p + 12]
            gtype, = struct.unpack_from("<i", raw, p + 12)
            gname = label.decode("latin1") if gtype == 0 else top
            if gtype == 0 and gname in DROP_TOP_GROUPS:
                stats["dropped group " + gname] += 1
                p += size
                continue
            inner = walk(plugin, raw, p + 24, p + size, stats, gname)
            if inner or size == 24:  # keep groups that were already empty
                hdr = bytearray(raw[p:p + 24])
                struct.pack_into("<I", hdr, 4, 24 + len(inner))
                out += hdr + inner
            p += size
            continue
        formid, = struct.unpack_from("<I", raw, p + 12)
        rec = raw[p:p + 24 + size]
        p += 24 + size
        if not plugin.is_new(formid):
            master = plugin.master_of(formid)
            vanilla = master.lower() in VANILLA or master.lower().startswith("cc")
            if t in KEEP_MOD_OVERRIDE_TYPES and not vanilla:
                stats[f"kept override {t} ({master})"] += 1
                out += rec
                continue
            stats[f"dropped override {t} ({master})"] += 1
            continue
        if t in DROP_NEW_TYPES:
            stats[f"dropped new {t}"] += 1
            continue
        if t == "COBJ":
            if is_temper_recipe(plugin, record_body(raw, p - 24 - size)):
                stats["kept COBJ temper"] += 1
            else:
                stats["dropped COBJ crafting"] += 1
                continue
        else:
            stats[f"kept {t}"] += 1
        out += rec
    return bytes(out)


def strip(raw):
    plugin = Plugin(raw)
    stats = collections.Counter()
    body = walk(plugin, raw, plugin.header_end, len(raw), stats)
    header = bytearray(raw[:plugin.header_end])
    kept = sum(n for k, n in stats.items() if k.startswith("kept"))
    # HEDR record count (number of records + groups is what the CK writes; records is close enough
    # and the game does not rely on it)
    hp = 24
    while hp < plugin.header_end:
        t = header[hp:hp + 4]
        n, = struct.unpack_from("<H", header, hp + 4)
        if t == b"HEDR":
            struct.pack_into("<i", header, hp + 6 + 4, kept)
            break
        hp += 6 + n
    return bytes(header) + body, stats, plugin


def verify(raw):
    """Walk the whole file and check every GRUP/record size adds up. Returns record count."""
    hsize, = struct.unpack_from("<I", raw, 4)
    count = 0

    def go(start, end):
        nonlocal count
        p = start
        while p < end:
            t = raw[p:p + 4]
            size, = struct.unpack_from("<I", raw, p + 4)
            if t == b"GRUP":
                if size < 24 or p + size > end:
                    raise ValueError(f"bad GRUP size at {p}")
                go(p + 24, p + size)
                p += size
            else:
                if p + 24 + size > end:
                    raise ValueError(f"bad record size at {p}")
                count += 1
                p += 24 + size
        if p != end:
            raise ValueError("group overrun")
    go(24 + hsize, len(raw))
    return count


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("plugin")
    ap.add_argument("out", nargs="?")
    ap.add_argument("--dry-run", action="store_true", help="only print what would be removed")
    a = ap.parse_args()
    raw = open(a.plugin, "rb").read()
    result, stats, plugin = strip(raw)
    print(f"{a.plugin}  masters: {', '.join(plugin.masters) or '-'}")
    for k in sorted(stats):
        print(f"  {stats[k]:5d}  {k}")
    if not any(k.startswith("kept ARMO") for k in stats):
        print("  WARNING: no ARMO kept - is this really an outfit mod?")
    if a.dry_run or not a.out:
        return
    n = verify(result)
    open(a.out, "wb").write(result)
    print(f"  structure OK, {n} records")
    print(f"written {a.out} ({len(raw)} -> {len(result)} bytes)")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    main()
