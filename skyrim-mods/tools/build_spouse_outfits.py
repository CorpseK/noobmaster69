"""Build SpouseOutfits.esp (ESL-flagged, only OTFT records) and SpouseOutfits_DISTR.ini (SPID)
from skyrim-mods/SpouseOutfits/outfits.json.

Every reference in the JSON is checked against the real plugin found through MO2 (enabled mods,
highest priority wins, then the game's Data folder): it must exist, outfit pieces must be ARMO,
items must be WEAP/ARMO/AMMO, and no outfit piece may take a head/hair slot (30, 31, 41).

  py -I build_spouse_outfits.py --mo2 "E:\\Skyrim Modding\\MO2" --data "<game>\\Data" --out "<folder>"
  (--check only validates and prints, writes nothing)
"""
import argparse
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import esp  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(HERE, "..", "SpouseOutfits", "outfits.json")
VANILLA = ["Skyrim.esm", "Update.esm", "Dawnguard.esm", "HearthFires.esm", "Dragonborn.esm"]
HAIR_SLOTS = {30, 31, 41}


def find_plugins(mo2, profile, data):
    """name.lower() -> path, MO2 priority (modlist.txt top line = highest priority)."""
    found = {}
    ml = os.path.join(mo2, "profiles", profile, "modlist.txt")
    for line in open(ml, encoding="utf-8"):
        line = line.rstrip("\r\n")
        if not line.startswith("+"):
            continue
        d = os.path.join(mo2, "mods", line[1:])
        if not os.path.isdir(d):
            continue
        for f in os.listdir(d):
            if f.lower().endswith((".esp", ".esm", ".esl")) and f.lower() not in found:
                found[f.lower()] = os.path.join(d, f)
    for f in os.listdir(data):
        if f.lower().endswith((".esp", ".esm", ".esl")) and f.lower() not in found:
            found[f.lower()] = os.path.join(data, f)
    return found


def load_order(mo2, profile):
    order = [v.lower() for v in VANILLA]
    p = os.path.join(mo2, "profiles", profile, "plugins.txt")
    for line in open(p, encoding="utf-8", errors="replace"):
        line = line.strip()
        if line and not line.startswith("#"):
            order.append(line.lstrip("*").lower())
    return order


def top_groups(raw, p, labels):
    """Records of the wanted top-level groups only (Skyrim.esm is large)."""
    while p < len(raw):
        size, = struct.unpack_from("<I", raw, p + 4)
        if raw[p:p + 4] == b"GRUP" and raw[p + 8:p + 12].decode("latin1") in labels:
            yield from esp.records(raw[p + 24:p + size])
        p += size if raw[p:p + 4] == b"GRUP" else 24 + size


class Index:
    """Records defined (new) in each referenced plugin: local id -> (type, edid, slots)."""

    def __init__(self, paths):
        self.paths = paths
        self.cache = {}

    def get(self, plugin):
        key = plugin.lower()
        if key not in self.cache:
            if key not in self.paths:
                raise SystemExit(f"plugin not found in MO2 or Data: {plugin}")
            raw = open(self.paths[key], "rb").read()
            hsize, = struct.unpack_from("<I", raw, 4)
            n = sum(1 for t, _ in esp.subrecords(raw[24:24 + hsize]) if t == "MAST")
            table = {}
            for t, fid, fl, body in top_groups(raw, 24 + hsize, {"ARMO", "WEAP", "AMMO", "NPC_", "OTFT"}):
                if (fid >> 24) < n:
                    continue
                sub = {}
                for st, d in esp.subrecords(body):
                    sub.setdefault(st, d)
                slots = set()
                b = sub.get("BOD2") or sub.get("BODT")
                if t == "ARMO" and b:
                    v, = struct.unpack_from("<I", b)
                    slots = {30 + i for i in range(32) if v >> i & 1}
                table[fid & 0xFFFFFF] = (t, sub.get("EDID", b"").rstrip(b"\0").decode("cp1252", "replace"), slots)
            self.cache[key] = table
        return self.cache[key]


def parse_ref(ref):
    plugin, local = ref.rsplit(":", 1)
    return plugin, int(local, 16)


def zstr(s):
    return s.encode("cp1252") + b"\0"


def sub(t, data):
    return t.encode() + struct.pack("<H", len(data)) + data


def record(t, formid, data, flags=0):
    return t.encode() + struct.pack("<IIIHHHH", len(data), flags, formid, 0, 0, 44, 0) + data


def build_plugin(outfits, masters):
    """outfits: list of (edid, [(plugin, local)]). Returns plugin bytes."""
    midx = {m.lower(): i for i, m in enumerate(masters)}
    recs = b""
    ids = []
    for i, (edid, pieces) in enumerate(outfits):
        fid = (len(masters) << 24) | (0x800 + i)
        inam = b"".join(struct.pack("<I", (midx[p.lower()] << 24) | local) for p, local in pieces)
        recs += record("OTFT", fid, sub("EDID", zstr(edid)) + sub("INAM", inam))
        ids.append(0x800 + i)
    grup = b"GRUP" + struct.pack("<I", 24 + len(recs)) + b"OTFT" + struct.pack("<iHHI", 0, 0, 0, 0) + recs
    hedr = struct.pack("<fiI", 1.71, len(outfits), 0x800 + len(outfits))
    head = sub("HEDR", hedr) + sub("CNAM", zstr("SpouseOutfits"))
    head += sub("SNAM", zstr("Outfits for female NPCs, distributed by SpouseOutfits_DISTR.ini (SPID)."))
    for m in masters:
        head += sub("MAST", zstr(m)) + sub("DATA", struct.pack("<Q", 0))
    tes4 = record("TES4", 0, head, flags=0x200)  # ESL flag
    return tes4 + grup, ids


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mo2", required=True)
    ap.add_argument("--profile", default="Default")
    ap.add_argument("--data", required=True, help="game Data folder")
    ap.add_argument("--spec", default=SPEC)
    ap.add_argument("--out", help="folder for SpouseOutfits.esp and SpouseOutfits_DISTR.ini")
    ap.add_argument("--check", action="store_true")
    a = ap.parse_args()

    spec = json.load(open(a.spec, encoding="utf-8"))
    paths = find_plugins(a.mo2, a.profile, a.data)
    index = Index(paths)
    errors, used = [], set()

    def check(ref, kinds, label, outfit_piece=False):
        plugin, local = parse_ref(ref)
        table = index.get(plugin)
        if local not in table:
            errors.append(f"{label}: {ref} not defined in {plugin}")
            return None
        t, edid, slots = table[local]
        if t not in kinds:
            errors.append(f"{label}: {ref} is {t} ({edid}), expected {'/'.join(kinds)}")
        if outfit_piece and slots & HAIR_SLOTS:
            errors.append(f"{label}: {ref} ({edid}) uses head/hair slot {sorted(slots & HAIR_SLOTS)}")
        used.add(plugin.lower())
        return plugin, local, edid

    outfits, lines = [], []
    for npc in spec["npcs"]:
        name = npc["name"]
        nref = check(npc["npc"], {"NPC_"}, name)
        entry = {"name": name, "npc": nref, "outfit": None, "items": []}
        if "outfit" in npc:
            pieces = [check(r, {"ARMO"}, name, True) for r, _ in npc["outfit"]]
            edid = "SO_" + "".join(c for c in name.title() if c.isalnum()) + "Outfit"
            outfits.append((edid, [(p, l) for p, l, _ in filter(None, pieces)]))
            entry["outfit"] = ("new", len(outfits) - 1, edid)
        elif "outfit_ref" in npc:
            r = check(npc["outfit_ref"][0], {"OTFT"}, name)
            if r:
                entry["outfit"] = ("ref", r)
        for it in npc.get("items", []):
            r = check(it[0], {"WEAP", "ARMO", "AMMO"}, name)
            if r:
                entry["items"].append((r, it[1], it[2] if len(it) > 2 else 1))
        lines.append(entry)

    if errors:
        print("ERRORS:")
        for e in errors:
            print("  " + e)
        sys.exit(1)

    order = load_order(a.mo2, a.profile)
    out_plugin = spec["plugin"]
    masters = sorted({p for _, pcs in outfits for p, _ in pcs},
                     key=lambda m: order.index(m.lower()) if m.lower() in order else 10_000)
    missing = [m for m in masters if m.lower() not in order]
    if missing:
        print("WARNING: not in load order (plugins.txt): " + ", ".join(missing))
    # keep the master's own spelling from disk
    masters = [os.path.basename(paths[m.lower()]) for m in masters]
    data, ids = build_plugin(outfits, masters)

    def ref(plugin, local):
        return f"0x{local:X}~{plugin}"

    ini = ["; SpouseOutfits: outfits and weapons for female NPCs (SPID).",
           "; Generated by skyrim-mods/tools/build_spouse_outfits.py from SpouseOutfits/outfits.json. Edit the JSON, not this file.",
           ""]
    for e in lines:
        np_, nl, _ = e["npc"]
        ini.append(f"; {e['name']}")
        # comments on their own line: SPID's ini reader does not strip trailing "; ..." from values
        if e["outfit"]:
            if e["outfit"][0] == "new":
                _, i, edid = e["outfit"]
                ini += [f";   {edid}", f"Outfit = {ref(out_plugin, ids[i])}|NONE|{ref(np_, nl)}"]
            else:
                p, l, edid = e["outfit"][1]
                ini += [f";   {edid} ({p})", f"Outfit = {ref(p, l)}|NONE|{ref(np_, nl)}"]
        for (p, l, edid), label, count in e["items"]:
            ini += [f";   {label}", f"Item = {ref(p, l)}|NONE|{ref(np_, nl)}|NONE|NONE|{count}"]
        ini.append("")

    print(f"{len(outfits)} outfits, {sum(len(e['items']) for e in lines)} item lines, {len(masters)} masters:")
    print("  " + ", ".join(masters))
    if a.check or not a.out:
        return
    os.makedirs(a.out, exist_ok=True)
    open(os.path.join(a.out, out_plugin), "wb").write(data)
    open(os.path.join(a.out, "SpouseOutfits_DISTR.ini"), "w", encoding="utf-8", newline="\r\n").write("\n".join(ini))
    print(f"written {out_plugin} ({len(data)} bytes) and SpouseOutfits_DISTR.ini to {a.out}")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    main()
