"""Build SpouseOutfits.esp (ESL-flagged, only OTFT records) and SpouseOutfits_DISTR.ini (SPID)
from skyrim-mods/SpouseOutfits/outfits.json.

Every reference in the JSON is checked against the real plugin found through MO2 (enabled mods,
highest priority wins, then the game's Data folder): it must exist, outfit pieces must be ARMO,
items must be WEAP/ARMO/AMMO, and no outfit piece may take a head/hair slot (30, 31, 41).
An NPC's optional "heavy" list names outfit pieces to turn into heavy armor: the plugin then also carries an
override of those ARMO records (armor type Heavy, keyword ArmorLight -> ArmorHeavy, nothing else changed).

  py -I build_spouse_outfits.py --mo2 "E:\\Skyrim Modding\\MO2" --data "<game>\\Data" --out "<folder>"
  (--check only validates and prints, writes nothing)
"""
import argparse
import json
import os
import struct
import sys
import zlib

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


ARMOR_LIGHT, ARMOR_HEAVY = 0x06BBD3, 0x06BBD2  # Skyrim.esm keywords
# Outfit fix for saves where another mod already called SetOutfit (SPID then keeps that outfit): SPID gives every NPC
# a hidden ability; its script calls SetOutfit with the planned outfit once per NPC (PapyrusUtil StorageUtil marker,
# version = checksum of the NPC -> outfit table and the outfit pieces, so any change runs it again). Fixed FormIDs, kept clear of the OTFT range 0x800...
FIX_MGEF, FIX_SPEL = 0xF00, 0xF01
FIX_SCRIPT = "SpouseOutfitsFixEffect"
FORMID_SUBS = {"KWDA", "MODL", "EITM", "ETYP", "BIDS", "BAMT", "YNAM", "ZNAM", "RNAM", "TNAM"}


def heavy_override(path, local):
    """Read ARMO `local` from the plugin at `path` and return (masters, flags, subrecords) changed to
    heavy armor: BOD2 armor type -> Heavy, keyword ArmorLight -> ArmorHeavy. Everything else unchanged.
    FormIDs inside still use the source plugin's master indexes (remapped in build_plugin)."""
    masters, _, recs = esp.load(path)
    for t, fid, flags, body in recs:
        if t == "ARMO" and fid & 0xFFFFFF == local and fid >> 24 == len(masters):
            break
    else:
        raise SystemExit(f"heavy: ARMO {local:06X} not found in {path}")
    subs = []
    for st, d in esp.subrecords(body):
        if st == "VMAD":
            raise SystemExit(f"heavy: {path} {local:06X} has scripts (VMAD), not supported")
        if st == "BOD2":
            d = d[:4] + struct.pack("<I", 1) + d[8:]
        subs.append([st, d])
    sky = next((i for i, m in enumerate(masters) if m.lower() == "skyrim.esm"), None)
    if sky is None:
        raise SystemExit(f"heavy: {path} does not have Skyrim.esm as master")
    kw = next((s for s in subs if s[0] == "KWDA"), None)
    ids = list(struct.unpack(f"<{len(kw[1]) // 4}I", kw[1])) if kw else []
    ids = [x for x in ids if x != (sky << 24 | ARMOR_LIGHT) and x != (sky << 24 | ARMOR_HEAVY)]
    ids.append(sky << 24 | ARMOR_HEAVY)
    if kw:
        kw[1] = struct.pack(f"<{len(ids)}I", *ids)
        for s in subs:
            if s[0] == "KSIZ":
                s[1] = struct.pack("<I", len(ids))
    else:
        # field order: ... RNAM, KSIZ, KWDA, DESC, MODL, DATA ...
        i = next((i for i, s in enumerate(subs) if s[0] in ("DESC", "MODL", "DATA")), len(subs))
        subs[i:i] =[["KSIZ", struct.pack("<I", 1)], ["KWDA", struct.pack("<I", ids[0])]]
    return masters, flags & ~0x40000, subs


def remap_subs(subs, src_masters, src_plugin, midx):
    """Rewrite every FormID in the ARMO subrecords from the source plugin's master list to ours."""
    def fix(fid):
        if fid == 0:
            return 0
        i = fid >> 24
        name = src_masters[i] if i < len(src_masters) else src_plugin
        return (midx[name.lower()] << 24) | (fid & 0xFFFFFF)

    out = b""
    for st, d in subs:
        if st in FORMID_SUBS and len(d) % 4 == 0 and not (st == "MODL" and len(d) != 4):
            d = b"".join(struct.pack("<I", fix(x)) for x in struct.unpack(f"<{len(d) // 4}I", d))
        elif st in ("MO2S", "MO3S", "MO4S", "MO5S"):  # alternate textures: count, (len, name, TXST, index)*
            n, = struct.unpack_from("<I", d)
            p, nd = 4, bytearray(d)
            for _ in range(n):
                ln, = struct.unpack_from("<I", d, p)
                p += 4 + ln
                struct.pack_into("<I", nd, p, fix(struct.unpack_from("<I", d, p)[0]))
                p += 8
            d = bytes(nd)
        out += sub(st, d)
    return out


def build_plugin(outfits, masters, overrides=()):
    """outfits: list of (edid, [(plugin, local)]); overrides: [(plugin, local, (masters, flags, subs))].
    Returns plugin bytes."""
    midx = {m.lower(): i for i, m in enumerate(masters)}
    armo = b""
    for plugin, local, (src_masters, flags, subs) in overrides:
        fid = (midx[plugin.lower()] << 24) | local
        armo += record("ARMO", fid, remap_subs(subs, src_masters, plugin, midx), flags)
    recs = b""
    ids = []
    for i, (edid, pieces) in enumerate(outfits):
        fid = (len(masters) << 24) | (0x800 + i)
        inam = b"".join(struct.pack("<I", (midx[p.lower()] << 24) | local) for p, local in pieces)
        recs += record("OTFT", fid, sub("EDID", zstr(edid)) + sub("INAM", inam))
        ids.append(0x800 + i)
    def group(label, data):
        return b"GRUP" + struct.pack("<I", 24 + len(data)) + label.encode() + struct.pack("<iHHI", 0, 0, 0, 0) + data

    # hidden constant-effect ability (given by SPID) whose script sets the NPC's outfit once (see FIX_* below)
    own = len(masters) << 24
    vmad = struct.pack("<hhH", 5, 2, 1) + struct.pack("<H", len(FIX_SCRIPT)) + FIX_SCRIPT.encode() + struct.pack("<BH", 0, 0)
    data = bytearray(152)
    struct.pack_into("<I", data, 0, 0x8000 | 0x200 | 0x400 | 0x800 | 0x8000000 | 0x10000000)  # hide in UI, no
    # duration/magnitude/area, no hit effect, no death dispel
    struct.pack_into("<ii", data, 12, -1, -1)  # magic skill, resist value: none
    struct.pack_into("<Ii", data, 64, 1, -1)  # archetype Script, actor value none
    struct.pack_into("<II", data, 80, 0, 0)  # casting type constant, delivery self
    struct.pack_into("<i", data, 88, -1)  # second actor value none
    struct.pack_into("<f", data, 104, 1.0)  # skill usage mult
    struct.pack_into("<f", data, 112, 1.0)  # dual casting scale
    mgef = record("MGEF", own | FIX_MGEF, sub("EDID", zstr("SO_FixOutfitEffect")) + sub("VMAD", vmad)
                  + sub("FULL", zstr("SpouseOutfits outfit fix")) + sub("DATA", bytes(data)) + sub("SNDD", b"")
                  + sub("DNAM", zstr("")))
    spit = struct.pack("<IIIfIIffI", 0, 0, 4, 0.0, 0, 0, 0.0, 0.0, 0)  # type Ability, constant, self
    spel = record("SPEL", own | FIX_SPEL, sub("EDID", zstr("SO_FixOutfitAbility")) + sub("OBND", bytes(12))
                  + sub("FULL", zstr("SpouseOutfits outfit fix")) + sub("DESC", zstr("")) + sub("SPIT", spit)
                  + sub("EFID", struct.pack("<I", own | FIX_MGEF)) + sub("EFIT", struct.pack("<fII", 0.0, 0, 0)))

    grup = group("MGEF", mgef) + group("SPEL", spel)
    if armo:
        grup += group("ARMO", armo)
    grup += group("OTFT", recs)
    hedr = struct.pack("<fiI", 1.71, len(outfits) + len(overrides) + 2, FIX_SPEL + 1)
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

    outfits, lines, heavy = [], [], {}
    for npc in spec["npcs"]:
        name = npc["name"]
        nref = check(npc["npc"], {"NPC_"}, name)
        entry = {"name": name, "npc": nref, "outfit": None, "items": [], "obody": npc.get("obody_presets")}
        for r in npc.get("heavy", []):
            # make an outfit piece heavy armor (override record in our plugin)
            if r not in [o[0] for o in npc.get("outfit", [])]:
                errors.append(f"{name}: heavy piece {r} is not in the outfit")
            elif r not in heavy:
                p, l = parse_ref(r)
                heavy[r] = (p, l, heavy_override(paths[p.lower()], l))
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
    needed = {p for _, pcs in outfits for p, _ in pcs}
    for p, _, (src_masters, _, _) in heavy.values():
        needed |= {p, *src_masters}
    # one spelling per plugin (case-insensitive)
    needed = list({m.lower(): m for m in needed}.values())
    masters = sorted(needed, key=lambda m: order.index(m.lower()) if m.lower() in order else 10_000)
    missing = [m for m in masters if m.lower() not in order]
    if missing:
        print("WARNING: not in load order (plugins.txt): " + ", ".join(missing))
    # keep the master's own spelling from disk
    masters = [os.path.basename(paths[m.lower()]) for m in masters]
    if 0x800 + len(outfits) > FIX_MGEF:
        raise SystemExit("too many outfits for the fixed FIX_MGEF/FIX_SPEL FormIDs")
    data, ids = build_plugin(outfits, masters, list(heavy.values()))

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
    targets = ",".join(ref(*e["npc"][:2]) for e in lines if e["outfit"])
    ini += ["; Hidden ability: its script re-applies the outfit once when another mod already changed it (SetOutfit).",
            f"Spell = {ref(out_plugin, FIX_SPEL)}|NONE|{targets}", ""]

    print(f"{len(outfits)} outfits, {sum(len(e['items']) for e in lines)} item lines, "
          f"{len(heavy)} pieces made heavy, {len(masters)} masters:")
    print("  " + ", ".join(masters))
    if a.check or not a.out:
        return
    os.makedirs(a.out, exist_ok=True)
    open(os.path.join(a.out, out_plugin), "wb").write(data)
    open(os.path.join(a.out, "SpouseOutfits_DISTR.ini"), "w", encoding="utf-8", newline="\r\n").write("\n".join(ini))
    print(f"written {out_plugin} ({len(data)} bytes) and SpouseOutfits_DISTR.ini to {a.out}")

    # Outfit fix scripts (see FIX_* at the top). Compile both .psc with the Creation Kit's PapyrusCompiler
    # into <out>\Scripts (imports: SKSE sources, PapyrusUtil sources, vanilla sources).
    pairs, contents = [], []
    for e in lines:
        if not e["outfit"]:
            continue
        np_, nl, _ = e["npc"]
        if e["outfit"][0] == "new":
            op, ol = out_plugin, ids[e["outfit"][1]]
            contents.append(outfits[e["outfit"][1]][1])
        else:
            op, ol, _ = e["outfit"][1]
            contents.append(None)
        pairs.append((e["name"], np_, nl, op, ol))
    # version = checksum of the NPC -> outfit table and of the pieces in each outfit: any change
    # (another outfit, or a piece swapped inside one) re-applies the outfits once on every NPC
    version = zlib.crc32(repr([(np_, nl, op, ol) for _, np_, nl, op, ol in pairs] + contents).encode()) & 0x7FFFFFFF
    util = ["Scriptname SpouseOutfitsUtil Hidden",
            "; Generated by skyrim-mods/tools/build_spouse_outfits.py. Edit outfits.json, not this file.",
            "",
            "Outfit Function OutfitFor(ActorBase b) global"]
    for i, (name, np_, nl, op, ol) in enumerate(pairs):
        util += [f"	{'If' if i == 0 else 'ElseIf'} b == Game.GetFormFromFile(0x{nl:X}, \"{np_}\")  ; {name}",
                 f"		Return Game.GetFormFromFile(0x{ol:X}, \"{op}\") as Outfit"]
    util += ["	EndIf", "	Return None", "EndFunction", ""]
    effect = [f"Scriptname {FIX_SCRIPT} extends ActiveMagicEffect",
              "; Generated by skyrim-mods/tools/build_spouse_outfits.py. On the hidden ability SO_FixOutfitAbility (SPID).",
              "; Sets the planned outfit once per NPC, also when another mod (follower framework) changed it with SetOutfit.",
              "",
              "Event OnEffectStart(Actor akTarget, Actor akCaster)",
              f"	If !akTarget || StorageUtil.GetIntValue(akTarget, \"SpouseOutfits.FixVersion\") == {version}",
              "		Return",
              "	EndIf",
              "	Outfit o = SpouseOutfitsUtil.OutfitFor(akTarget.GetActorBase())",
              "	If o",
              "		akTarget.SetOutfit(o)",
              "	EndIf",
              f"	StorageUtil.SetIntValue(akTarget, \"SpouseOutfits.FixVersion\", {version})",
              "EndEvent",
              ""]
    src = os.path.join(a.out, "Scripts", "Source")
    os.makedirs(src, exist_ok=True)
    for name, body in (("SpouseOutfitsUtil", util), (FIX_SCRIPT, effect)):
        open(os.path.join(src, name + ".psc"), "w", encoding="utf-8", newline="\r\n").write("\n".join(body))
    print(f"written Scripts\\Source\\SpouseOutfitsUtil.psc, {FIX_SCRIPT}.psc (compile with PapyrusCompiler)")

    ob = spec.get("obody")
    if ob:
        # OBody NG include file: npcFormID wins over faction/random rules. NPCs with their own "obody_presets"
        # always get them; the others get the shared "presets" list only if one is given (else OBody's own config).
        table = {}
        for e in lines:
            presets = e["obody"] or ob.get("presets")
            if not presets or e["name"] in ob.get("skip", []):
                continue
            p, l, _ = e["npc"]
            table.setdefault(os.path.basename(paths[p.lower()]), {})[f"{l:08X}"] = list(presets)
        inc = os.path.join(a.out, "SKSE", "Plugins", "OBody", "obody_includes")
        os.makedirs(inc, exist_ok=True)
        with open(os.path.join(inc, ob["include_file"]), "w", encoding="utf-8") as f:
            json.dump({"npcFormID": table}, f, ensure_ascii=False, indent=2)
        print(f"written OBody include {ob['include_file']}: {sum(len(v) for v in table.values())} NPCs")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    main()
